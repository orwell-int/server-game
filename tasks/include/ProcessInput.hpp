#pragma once

//protobuf
#include <controller.pb.h>

//tasks
#include <GlobalContext.hpp>
#include <InterfaceProcess.hpp>

#include "log4cxx/logger.h"

namespace orwell {

namespace messages {
class Input;
}

namespace tasks {

class ProcessInput : public InterfaceProcess
{
public:
    ProcessInput(std::string const & iDest, messages::Input const & iInputMsg, GlobalContext & ioCtx);
    ~ProcessInput ();

    void execute();


private:
    std::string _dest;
    messages::Input const & _input ;
    log4cxx::LoggerPtr _logger;


};

}} //namespaces
