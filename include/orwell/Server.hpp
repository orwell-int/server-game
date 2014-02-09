#pragma once

#include <string>
#include <memory>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <log4cxx/logger.h>
#include <ProcessDecider.hpp>

#include "Game.hpp"

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
	        long const iTicDuration = 500, //milliseconds
			log4cxx::LoggerPtr iLogger = log4cxx::Logger::getLogger("orwell.log") );

	~Server();

    /// processMessageIfAvailable
	bool processMessageIfAvailable();
	/// Wait for 1 message and process it. Execute timed operations if needed.
	void loopUntilOneMessageIsProcessed();
    /// Loop eternaly to process all incoming messages.
    void loop();

	orwell::game::Game & accessContext();

private:
	std::shared_ptr< com::Receiver > _puller;
	std::shared_ptr< com::Sender > _publisher;
	log4cxx::LoggerPtr _logger;
	orwell::game::Game _Game;
    orwell::callbacks::ProcessDecider _decider;

    boost::posix_time::time_duration const _ticDuration;
    boost::posix_time::ptime _previousTic;
};

}
}

