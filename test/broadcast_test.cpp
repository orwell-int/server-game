//
//  broadcast_test.cpp
//  orwell
//
//  Created by Massimo Gengarelli on 15/02/14.
//
//

#define __HYPER_BLASTER__ 1

#include "RawMessage.hpp"

#include <zmq.hpp>

#include "controller.pb.h"
#include "server-game.pb.h"

#include "Sender.hpp"
#include "Receiver.hpp"
#include "Server.hpp"

#include "Common.hpp"

#include <log4cxx/logger.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/ndc.h>

#include <string>
#include <string.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <iostream>
#include <ostream>
#include <istream>
#include <sstream>


using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using namespace std;

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
    unsigned long aMessageLength;
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
    get_ip4(address);
    address.b4 = 255;
    
    if (address)
        LOG4CXX_INFO(iLogger, "IP: " << address);
    
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
        LOG4CXX_INFO(iLogger, "Not allowed to send broadcast");
        return 255;
    }
    
    if (sendto(aSocket, aMessageToSend, aMessageLength, 0, (struct sockaddr *) &aDestination, sizeof(aDestination)) != aMessageLength)
    {
        LOG4CXX_INFO(iLogger, "Did not send the right number of bytes..");
        return 255;
    }
    
    aDestinationLength = sizeof(aDestination);
    if (recvfrom(aSocket, aReply, sizeof(aReply), 0, (struct sockaddr *) &aDestination, &aDestinationLength) == -1)
    {
        LOG4CXX_INFO(iLogger, "Did not receive a message...");
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


int main(int argc, const char * argv [])
{
    auto logger = Common::SetupLogger("hello");
	NDC ndc("broadcast");
	std::shared_ptr< orwell::tasks::Server > aServer =
        std::make_shared< orwell::tasks::Server >("tcp://*:9801", "tcp://*:9991", 500, logger);
    
    switch (fork())
    {
        case 0:
            aServer->runBroadcastReceiver();
            return 0;
        default:
            sleep(3);
            return simulateClient(logger);
    }
    
    // We should never arrive here
    return 2;
}