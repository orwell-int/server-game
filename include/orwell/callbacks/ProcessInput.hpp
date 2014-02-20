#pragma once

//tasks
#include "orwell/callbacks/InterfaceProcess.hpp"

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
