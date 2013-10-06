#pragma once

#include <string>
#include <memory>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <log4cxx/logger.h>

#include "GlobalContext.hpp"

namespace orwell
{

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
	        long const iTicDuration = 1000, //milliseconds
			log4cxx::LoggerPtr iLogger = log4cxx::Logger::getLogger("orwell.log") );

	~Server();

    /// processMessageIfAvailable
	bool processMessageIfAvailable();
	/// Wait for 1 message and process it. Execute timed operations if needed.
	void loopUntilOneMessageIsProcessed();
    /// Loop eternaly to process all incoming messages.
    void loop();

	orwell::tasks::GlobalContext & accessContext();

private:
	std::shared_ptr< com::Receiver > _puller;
	std::shared_ptr< com::Sender > _publisher;
	log4cxx::LoggerPtr _logger;
	orwell::tasks::GlobalContext _globalContext;

    boost::posix_time::time_duration const _ticDuration;
    boost::posix_time::ptime _previousTic;
};

}
}

