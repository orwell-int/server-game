#pragma once

#include "orwell/callbacks/InterfaceProcess.hpp"

namespace orwell
{

namespace callbacks
{

class ProcessPing : public InterfaceProcess
{
public:
	ProcessPing(
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	void execute();

private:
};

}
}
