#include <ProcessTimer.hpp>

#include <RawMessage.hpp>
#include <GlobalContext.hpp>

#include <server-game.pb.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::com::RawMessage;
using orwell::messages::GameState;

namespace orwell{
namespace tasks{

ProcessTimer::ProcessTimer(GlobalContext & ioCtx, log4cxx::LoggerPtr iLogger) :
InterfaceProcess(ioCtx), _logger(iLogger)
{
}

ProcessTimer::~ProcessTimer ()
{
}

void ProcessTimer::execute()
{
    LOG4CXX_INFO(_logger, "ProcessTimer::execute : broadcast Gamestate");

    GameState aGameState;
    aGameState.set_playing(false);

    RawMessage aMessage("all_clients", "GameState", aGameState.SerializeAsString());
    _ctx.getPublisher()->send( aMessage );
}

}}
