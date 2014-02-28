//
//  broadcast_test.cpp
//  orwell
//
//  Created by Massimo Gengarelli on 15/02/14.
//
//

#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>

#include "controller.pb.h"
#include "server-game.pb.h"

#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"

#include "Common.hpp"

#include <log4cxx/logger.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/ndc.h>

#include <string>
#include <cstring>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <signal.h>


using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using namespace std;

static orwell::tasks::Server * ServerPtr;

static void signal_handler(int signum)
{
	ServerPtr->stop();
}

struct IP4
{
	uint8_t b1, b2, b3, b4 = 0;

	operator std::string() const
	{
		char buffer[32];
		sprintf(&buffer[0], "%u.%u.%u.%u", b1, b2, b3, b4);

		return std::string(buffer);
	};

	operator bool() const
	{
		return (not
				(b1 == 0 and b2 == 0 and b3 == 0 and b4 == 0));
	};

	friend std::ostream & operator<<(std::ostream & _stream, IP4 const & ip4)
	{
		_stream << (std::string) ip4;
		return _stream;
	};
};

bool get_ip4(IP4 & oIp4)
{
	char szBuffer[1024];

	if(gethostname(szBuffer, sizeof(szBuffer)) == -1)
	{
		return false;
	}

	struct hostent *host = gethostbyname(szBuffer);
	if(host == NULL)
	{
		return false;
	}

	//Obtain the computer's IP
	oIp4.b1 = (uint8_t) host->h_addr_list[0][0];
	oIp4.b2 = (uint8_t) host->h_addr_list[0][1];
	oIp4.b3 = (uint8_t) host->h_addr_list[0][2];
	oIp4.b4 = (uint8_t) host->h_addr_list[0][3];

	return true;
}

uint32_t simulateClient(log4cxx::LoggerPtr iLogger)
{
	int aSocket;
	ssize_t aMessageLength;
	struct sockaddr_in aDestination;
	unsigned int aDestinationLength;
	char aReply[256];

	char *aMessageToSend = (char*) "1AFTW";
	aMessageLength = strlen(aMessageToSend);

	int broadcast = 1;

	// Build the socket
	if ( (aSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) < 0)
	{
		perror("socket()");
		return 255;
	}

	IP4 address;
	if (not get_ip4(address))
	{
		LOG4CXX_ERROR(iLogger, "Couldn't retrieve local address");
		return 255;
	}
	address.b4 = 255;

	if (address)
	{
		LOG4CXX_INFO(iLogger, "IP: " << address);
	}

	// Build the destination object
	memset(&aDestination, 0, sizeof(aDestination));
	aDestination.sin_family = AF_INET;
	aDestination.sin_addr.s_addr = inet_addr(
			((std::string) address).c_str());
	aDestination.sin_port = htons(9080);

	// Set the destination to the socket
	setsockopt(aSocket, IPPROTO_IP, IP_MULTICAST_IF, &aDestination, sizeof(aDestination));

	// Allow the socket to send broadcast messages
	if ( (setsockopt(aSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(int))) == -1)
	{
		LOG4CXX_ERROR(iLogger, "Not allowed to send broadcast");
		return 255;
	}

	if (sendto(aSocket, aMessageToSend, aMessageLength, 0, (struct sockaddr *) &aDestination, sizeof(aDestination)) != aMessageLength)
	{
		LOG4CXX_ERROR(iLogger, "Did not send the right number of bytes..");
		return 255;
	}

	aDestinationLength = sizeof(aDestination);
	if (recvfrom(aSocket, aReply, sizeof(aReply), 0, (struct sockaddr *) &aDestination, &aDestinationLength) == -1)
	{
		LOG4CXX_ERROR(iLogger, "Did not receive a message...");
		return 255;
	}

	uint8_t aFirstSeparator, aSecondSeparator, aFirstSize, aSecondSize;
	std::string aFirstUrl, aSecondUrl;

	aFirstSeparator = (uint8_t) aReply[0];
	aFirstSize = (uint8_t) aReply[1];
	aFirstUrl = std::string(&aReply[2], aFirstSize);
	aSecondSeparator = (uint8_t) aReply[2 + aFirstSize];
	aSecondSize = (uint8_t) aReply[2 + aFirstSize + 1];
	aSecondUrl = std::string(&aReply[2 + aFirstSize + 2], aSecondSize);

	char aBufferForLogger[128];
	sprintf(aBufferForLogger, "0x%X %d (%s) 0x%X %d (%s)\n",
			aFirstSeparator, aFirstSize, aFirstUrl.c_str(), aSecondSeparator, aSecondSize, aSecondUrl.c_str());

	LOG4CXX_INFO(iLogger, aBufferForLogger);

	close(aSocket);

	return (aFirstSeparator == 0xA0 and aSecondSeparator == 0xA1) ? 0 : 1;
}

void simulateServer(log4cxx::LoggerPtr iLogger)
{
	LOG4CXX_INFO(iLogger, "Running broadcast receiver on server");
	ServerPtr->runBroadcastReceiver();
	LOG4CXX_INFO(iLogger, "Server stopped correctly");
}

int main(int argc, const char * argv [])
{
	int aRc(0);

	auto logger = Common::SetupLogger("broadcast");
	ServerPtr = new orwell::tasks::Server("tcp://*:9801", "tcp://*:9991", 500, logger);
	
	// Handle the signal CHLD (as SIG[INT|TERM] will kill ctest also)
	signal(SIGCHLD, signal_handler);

	pid_t aChild = fork();
	switch (aChild)
	{
		default:
			simulateServer(logger);
			break;
		case 0:
			usleep(2543);
			aRc = simulateClient(logger);
			break;
	}
	
	kill(aChild, SIGCHLD);
	
	// Only the father should delete the ServerPtr.
	if (not aChild)
		delete ServerPtr;

	return aRc;
}

