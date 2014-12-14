#include "orwell/callbacks/ProcessRobotState.hpp"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/game/Item.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"
#include "robot.pb.h"

using orwell::messages::ServerRobotState;
using orwell::com::RawMessage;

namespace orwell{
namespace callbacks{

ProcessRobotState::ProcessRobotState(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher, ioGame)
{
}

void ProcessRobotState::execute()
{
	std::string const & aDestination = getArgument("RoutingID").second;
	orwell::messages::ServerRobotState const & aRobotStateMsg = static_cast<orwell::messages::ServerRobotState const & >(*m_msg);

	ORWELL_LOG_INFO("ProcessRobotState::execute : simple relay");

	for (int i = 0; i < aRobotStateMsg.rfid_size() ; ++i)
	{
		switch (aRobotStateMsg.rfid(i).status())
		{
		case messages::Status::ON :
			ORWELL_LOG_INFO("Robot " << aDestination <<
					" records contact with RFID " << aRobotStateMsg.rfid(i).rfid() <<
					" at " << aRobotStateMsg.rfid(i).timestamp());
			m_game->robotIsInContactWith(aDestination, game::Item::GetItemByRfid(aRobotStateMsg.rfid(i).rfid()));
			break;
		case messages::Status::OFF :
			ORWELL_LOG_INFO("Robot " << aDestination <<
					" stops contact with RFID " << aRobotStateMsg.rfid(i).rfid() <<
					" at " << aRobotStateMsg.rfid(i).timestamp());
			m_game->robotDropsContactWith(aDestination, game::Item::GetItemByRfid(aRobotStateMsg.rfid(i).rfid()));
			break;
		}
	}
	for (int i = 0; i < aRobotStateMsg.colour_size() ; ++i)
	{
		switch (aRobotStateMsg.colour(i).status())
		{
		case messages::Status::ON :
			ORWELL_LOG_INFO("Robot " << aDestination <<
					" records contact with Colour tag " << aRobotStateMsg.colour(i).colour() <<
					" at " << aRobotStateMsg.colour(i).timestamp());
			m_game->robotIsInContactWith(aDestination, game::Item::GetItemByColor(aRobotStateMsg.colour(i).colour()));
			break;
		case messages::Status::OFF :
			ORWELL_LOG_INFO("Robot " << aDestination <<
					" stops contact with Colour tag " << aRobotStateMsg.colour(i).colour() <<
					" at " << aRobotStateMsg.colour(i).timestamp());
			m_game->robotDropsContactWith(aDestination, game::Item::GetItemByColor(aRobotStateMsg.colour(i).colour()));
			break;
		}
	}

	//todo : what do we forward to the player ?
	// forward this message to each controler
	//RawMessage aForward(aDestination, "RobotState", aRobotStateMsg.SerializeAsString());
	//m_publisher->send( aForward );
}

}}
