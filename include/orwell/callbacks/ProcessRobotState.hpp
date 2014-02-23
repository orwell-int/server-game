#pragma once

#include "orwell/callbacks/InterfaceProcess.hpp"

namespace orwell {

namespace callbacks {

class ProcessRobotState : public InterfaceProcess
{
public:
	ProcessRobotState(
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	void execute();

};

}} //namespaces
