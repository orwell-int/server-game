#pragma once

//protobuf
#include "robot.pb.h"

//tasks
#include "orwell/callbacks/InterfaceProcess.hpp"

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
    void execute();

};

}} //namespaces
