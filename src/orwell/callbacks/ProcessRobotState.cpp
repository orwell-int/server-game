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

ProcessRobotState::ProcessRobotState(std::string const & iRoutingId,
		RobotState const & iRobotStateMsg,
		game::Game & ioCtx) :
InterfaceProcess(ioCtx),
_dest(iRoutingId),
_robotState(iRobotStateMsg),
_logger(log4cxx::Logger::getLogger("orwell.log"))
{

}

ProcessRobotState::~ProcessRobotState ()
{

}

void ProcessRobotState::execute()
{
    LOG4CXX_INFO(_logger, "ProcessRobotState::execute : simple relay");

    RawMessage aForward(_dest, "RobotState", _robotState.SerializeAsString() );
    _ctx.getPublisher()->send( aForward );
}

}}
