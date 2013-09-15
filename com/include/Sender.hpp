#pragma once

#include <string>

#include <log4cxx/logger.h>

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
	Sender(
			std::string const & iUrl,
			unsigned int const iSocketType,
			bool const iBind);
	~Sender();

	void send( RawMessage const & iMessage );


private:

	zmq::context_t * _zmqContext;
	zmq::socket_t * _zmqSocket;
	log4cxx::LoggerPtr _logger;
};

}}

