#include "Server.hpp"

#include "MissingFromTheStandard.hpp"

#include "Receiver.hpp"
#include "Sender.hpp"
#include "ConnectionMode.hpp"
#include "RawMessage.hpp"

#include "ProcessDecider.hpp"

#include <iostream>
#include <zmq.hpp>

using orwell::com::Receiver;
using orwell::com::Sender;

namespace orwell
{
namespace tasks
{

Server::Server(
		std::string const & iPullUrl,
		std::string const & iPublishUrl,
		log4cxx::LoggerPtr iLogger)
	: _puller(std::make_shared< Receiver >(iPullUrl, ZMQ_PULL, orwell::com::ConnectionMode::BIND))
	, _publisher(std::make_shared< Sender >(iPublishUrl, ZMQ_PUB, orwell::com::ConnectionMode::BIND))
	, _logger(iLogger)
	, _globalContext(_publisher)
{
}

Server::~Server()
{
}

bool Server::run()
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

orwell::tasks::GlobalContext & Server::accessContext()
{
	return _globalContext;
}

}
}

