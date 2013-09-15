#include "Server.hpp"

#include "MissingFromTheStandard.hpp"

#include "Receiver.hpp"
#include "Sender.hpp"
#include "RawMessage.hpp"

#include "ProcessDecider.hpp"

#include <iostream>
#include <zmq.hpp>

#include <boost/foreach.hpp>

using namespace std;

namespace orwell
{
namespace tasks
{

Server::Server(
		std::string const & iPullUrl,
		std::string const & iPublishUrl,
		log4cxx::LoggerPtr iLogger)
	: _puller(make_shared< orwell::com::Receiver >(iPullUrl, ZMQ_PULL, true))
	, _publisher(make_shared< orwell::com::Sender>(iPublishUrl, ZMQ_PUB, true))
	, _logger(iLogger)
	, _globalContext(_publisher)
{
}

Server::~Server()
{
}

int Server::run()
{
	LOG4CXX_INFO(_logger, "server waiting for message")
	orwell::com::RawMessage aMessage = _puller->receive();

	processDecider::Process(aMessage, _globalContext);
	return 0;
}

orwell::tasks::GlobalContext & Server::accessContext()
{
	return _globalContext;
}

}
}

