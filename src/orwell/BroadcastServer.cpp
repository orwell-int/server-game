#include "orwell/BroadcastServer.hpp"

#include <string>
#include <cstring>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <unistd.h>

#include "orwell/support/GlobalLogger.hpp"

#define UDP_MESSAGE_LIMIT 512
#define MULTICAST_GROUP "225.0.0.42"

namespace orwell
{

BroadcastServer::BroadcastServer(
		std::string const & iPullerUrl,
		std::string const & iPublisherUrl)
	: _mainLoopRunning(false)
	, _forcedStop(false)
	, _pullerUrl(iPullerUrl)
	, _publisherUrl(iPublisherUrl)
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
	char aMessageBuffer[UDP_MESSAGE_LIMIT];
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
	aBroadcastServerAddress.sin_port = htons(9080);

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
	while (_mainLoopRunning)
	{
		// Wait for message and fill the ClientAddress structure we will use to reply
		if (recvfrom(aBsdSocket, aMessageBuffer, UDP_MESSAGE_LIMIT, 0, (struct sockaddr *) &aClientAddress, &aClientLength) == -1)
		{
			// Receive timeout, let's check if we should keep running..
			continue;
		}
		ORWELL_LOG_INFO("Received an UDP broadcast");

		// Reply with PULLER and PUBLISHER url
		// Since in UDP Discovery we are limited to 32 bytes (like ICMP_ECHO), build a binary message
		std::ostringstream anOstream;
		anOstream << (uint8_t) 0xA0;                       // A0 identifies the Puller ( 1 byte )
		anOstream << (uint8_t) _pullerUrl.size();          // size of puller url       ( 1 byte )
		anOstream << (const char *) _pullerUrl.c_str();    // Address of puller url    (12 bytes)
		anOstream << (uint8_t) 0xA1;                       // A1 is the PUBLISHER      ( 1 byte )
		anOstream << (uint8_t) _publisherUrl.size();       // size of publisher url    ( 1 byte )
		anOstream << (const char *) _publisherUrl.c_str(); // Address of publisher     (12 bytes)
		anOstream << (uint8_t) 0x00;                       // End of message           ( 1 byte )
		// -----------------------------------------------------------------------------------------------
		// Total                                                                               29 bytes

		ssize_t aMessageLength = sendto(
				aBsdSocket,
				anOstream.str().c_str(),
				anOstream.str().size(),
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
