#pragma once

#include "orwell/callbacks/InterfaceProcess.hpp"

namespace orwell
{

namespace callbacks
{

class ProcessRegister : public InterfaceProcess
{
public:
	ProcessRegister(
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	void execute();

};

}
}

