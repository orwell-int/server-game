#pragma once

#include "orwell/callbacks/InterfaceProcess.hpp"

namespace orwell
{
namespace callbacks
{

class ProcessInput : public InterfaceProcess
{
public:
	ProcessInput(
			game::Game & ioGame,
			std::shared_ptr< com::Sender > ioPublisher,
			std::shared_ptr< com::Socket > ioReplier);

	void execute();

private:

};

} // namespace callbacks
} // namespace orwell
