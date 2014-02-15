#include <ProcessRobotState.hpp>

#include <Game.hpp>
#include <RawMessage.hpp>
#include "Sender.hpp"

#include <controller.pb.h>
#include <server-game.pb.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::RobotState;
using orwell::com::RawMessage;

namespace orwell{
namespace callbacks{

void ProcessRobotState::execute()
{
    std::string const & aDestination = getArgument("RoutingID").second;
    orwell::messages::RobotState const & aRobotStateMsg = static_cast<orwell::messages::RobotState const & >(*_msg);
    
    LOG4CXX_INFO(_loggerPtr, "ProcessRobotState::execute : simple relay");

    RawMessage aForward(aDestination, "RobotState", aRobotStateMsg.SerializeAsString() );
    _ctx->getPublisher()->send( aForward );
}

}}
