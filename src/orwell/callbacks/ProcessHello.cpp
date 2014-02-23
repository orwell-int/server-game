#include "orwell/callbacks/ProcessHello.hpp"

#include "orwell/com/RawMessage.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"

#include "orwell/game/Game.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/com/Sender.hpp"

#include <unistd.h>

#include <log4cxx/logger.h>
#include <log4cxx/helpers/exception.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Hello;
using orwell::messages::Welcome;
using orwell::messages::Goodbye;
using orwell::com::RawMessage;
using std::string;

namespace orwell{
namespace callbacks{

ProcessHello::ProcessHello(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher, ioGame)
{
}

void ProcessHello::execute()
{
	LOG4CXX_INFO(_loggerPtr, "ProcessHello::execute");

	orwell::messages::Hello const & anHelloMsg = static_cast<orwell::messages::Hello const & >(*_msg);
	std::string const & aClientID = getArgument("RoutingID").second;
    
	string aNewPlayerName = anHelloMsg.name();
	bool aPlayerAddedSuccess = _ctx->addPlayer( aNewPlayerName );
	string aRobotForPlayer = _ctx->getRobotNameForPlayer( aNewPlayerName );
	string aAvailableRobot;

	if ( aRobotForPlayer.empty() )
	{
		aAvailableRobot = _ctx->getAvailableRobot();
	}
	
	if ( ( aAvailableRobot.empty() and aRobotForPlayer.empty() ) || !aPlayerAddedSuccess )
	{
		LOG4CXX_WARN(_loggerPtr, "Impossible to process Hello : availableRobot=" << aAvailableRobot.get() << "- player added with success :" << aPlayerAddedSuccess);

		Goodbye aGoodbye;
		RawMessage aReply(aClientID, "Goodbye", aGoodbye.SerializeAsString());
		_publisher->send( aReply );
	}
	else
	{
		LOG4CXX_INFO(_loggerPtr, "Player " << aNewPlayerName << " is now linked to robot " << aAvailableRobot->getName());

		if (aRobotForPlayer.empty())
		{
			_ctx->accessPlayer(aNewPlayerName).setRobot( aAvailableRobot );
			_ctx->accessRobot(aAvailableRobot).setPlayerName( aNewPlayerName );
		}

		Welcome aWelcome;
		aWelcome.set_robot( aRobotForPlayer.empty() ? aAvailableRobot : aRobotForPlayer );
		aWelcome.set_team( orwell::messages::RED ); //currently stupidly hardoded
		RawMessage aReply(aClientID, "Welcome", aWelcome.SerializeAsString());
		_publisher->send( aReply );
	}
}

}
}

