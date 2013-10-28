#pragma once

//tasks
#include "InterfaceProcess.hpp"

#include <string>

#include <log4cxx/logger.h>


namespace orwell {

namespace tasks {

class GlobalContext;

class ProcessTimer : public InterfaceProcess
{
public:
    ProcessTimer( GlobalContext & ioCtx, log4cxx::LoggerPtr iLogger = log4cxx::Logger::getLogger("orwell.log") );
    ~ProcessTimer();

    void execute();


private:
    log4cxx::LoggerPtr _logger;

};

}} //namespaces

