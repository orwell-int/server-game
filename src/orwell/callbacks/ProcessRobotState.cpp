#include "orwell/callbacks/ProcessRobotState.hpp"

#include "orwell/game/Game.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/com/Sender.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"
#include "robot.pb.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

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
	orwell::messages::RobotState const & aRobotStateMsg = static_cast<orwell::messages::RobotState const & >(*_msg);

	LOG4CXX_INFO(_loggerPtr, "ProcessRobotState::execute : simple relay");

	RawMessage aForward(aDestination, "RobotState", aRobotStateMsg.SerializeAsString());
	_publisher->send( aForward );
}

}}
