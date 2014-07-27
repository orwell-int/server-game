#include "orwell/com/Sender.hpp"

//std
#include <iostream>
#include <unistd.h>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>

#include <memory>

using std::string;

namespace orwell {
namespace com {

Sender::Sender(
		string const & iUrl,
		unsigned int const iSocketType,
		ConnectionMode const iConnectionMode,
		zmq::context_t & ioZmqContext,
		unsigned int const iSleep)
	: Socket(iUrl, iSocketType, iConnectionMode, ioZmqContext, iSleep)
{

}

Sender::~Sender()
{
}

}
}

