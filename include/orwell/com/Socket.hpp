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

class Socket
{

public :
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
	Socket(
			std::string const & iUrl,
			unsigned int const iSocketType,
			ConnectionMode const iConnectionMode,
			zmq::context_t & ioZmqContext,
			unsigned int const iSleep = 0);

	~Socket();

	void reset();

	/// Try to get a message from the socket (non blocking by default).
	///
	/// \param oMessage
	///  This string will contain the received message if available.
	///
	/// \param iBlocking
	///  If and only if set to true, make the call blocking.
	///
	/// \return
	///  True if and only if a message was received.
	///
	bool receiveString(
			std::string & oMessage,
			bool const iBlocking=false);

	bool receive(RawMessage & oMessage);

	void sendString(std::string const & iMessage) const;

	void send(RawMessage const & iMessage) const;

	std::string const & getUrl() const;

private :
	Socket(Socket const & iOther) = delete;

	zmq::context_t & m_zmqContext;
	zmq::socket_t * m_zmqSocket;
	std::string const m_url;
	unsigned int const m_socketType;
	ConnectionMode const m_connectionMode;

	void innerReset();
};

}
}

