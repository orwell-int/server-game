#include "ProcessHello.hpp"

#include "RawMessage.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"

#include "Game.hpp"
#include "Player.hpp"
#include "Sender.hpp"

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


void ProcessHello::execute()
{
    LOG4CXX_INFO(_loggerPtr, "ProcessHello::execute");

    orwell::messages::Hello const & anHelloMsg = static_cast<orwell::messages::Hello const & >(*_msg);
    std::string const & aClientID = getArgument("RoutingID").second;
    
    string aNewPlayerName = anHelloMsg.name();
    bool aPlayerAddedSuccess = _ctx->addPlayer( aNewPlayerName );
    string aAvailableRobot = _ctx->getAvailableRobot();

    if ( aAvailableRobot.empty() || !aPlayerAddedSuccess )
    {
        LOG4CXX_WARN(_loggerPtr, "Impossible to process Hello : availableRobot=" << aAvailableRobot << "- player added with success :" << aPlayerAddedSuccess);

        Goodbye aGoodbye;
        RawMessage aReply(aClientID, "Goodbye", aGoodbye.SerializeAsString());
        _ctx->getPublisher()->send( aReply );
    }
    else
    {
        LOG4CXX_INFO(_loggerPtr, "Player " << aNewPlayerName << " is now linked to robot " << aAvailableRobot);

        _ctx->accessPlayer(aNewPlayerName).setRobot( aAvailableRobot );
        _ctx->accessRobot(aAvailableRobot).setPlayerName( aNewPlayerName );

        Welcome aWelcome;
        aWelcome.set_robot( aAvailableRobot );
        aWelcome.set_team( orwell::messages::RED ); //currently stupidly hardoded
        RawMessage aReply(aClientID, "Welcome", aWelcome.SerializeAsString());
        _ctx->getPublisher()->send( aReply );
    }
}


}
}

