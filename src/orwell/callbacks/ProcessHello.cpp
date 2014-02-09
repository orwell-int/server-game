#include "ProcessHello.hpp"

#include "RawMessage.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"

#include "Game.hpp"
#include "Player.hpp"
#include "Sender.hpp"
#include "Robot.hpp"

#include <unistd.h>
#include <memory>

#include <log4cxx/logger.h>
#include <log4cxx/helpers/exception.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Hello;
using orwell::messages::Welcome;
using orwell::messages::Goodbye;
using orwell::com::RawMessage;
using std::string;
using std::shared_ptr;

namespace orwell{
namespace callbacks{

ProcessHello::ProcessHello(string const & iClientId,
		Hello const & iHelloMsg,
		game::Game & ioCtx,
		std::shared_ptr< com::Sender > ioPublisher) :
InterfaceProcess(ioCtx, ioPublisher),
_clientId(iClientId),
_hello(iHelloMsg),
_logger(log4cxx::Logger::getLogger("orwell.log"))
{

}

ProcessHello::~ProcessHello()
{

}

void ProcessHello::execute()
{
    LOG4CXX_INFO(_logger, "ProcessHello::execute- from player " << _hello.name());

    string aNewPlayerName = _hello.name();
    bool aPlayerAddedSuccess = _game.addPlayer( aNewPlayerName );
    shared_ptr<game::Robot> aAvailableRobot = _game.getAvailableRobot();

    if ( aAvailableRobot == NULL || !aPlayerAddedSuccess )
    {
        LOG4CXX_WARN(_logger, "Impossible to process Hello : availableRobot=" << aAvailableRobot << "- player added with success :" << aPlayerAddedSuccess);

        Goodbye aGoodbye;
        RawMessage aReply(_clientId, "Goodbye", aGoodbye.SerializeAsString());
        _publisher->send( aReply );
    }
    else
    {
        LOG4CXX_INFO(_logger, "Player " << aNewPlayerName << " is now linked to robot " << aAvailableRobot);

        _game.accessPlayer(aNewPlayerName).setRobot( aAvailableRobot->getName() );
        aAvailableRobot->setPlayerName( aNewPlayerName );

        Welcome aWelcome;
        aWelcome.set_robot( aAvailableRobot->getName() );
        aWelcome.set_team( orwell::messages::RED ); //currently stupidly hardoded
        RawMessage aReply(_clientId, "Welcome", aWelcome.SerializeAsString());
        _publisher->send( aReply );

    }
}


}
}

