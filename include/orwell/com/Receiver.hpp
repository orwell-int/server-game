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

class Receiver
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

	/// Try to get a message from the socket (non blocking).
	///
	/// \param oMessage
	///  This string will contain the received message if available.
	///
	/// \return
	///  True if and only if a message was received.
	///
	bool receiveString(std::string & oMessage);
	bool receive(RawMessage & oMessage);
	std::string const & getUrl() const;

private :
	Receiver(Receiver const & iOther);

	zmq::socket_t * m_zmqSocket;
	std::string m_url;

};

}}

