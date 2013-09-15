#pragma once

#include <string>

#include "RawMessage.hpp"

#include <log4cxx/logger.h>

namespace zmq {
	class context_t;
	class socket_t;
}

namespace orwell {
namespace com {

class Receiver
{

public:
	Receiver(
			std::string const & iUrl,
			unsigned int const iSocketType,
			bool const iBind);
	~Receiver();

	RawMessage receive();

private:

	zmq::context_t * _zmqContext;
	zmq::socket_t * _zmqSocket;
	log4cxx::LoggerPtr  _logger;

};

}}

