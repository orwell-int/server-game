#include <ProcessTimer.hpp>

#include <RawMessage.hpp>
#include <Game.hpp>
#include "Sender.hpp"

#include <server-game.pb.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::com::RawMessage;
using orwell::messages::GameState;

namespace orwell{
namespace callbacks{

ProcessTimer::ProcessTimer(game::Game & ioCtx,
		std::shared_ptr< com::Sender > ioPublisher,
		log4cxx::LoggerPtr iLogger) :
InterfaceProcess(ioCtx, ioPublisher), _logger(iLogger)
{
}

ProcessTimer::~ProcessTimer ()
{
}

void ProcessTimer::execute()
{
    LOG4CXX_DEBUG(_logger, "ProcessTimer::execute : broadcast Gamestate");

    GameState aGameState;
    aGameState.set_playing(false);

    RawMessage aMessage("all_clients", "GameState", aGameState.SerializeAsString());
    _publisher->send( aMessage );
}

}}
