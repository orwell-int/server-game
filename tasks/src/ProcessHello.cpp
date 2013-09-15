#include <ProcessHello.hpp>

#include <RawMessage.hpp>

#include <controller.pb.h>
#include <server-game.pb.h>

#include "PlayerContext.hpp"

#include <unistd.h>

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Hello;
using orwell::messages::Welcome;
using orwell::messages::Goodbye;
using orwell::com::RawMessage;
using std::string;

namespace orwell{
namespace tasks{

ProcessHello::ProcessHello(string const & iClientId, Hello const & iHelloMsg, GlobalContext & ioCtx) :
InterfaceProcess(ioCtx), _clientId(iClientId), _hello(iHelloMsg), _logger(log4cxx::Logger::getLogger("orwell.log"))
{

}

ProcessHello::~ProcessHello()
{

}

void ProcessHello::execute()
{
    LOG4CXX_INFO(_logger, "ProcessHello::execute");

    string aNewPlayerName = _hello.name();
    bool aPlayerAddedSuccess = _ctx.addPlayer( aNewPlayerName );
    string aAvailableRobot = _ctx.getAvailableRobot();

    if ( aAvailableRobot.empty() || !aPlayerAddedSuccess )
    {
        LOG4CXX_WARN(_logger, "Impossible to process Hello : availableRobot=" << aAvailableRobot << "- player added with success :" << aPlayerAddedSuccess);

        Goodbye aGoodbye;
        RawMessage aReply(_clientId, "Goodbye", aGoodbye.SerializeAsString());
        _ctx.getPublisher()->send( aReply );
    }
    else
    {
        LOG4CXX_INFO(_logger, "Player " << aNewPlayerName << " is now linked to robot " << aAvailableRobot);

        _ctx.accessPlayer(aNewPlayerName).setRobot( aAvailableRobot );
        _ctx.accessRobot(aAvailableRobot).setPlayerName( aNewPlayerName );

        Welcome aWelcome;
        aWelcome.set_robot( aAvailableRobot );
        aWelcome.set_team( orwell::messages::RED ); //currently stupidly hardoded
        RawMessage aReply(_clientId, "Welcome", aWelcome.SerializeAsString());
        _ctx.getPublisher()->send( aReply );
    }
}


}
}

