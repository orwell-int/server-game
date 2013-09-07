#pragma once

namespace orwell {
namespace tasks {

class GlobalContext;

class InterfaceProcess
{

public:
    InterfaceProcess(GlobalContext & ioCtx);
    virtual ~InterfaceProcess ();

    virtual void execute() = 0;


protected:
    GlobalContext & _ctx;

};

}} //namespaces
