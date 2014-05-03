#pragma once

#include <string>

#include "orwell/com/ConnectionMode.hpp"

namespace zmq {
class context_t;
class socket_t;
}

namespace orwell {
namespace com {

class RawMessage;

class Sender
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

	void sendString(std::string const & iMessage);

	void send( RawMessage const & iMessage );
	std::string const & getUrl() const;


private:
	Sender(Sender const & iOther);

	zmq::socket_t * m_zmqSocket;
	std::string m_url;
};

}}

