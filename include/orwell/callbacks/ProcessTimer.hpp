#pragma once

//tasks
#include "InterfaceProcess.hpp"

#include <string>
#include <log4cxx/logger.h>

namespace orwell {
namespace game {
	class Game;
}
namespace callbacks {

class ProcessTimer : public InterfaceProcess
{
public:
    ProcessTimer( game::Game & ioCtx,
    		std::shared_ptr< com::Sender > ioPublisher,
    		log4cxx::LoggerPtr iLogger = log4cxx::Logger::getLogger("orwell.log"));
    ~ProcessTimer();

    void execute();

private:
    log4cxx::LoggerPtr _logger;

};

}} //namespaces

