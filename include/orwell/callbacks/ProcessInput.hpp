#pragma once

//protobuf
#include "controller.pb.h"

//tasks
#include "InterfaceProcess.hpp"

#include <log4cxx/logger.h>

namespace orwell {

namespace messages {
	class Input;
}
namespace game {
	class Game;
}

namespace callbacks {

class ProcessInput : public InterfaceProcess
{
public:
	void execute();

private:

};

}} //namespaces
