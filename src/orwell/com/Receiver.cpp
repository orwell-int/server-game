#include "orwell/com/Receiver.hpp"

//std
#include <iostream>
#include <unistd.h>

#include <zmq.hpp>

#include <sstream>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"

using std::string;

namespace orwell
{
namespace com
{

Receiver::Receiver(
		std::string const & iUrl,
		unsigned int const iSocketType,
		ConnectionMode const iConnectionMode,
		zmq::context_t & ioZmqContext,
		unsigned int const iSleep)
	: Socket(iUrl, iSocketType, iConnectionMode, ioZmqContext, iSleep)
{
}

Receiver::~Receiver()
{
}

}
}

