#pragma once

//protobuf
#include <controller.pb.h>

//tasks
#include <GlobalContext.hpp>
#include <InterfaceProcess.hpp>


namespace orwell {

namespace messages {
class Hello;
}

namespace tasks {

class ProcessHello : public InterfaceProcess
{
public:
    ProcessHello(messages::Hello const & iHelloMsg, GlobalContext & ioCtx);
    ~ProcessHello ();

    void execute();


private:
    messages::Hello const & _hello ;

};

}} //namespaces
