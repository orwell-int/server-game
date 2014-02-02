#include <ProcessInput.hpp>

#include <RawMessage.hpp>

#include <controller.pb.h>
#include <server-game.pb.h>
#include "Sender.hpp"

#include "Game.hpp"

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Input;
using orwell::com::RawMessage;

namespace orwell {
namespace callbacks {

ProcessInput::ProcessInput(std::string const & iDest,
		Input const & iInputMsg,
		game::Game & ioCtx) :
InterfaceProcess(ioCtx),
_dest(iDest),
_input(iInputMsg),
_logger(log4cxx::Logger::getLogger("orwell.log"))
{
}

ProcessInput::~ProcessInput ()
{

}

void ProcessInput::execute()
{
    LOG4CXX_INFO(_logger, "ProcessInput::execute : simple relay");

    LOG4CXX_DEBUG(_logger, "===Input Message===");
    LOG4CXX_DEBUG(_logger, "Move : left=" << _input.move().left() << ",right=" <<  _input.move().right() << "-");
    LOG4CXX_DEBUG(_logger, "Fire : w1=" << _input.fire().weapon1() << ",w2=" <<  _input.fire().weapon2() << "-");
    LOG4CXX_DEBUG(_logger, "===End Input Message===");

    RawMessage aReply(_dest, "Input", _input.SerializeAsString());
    _ctx.getPublisher()->send( aReply );
}

}}
