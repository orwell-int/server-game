#include "orwell/callbacks/ProcessRegister.hpp"

#include "orwell/com/RawMessage.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"
#include "robot.pb.h"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/com/Sender.hpp"

#include <unistd.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Register;
using orwell::messages::Registered;
using orwell::messages::Welcome;
using orwell::messages::Goodbye;
using orwell::com::RawMessage;

namespace orwell{
namespace callbacks{

ProcessRegister::ProcessRegister(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher, ioGame)
{
}

void ProcessRegister::execute()
{
	ORWELL_LOG_INFO("ProcessRegister::execute");

	orwell::messages::Register const & aRegisterMsg =
		static_cast< orwell::messages::Register const & >(*_msg);
	std::string const & aClientID = getArgument("RoutingID").second;
    
	std::string aTemporaryRobotId = aRegisterMsg.temporary_robot_id();
	std::shared_ptr< orwell::game::Robot > aRobot =
		_game->getRobotWithoutRealRobot(aTemporaryRobotId);
	std::string aRobotId;
	if (aRobot)
	{
		aRobotId = aRobot->getRobotId();
		aRobot->setHasRealRobot(true);
		aRobot->setVideoAddress(aRegisterMsg.video_address());
		aRobot->setVideoPort(aRegisterMsg.video_port());
	}

	Registered aRegistered;
	aRegistered.set_robot_id(aRobotId);
	RawMessage aReply(aClientID, "Registered", aRegistered.SerializeAsString());
	_publisher->send(aReply);
}

}
}

