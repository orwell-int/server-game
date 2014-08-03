#pragma once

#include <string>

#include "orwell/com/Socket.hpp"

namespace orwell
{
namespace com
{

class RawMessage;

class Receiver : private Socket
{

public :
	/// \param iSleep
	///  Time to sleep after bind and connect.
	///
	Receiver(
			std::string const & iUrl,
			unsigned int const iSocketType,
			ConnectionMode const iConnectionMode,
			zmq::context_t & ioZmqContext,
			unsigned int const iSleep = 0);

	~Receiver();

	using Socket::receiveString;

	using Socket::receive;

private :
	Receiver(Receiver const & iOther) = delete;


};

}
}

