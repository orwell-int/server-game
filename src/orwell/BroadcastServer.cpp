#include "orwell/BroadcastServer.hpp"

#include <string>
#include <cstring>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <unistd.h>
#include <stdlib.h>

#include "orwell/support/GlobalLogger.hpp"

#define UDP_MESSAGE_LIMIT 512
#define MULTICAST_GROUP "225.0.0.42"

namespace orwell
{

BroadcastServer::BroadcastServer(
		uint16_t const iBroadcastPort,
		std::string const & iPullerUrl,
		std::string const & iPublisherUrl,
		std::string const & iReplierUrl,
		std::string const & iAgentUrl)
	: _mainLoopRunning(false)
	, _forcedStop(false)
	, m_broadcastPort(iBroadcastPort)
	, _pullerUrl(iPullerUrl)
	, _publisherUrl(iPublisherUrl)
	, _replierUrl(iReplierUrl)
	, _agentUrl(iAgentUrl)
{
}

BroadcastServer::~BroadcastServer()
{
	ORWELL_LOG_INFO("DESTRUCTOR: ~BroadcastServer");
}

void BroadcastServer::runBroadcastReceiver()
{
	int aBsdSocket;
	struct sockaddr_in aBroadcastServerAddress;
	struct sockaddr_in aClientAddress;
	struct ip_mreq aGroup;
	unsigned int aClientLength = sizeof(aClientAddress);

	/* This is used to set a RCV Timeout on the socket */
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 1000;

	/* Create the socket */
	aBsdSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Just to be sure, init the two structs to zeroes.
	bzero(&aBroadcastServerAddress, sizeof(aBroadcastServerAddress));
	bzero(&aClientAddress, sizeof(aClientAddress));
	bzero(&aGroup, sizeof(aGroup));

	/* Fill in structure for server's address */
	aBroadcastServerAddress.sin_family = AF_INET;
	aBroadcastServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	aBroadcastServerAddress.sin_port = htons(m_broadcastPort);

	/* Bind server socket */
	bind(aBsdSocket, (struct sockaddr *) &aBroadcastServerAddress, sizeof(aBroadcastServerAddress));

	/* use setsockopt() to request that the kernel join a multicast group */
	aGroup.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
	aGroup.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(aBsdSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &aGroup, sizeof(aGroup)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}

	/* Set the RCV Timeout */
	setsockopt(aBsdSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	_mainLoopRunning = true;
	ssize_t aReadSize;
	int aVersion;
	char aMessageBuffer[UDP_MESSAGE_LIMIT];
	memset(aMessageBuffer, 0, UDP_MESSAGE_LIMIT);
	while (_mainLoopRunning)
	{
		// Wait for message and fill the ClientAddress structure we will use to reply
		aReadSize = recvfrom(
				aBsdSocket, aMessageBuffer, UDP_MESSAGE_LIMIT, 0,
				(struct sockaddr *) &aClientAddress, &aClientLength);
		aVersion = 0;
		if (aReadSize == -1)
		{
			// Receive timeout, let's check if we should keep running..
			continue;
		}
		else if (aReadSize > 0)
		{
			aVersion = atoi(aMessageBuffer);
			memset(aMessageBuffer, 0, aReadSize);
		}
		ORWELL_LOG_INFO("Received an UDP broadcast version (" << aVersion << ")");

		// Reply with PULLER and PUBLISHER url
		// Since in UDP Discovery we are limited to 32 bytes (like ICMP_ECHO), build a binary message
		std::ostringstream aOstream;
		if (aVersion >= 0)
		{
			aOstream << (uint8_t) 0xA0;                       // A0 identifies the Puller ( 1 byte )
			aOstream << (uint8_t) _pullerUrl.size();          // size of puller url       ( 1 byte )
			aOstream << (const char *) _pullerUrl.c_str();    // Address of puller url    (12 bytes)
			aOstream << (uint8_t) 0xA1;                       // A1 is the Publisher      ( 1 byte )
			aOstream << (uint8_t) _publisherUrl.size();       // size of publisher url    ( 1 byte )
			aOstream << (const char *) _publisherUrl.c_str(); // Address of publisher     (12 bytes)
		}
		if (aVersion >= 1)
		{
			aOstream << (uint8_t) 0xA2;                       // A2 is the REQ/REP        ( 1 byte )
			aOstream << (uint8_t) _replierUrl.size();         // size of REQ/REP url      ( 1 byte )
			aOstream << (const char *) _replierUrl.c_str();   // Address of REQ/REP       (12 bytes)
			// the rumour said 32 bytes was the limit but this still works even if bigger
		}
		if (aVersion >= 2)
		{
			aOstream << (uint8_t) 0xA3;                       // A3 is the agent        ( 1 byte )
			aOstream << (uint8_t) _agentUrl.size();           // size of agent url      ( 1 byte )
			aOstream << (const char *) _agentUrl.c_str();     // Address of agent       (12 bytes)
		}
		aOstream << (uint8_t) 0x00;                       // End of message               ( 1 byte )

		ssize_t aMessageLength = sendto(
				aBsdSocket,
				aOstream.str().c_str(),
				aOstream.str().size(),
				0,
				(struct sockaddr *) &aClientAddress,
				sizeof(aClientAddress));
		ORWELL_LOG_INFO("sendto returned length " << aMessageLength);
	}

	ORWELL_LOG_INFO("Closing broadcast service");
	close(aBsdSocket);
}

void BroadcastServer::stop()
{
	ORWELL_LOG_INFO("Terminating server main loop");
	_forcedStop = true;
	_mainLoopRunning = false;
}

}
