#include "orwell/callbacks/ProcessTimer.hpp"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/com/Sender.hpp"

#include "server-game.pb.h"

using orwell::com::RawMessage;
using orwell::messages::GameState;

namespace orwell
{
namespace callbacks
{

ProcessTimer::ProcessTimer(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher, ioGame)
{
}

void ProcessTimer::execute()
{
	ORWELL_LOG_TRACE("ProcessTimer::execute : broadcast Gamestate");

	GameState aGameState;
	aGameState.set_playing(m_game->getIsRunning());

	if (m_game->getWinner())
	{
		aGameState.set_winner(*m_game->getWinner());
	}

	RawMessage aMessage("all_clients", "GameState", aGameState.SerializeAsString());
	m_publisher->send( aMessage );
}

}
}
