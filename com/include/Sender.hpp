#pragma once

#include <string>

#include "log4cxx/logger.h"

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

    Sender(std::string const & iUrl, unsigned int const iSocketType);
    ~Sender();

    void send( std::string const & iDest, RawMessage const & iMessage );


private:

    zmq::context_t * _zmqContext;
    zmq::socket_t * _zmqSocket;
    log4cxx::LoggerPtr  _logger;
};

}}

