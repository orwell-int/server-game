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
			game::Game & ioGame,
			std::shared_ptr< com::Sender > ioPublisher,
			std::shared_ptr< com::Socket > ioReplier);

	void execute();

};

}
}
