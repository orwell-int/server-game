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
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	void execute();

private:

};

} // namespace callbacks
} // namespace orwell
