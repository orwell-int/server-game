#include "orwell/callbacks/ProcessRegister.hpp"

#include <unistd.h>

#include "orwell/com/RawMessage.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"
#include "robot.pb.h"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/com/Sender.hpp"

using orwell::messages::Register;
using orwell::messages::Registered;
using orwell::messages::Welcome;
using orwell::messages::Goodbye;
using orwell::com::RawMessage;

namespace orwell
{
namespace callbacks
{

ProcessRegister::ProcessRegister(
		game::Game & ioGame,
		std::shared_ptr< com::Sender > ioPublisher,
		std::shared_ptr< com::Socket > ioReplier)
	: InterfaceProcess(ioGame, ioPublisher, ioReplier)
{
}

void ProcessRegister::execute()
{
	ORWELL_LOG_INFO("ProcessRegister::execute");

	orwell::messages::Register const & aRegisterMsg =
		static_cast< orwell::messages::Register const & >(*m_msg);
	std::string const & aClientID = getArgument("RoutingID");
    
	std::string aTemporaryRobotId = aRegisterMsg.temporary_robot_id();
	std::shared_ptr< orwell::game::Robot > aRobot =
		m_game->getRobotWithoutRealRobot(aTemporaryRobotId);
	Registered aRegistered;
	std::string aRobotId;
	if (aRobot)
	{
		aRobotId = aRobot->getRobotId();
		aRobot->setHasRealRobot(true);
		aRobot->setVideoUrl(aRegisterMsg.video_url());
		aRegistered.set_team(aRobot->getTeam().getName());
	}
	aRegistered.set_robot_id(aRobotId);
	RawMessage aReply(aClientID, "Registered", aRegistered.SerializeAsString());
	reply(aReply);
	if (aRobot)
	{
		ORWELL_LOG_INFO("ProcessRegister::execute success");
	}
	else
	{
		ORWELL_LOG_INFO("ProcessRegister::execute failure");
	}
}

}
}
