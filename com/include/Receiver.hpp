#pragma once

#include <string>

#include "RawMessage.hpp"

#include <log4cxx/logger.h>

#include "ConnectionMode.hpp"

namespace zmq {
	class context_t;
	class socket_t;
}

namespace orwell {
namespace com {

class Receiver
{

public:
	/// \param iSleep
	///  Time to sleep after bind and connect.
	///
	Receiver(
			std::string const & iUrl,
			unsigned int const iSocketType,
			ConnectionMode::ConnectionMode const iConnectionMode,
			unsigned int const iSleep = 0);

	~Receiver();

	RawMessage receive();

private:

	zmq::context_t * _zmqContext;
	zmq::socket_t * _zmqSocket;
	log4cxx::LoggerPtr  _logger;

};

}}

