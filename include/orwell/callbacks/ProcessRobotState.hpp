#pragma once

//protobuf
#include "robot.pb.h"

//tasks
#include "InterfaceProcess.hpp"

#include <log4cxx/logger.h>

namespace orwell {

namespace messages {
	class RobotState;
}
namespace game {
	class Game;
}

namespace callbacks {

class ProcessRobotState : public InterfaceProcess
{
public:
    ProcessRobotState(std::string const & iRoutingId,
    		messages::RobotState const & iRobotStateMsg,
    		game::Game & ioCtx,
    		std::shared_ptr< com::Sender > ioPublisher);
    ~ProcessRobotState();

    void execute();


private:
    std::string _dest;
    messages::RobotState const & _robotState ;
    log4cxx::LoggerPtr _logger;


};

}} //namespaces
