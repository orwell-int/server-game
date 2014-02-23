#pragma once

#include "orwell/callbacks/InterfaceProcess.hpp"

namespace orwell {

namespace callbacks {

class ProcessHello : public InterfaceProcess
{
public:
	ProcessHello(
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	void execute();

private:
};

}} //namespaces
