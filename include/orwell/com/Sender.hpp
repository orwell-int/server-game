#pragma once

#include <string>

#include <log4cxx/logger.h>

#include "ConnectionMode.hpp"

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
			ConnectionMode::ConnectionMode const iConnectionMode,
			unsigned int const iSleep = 0);
	~Sender();

	void send( RawMessage const & iMessage );
    std::string const & getUrl() const;


private:

	zmq::context_t * _zmqContext;
	zmq::socket_t * _zmqSocket;
	log4cxx::LoggerPtr _logger;
    std::string _url;
};

}}

