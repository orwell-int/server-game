#pragma once

#include <string>
#include <log4cxx/logger.h>

//protobuf
#include "controller.pb.h"
#include "InterfaceProcess.hpp"

namespace orwell {

namespace messages {
	class Hello;
}
namespace game {
	class Game;
}

namespace callbacks {

class ProcessHello : public InterfaceProcess
{
public:
    ProcessHello(std::string const & iClientId,
    		messages::Hello const & iHelloMsg,
    		game::Game & ioCtx);
    ~ProcessHello();

    void execute();


private:
    std::string _clientId;
    messages::Hello const & _hello;
    log4cxx::LoggerPtr _logger;

};

}} //namespaces

