#include "orwell/callbacks/ProcessRobotState.hpp"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/com/Sender.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"
#include "robot.pb.h"

using orwell::messages::RobotState;
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
	orwell::messages::RobotState const & aRobotStateMsg = static_cast<orwell::messages::RobotState const & >(*m_msg);

	ORWELL_LOG_INFO("ProcessRobotState::execute : simple relay");

	if (not aRobotStateMsg.rfid().empty())
	{
		//game::Item::GetItemByRfid(aRobotStateMsg.rfid());
	}
	if (aRobotStateMsg.color() != -1)
	{
		// game::Item::GetItemByColor(aRobotStateMsg.color());
	}

	// forward this message to each controler
	RawMessage aForward(aDestination, "RobotState", aRobotStateMsg.SerializeAsString());
	m_publisher->send( aForward );
}

}}
