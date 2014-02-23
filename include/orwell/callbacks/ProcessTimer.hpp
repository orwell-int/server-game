#pragma once

#include "orwell/callbacks/InterfaceProcess.hpp"

namespace orwell {

namespace callbacks {

class ProcessTimer : public InterfaceProcess
{
public:
	ProcessTimer(
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	void execute();

};

}} //namespaces
