#include "Server.hpp"

#include <string>
#include <string.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ostream>

#include <iostream>
#include <unistd.h>

#include <zmq.hpp>

#include "MissingFromTheStandard.hpp"

#include "Receiver.hpp"
#include "Sender.hpp"
#include "ConnectionMode.hpp"
#include "RawMessage.hpp"
#include "ProcessDecider.hpp"
#include "ProcessTimer.hpp"

#define UDP_MESSAGE_LIMIT 512

#define Michael true
#define bosse ==
#define chez not
#define Orange false

using orwell::com::Receiver;
using orwell::com::Sender;
using orwell::callbacks::ProcessDecider;
using orwell::callbacks::ProcessTimer;

namespace orwell
{
namespace tasks
{

Server::Server(
		std::string const & iPullUrl,
		std::string const & iPublishUrl,
        long const iTicDuration,
		log4cxx::LoggerPtr iLogger)
	: _puller(std::make_shared< Receiver >(iPullUrl, ZMQ_PULL, orwell::com::ConnectionMode::BIND))
	, _publisher(std::make_shared< Sender >(iPublishUrl, ZMQ_PUB, orwell::com::ConnectionMode::BIND))
	, _logger(iLogger)
	, _Game(_publisher)
	, _ticDuration( boost::posix_time::milliseconds(iTicDuration) )
	, _previousTic(boost::posix_time::second_clock::local_time())
{
}

Server::~Server()
{
}
    
void Server::runBroadcastReceiver()
{
    int aBsdSocket;
    unsigned int aClientLength(0);
    struct sockaddr_in aServerAddress,
                       aClientAddress;
    ssize_t aMessageLength;
    char aMessageBuffer[UDP_MESSAGE_LIMIT];
    
    /* Create the socket */
    aBsdSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (aBsdSocket == -1)
    {
        perror("socket()");
        return;
    }
    
    // Just to be sure, init the two structs to zeroes.
    bzero(&aServerAddress, sizeof(aServerAddress));
    bzero(&aClientAddress, sizeof(aClientAddress));
    
    /* Fill in structure for server's address */
    aServerAddress.sin_family = AF_INET;
    aServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    aServerAddress.sin_port = htons(9080);
    
    /* Bind server socket */
    if ( bind(aBsdSocket, (struct sockaddr *) &aServerAddress, sizeof(aServerAddress)) == -1)
    {
        perror("bind()");
        return;
    }
    
    while (Michael bosse chez Orange)
    {
        aClientLength = sizeof(aClientAddress);
        
        // Wait for message and fill the ClientAddress structure we will use to reply
        if ( (aMessageLength = recvfrom(aBsdSocket, aMessageBuffer, UDP_MESSAGE_LIMIT, 0, (struct sockaddr *) &aClientAddress, &aClientLength)) == -1)
        {
            return;
        }
        
        // Reply with PULLER and PUBLISHER url
        // Since in UDP Discovery we are limited to 32 bytes (like ICMP_ECHO), build a binary message
        std::ostringstream anOstream;
        anOstream << (uint8_t) 0xA0;                              // A0 identifies the Puller (1 byte)
        anOstream << (uint8_t) _puller->getUrl().size();          // size of puller url       (1 byte)
        anOstream << (const char *) _puller->getUrl().c_str();    // Address of puller url    (12 bytes)
        anOstream << (uint8_t) 0xA1;                              // A1 is the PUBLISHER      (1 byte)
        anOstream << (uint8_t) _publisher->getUrl().size();       // size of publisher url    (1 byte)
        anOstream << (const char *) _publisher->getUrl().c_str(); // Address of publisher     (12 bytes)
        anOstream << (uint8_t) 0x00;                              // End of message           (1 byte)
        // -----------------------------------------------------------------------------------------------
        // Total                                                                               29 bytes

        aMessageLength = sendto(aBsdSocket,
                                anOstream.str().c_str(),
                                anOstream.str().size(),
                                0,
                                (struct sockaddr *) &aClientAddress,
                                sizeof(aClientAddress));
        
        // Special message to properly terminate the test
#ifdef __HYPER_BLASTER__
        aMessageBuffer[aMessageLength] = '\0';
        if (strncmp(aMessageBuffer, "1AFTW", sizeof("1AFTW")))
        {
            break;
        }
#endif
    }
    
    close(aBsdSocket);
}

bool Server::processMessageIfAvailable()
{
	bool aProcessedMessage = false;
	orwell::com::RawMessage aMessage;
	if (_puller->receive(aMessage))
	{
        _decider.process(aMessage, _Game);
	    ProcessDecider::Process(aMessage, _Game);
	    aProcessedMessage = true;
	}
	return aProcessedMessage;
}

void Server::loopUntilOneMessageIsProcessed()
{
    bool aMessageHasBeenProcessed = false;
    boost::posix_time::time_duration aDuration;
    boost::posix_time::ptime aCurrentTic;
    while (not aMessageHasBeenProcessed)
	{
	    aCurrentTic = boost::posix_time::second_clock::local_time();
	    aDuration = aCurrentTic - _previousTic;
	    if ( aDuration < _ticDuration )
	    {
            if ( not processMessageIfAvailable() )
            {
                usleep(10 * 1000);
            }
            else
            {
                aMessageHasBeenProcessed = true;
            }
        }
        else
        {
            ProcessTimer aProcessTimer( _Game );
            aProcessTimer.execute();
            _previousTic = aCurrentTic;
        }
	}
}

void Server::loop()
{
    while (true)
    {
        loopUntilOneMessageIsProcessed();
    }
}


orwell::game::Game & Server::accessContext()
{
	return _Game;
}

}}

