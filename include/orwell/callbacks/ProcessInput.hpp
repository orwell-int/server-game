#pragma once

//protobuf
#include "controller.pb.h"

//tasks
#include "InterfaceProcess.hpp"

#include <log4cxx/logger.h>

namespace orwell {

namespace messages {
	class Input;
}
namespace game {
	class Game;
}

namespace callbacks {

class ProcessInput : public InterfaceProcess
{
public:
    ProcessInput(std::string const & iDest,
    		messages::Input const & iInputMsg,
    		game::Game & ioCtx,
    		std::shared_ptr< com::Sender > ioPublisher);
    ~ProcessInput();

    void execute();


private:
    std::string _dest;
    messages::Input const & _input ;
    log4cxx::LoggerPtr _logger;


};

}} //namespaces