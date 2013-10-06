#include "Server.hpp"

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

using orwell::com::Receiver;
using orwell::com::Sender;

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
	, _globalContext(_publisher)
	, _ticDuration( boost::posix_time::milliseconds(iTicDuration) )
	, _previousTic(boost::posix_time::second_clock::local_time())
{
}

Server::~Server()
{
}

bool Server::processMessageIfAvailable()
{
	bool aProcessedMessage = false;
	orwell::com::RawMessage aMessage;
	if (_puller->receive(aMessage))
	{
	    processDecider::Process(aMessage, _globalContext);
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
            ProcessTimer aProcessTimer( _globalContext );
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


orwell::tasks::GlobalContext & Server::accessContext()
{
	return _globalContext;
}

}
}

