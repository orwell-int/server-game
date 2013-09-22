#pragma once

#include <string>
#include <memory>
#include "GlobalContext.hpp"

#include <log4cxx/logger.h>

namespace orwell {

namespace com
{
class Receiver;
class Sender;
}

namespace tasks
{

class Server
{
public:
	Server(
			std::string const & iPullUrl = "tcp://*:9000",
			std::string const & iPublishUrl = "tcp://*:9001",
			log4cxx::LoggerPtr iLogger = log4cxx::Logger::getLogger("orwell.log"));

	~Server();

	bool run();

	orwell::tasks::GlobalContext & accessContext();

private:
	std::shared_ptr< com::Receiver > _puller;
	std::shared_ptr< com::Sender > _publisher;
	log4cxx::LoggerPtr _logger;
	orwell::tasks::GlobalContext _globalContext;
};

}
}

