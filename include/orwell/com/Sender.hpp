#pragma once

#include <string>

#include "orwell/com/Socket.hpp"

namespace orwell
{
namespace com
{

class RawMessage;

class Sender : private Socket
{
public:

	/// \param iUrl
	///    Url object used to know where to open the socket.
	///
	/// \param iSocketType
	///    ZMQ type of socket.
	///
	/// \param iBind
	///    True if and only if the socket is to call bind instead of connect.
	///
	/// \param iSleep
	///  Time to sleep after bind and connect.
	///
	Sender(
			std::string const & iUrl,
			unsigned int const iSocketType,
			ConnectionMode const iConnectionMode,
			zmq::context_t & ioZmqContext,
			unsigned int const iSleep = 0);
	~Sender();

	using Socket::sendString;

	using Socket::send;


private:
	Sender(Sender const & iOther) = delete;
};

}
}

