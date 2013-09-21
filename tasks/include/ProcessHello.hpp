#pragma once

//protobuf
#include "controller.pb.h"

//tasks
#include "GlobalContext.hpp"
#include "InterfaceProcess.hpp"

#include <string>

#include <log4cxx/logger.h>


namespace orwell {

namespace messages {
class Hello;
}

namespace tasks {

class ProcessHello : public InterfaceProcess
{
public:
    ProcessHello(std::string const & iClientId, messages::Hello const & iHelloMsg, GlobalContext & ioCtx);
    ~ProcessHello();

    void execute();


private:
    std::string _clientId;
    messages::Hello const & _hello;
    log4cxx::LoggerPtr _logger;

};

}} //namespaces

