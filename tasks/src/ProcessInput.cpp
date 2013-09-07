#include <ProcessInput.hpp>

#include <RawMessage.hpp>

#include <controller.pb.h>
#include <server-game.pb.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Input;
using orwell::messages::Welcome;
using orwell::com::RawMessage;

namespace orwell{
namespace tasks{

ProcessInput::ProcessInput(Input const & iInputMsg, GlobalContext & ioCtx) : InterfaceProcess(ioCtx), _input(iInputMsg), _logger(log4cxx::Logger::getLogger("orwell.log"))
{

}

ProcessInput::~ProcessInput ()
{

}

void ProcessInput::execute()
{
    LOG4CXX_INFO(_logger, "ProcessInput::execute : simple relay");
    RawMessage aReply("Input", _input.SerializeAsString() );
    _ctx.getPublisher().send( "", aReply );
}

}}
