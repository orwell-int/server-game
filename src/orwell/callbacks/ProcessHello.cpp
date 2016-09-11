#include "orwell/callbacks/ProcessHello.hpp"

#include "orwell/com/RawMessage.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/com/Sender.hpp"

#include <unistd.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Hello;
using orwell::messages::Welcome;
using orwell::messages::Goodbye;
using orwell::com::RawMessage;
using std::string;

namespace orwell
{
namespace callbacks
{

ProcessHello::ProcessHello(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher, ioGame)
{
}

void ProcessHello::execute()
{
	ORWELL_LOG_INFO("ProcessHello::execute");

	orwell::messages::Hello const & anHelloMsg = static_cast<orwell::messages::Hello const & >(*m_msg);
	std::string const & aClientID = getArgument("RoutingID");
    
	string aNewPlayerName = anHelloMsg.name();
	bool const aPlayerAddedSuccess = m_game->addPlayer( aNewPlayerName );
	bool aFailure = not aPlayerAddedSuccess;
	if (aPlayerAddedSuccess)
	{
		std::shared_ptr< ::orwell::game::Robot > aAvailableRobot = m_game->getRobotForPlayer(aNewPlayerName);
		aFailure = true;

		if (aAvailableRobot.get() != nullptr)
		{
			ORWELL_LOG_INFO( "Player " << aNewPlayerName << " is now linked to robot " << aAvailableRobot->getName() );

			std::shared_ptr< game::Player > aPlayer = m_game->accessPlayer(aNewPlayerName);
			if (nullptr != aPlayer)
			{
				aPlayer->setRobot(aAvailableRobot);
				aAvailableRobot->setPlayer(aPlayer);

				Welcome aWelcome;
				aWelcome.set_robot(aAvailableRobot->getName());
				aWelcome.set_team("team_red"); //currently stupidly hard coded
				aWelcome.set_id(aAvailableRobot->getRobotId());

				if (aAvailableRobot->getVideoUrl().find("nc:") == 0)
				{
					size_t aAddressIndex = aAvailableRobot->getVideoUrl().find(":");
					size_t aPortIndex = aAvailableRobot->getVideoUrl().find(":", aAddressIndex + 1);
					std::string const aAddress = aAvailableRobot->getVideoUrl().substr(
							aAddressIndex + 1, aPortIndex - aAddressIndex - 1);
					std::string aPortStr = aAvailableRobot->getVideoUrl().substr(aPortIndex + 1);
					ORWELL_LOG_INFO("aAddress = " << aAddress << " ; aPortStr = " << aPortStr);
					uint16_t aPort = boost::lexical_cast< uint16_t >(aPortStr);
					aWelcome.set_video_address(aAddress);
					aWelcome.set_video_port(aPort);
				}
				else
				{
					aWelcome.set_video_address("localhost");
					aWelcome.set_video_port(aAvailableRobot->getVideoRetransmissionPort());
				}

				RawMessage aReply(aClientID, "Welcome", aWelcome.SerializeAsString());
				m_publisher->send(aReply);
				aFailure = false;

				if (m_game->getAvailableRobot() == nullptr)
				{
					m_game->start();
				}
			}
			else
			{
				// there is no reason for the player not to be found
				// but we consider Goodbye would be sent.
			}
		}
		else
		{
			ORWELL_LOG_INFO("No robot available for player (" << aNewPlayerName << ")");
		}
	}
	if (aFailure)
	{
		ORWELL_LOG_WARN(
				"Impossible to process Hello ; player added with success :" << aPlayerAddedSuccess);

		Goodbye aGoodbye;
		RawMessage aReply(aClientID, "Goodbye", aGoodbye.SerializeAsString());
		m_publisher->send(aReply);
	}
}

}
}

