#pragma once

#include "orwell/callbacks/InterfaceProcess.hpp"

namespace orwell
{

namespace callbacks
{

class ProcessPong : public InterfaceProcess
{
public:
	ProcessPong(
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	void execute();

private:
};

}
}
