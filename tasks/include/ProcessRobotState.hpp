#pragma once

//protobuf
#include "robot.pb.h"

//tasks
#include "GlobalContext.hpp"
#include "InterfaceProcess.hpp"

#include <log4cxx/logger.h>

namespace orwell {

namespace messages {
class RobotState;
}

namespace tasks {

class ProcessRobotState : public InterfaceProcess
{
public:
    ProcessRobotState(std::string const & iRoutingId, messages::RobotState const & iRobotStateMsg, GlobalContext & ioCtx);
    ~ProcessRobotState();

    void execute();


private:
    std::string _dest;
    messages::RobotState const & _robotState ;
    log4cxx::LoggerPtr _logger;


};

}} //namespaces
