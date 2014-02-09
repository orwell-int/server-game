#include <ProcessInput.hpp>

#include <RawMessage.hpp>

#include <controller.pb.h>
#include <server-game.pb.h>
#include "Sender.hpp"

#include "Game.hpp"

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Input;
using orwell::com::RawMessage;

namespace orwell {
namespace callbacks {

void ProcessInput::execute()
{
    std::string aDestination = getArgument("RoutingID").second;
    orwell::messages::Input const & anInputMsg = static_cast<orwell::messages::Input const &>(*_msg);
    
    LOG4CXX_INFO(_loggerPtr, "ProcessInput::execute : simple relay");

    LOG4CXX_DEBUG(_loggerPtr, "===Input Message===");
    LOG4CXX_DEBUG(_loggerPtr, "Move : left=" << anInputMsg.move().left() << ",right=" <<  anInputMsg.move().right() << "-");
    LOG4CXX_DEBUG(_loggerPtr, "Fire : w1=" << anInputMsg.fire().weapon1() << ",w2=" <<  anInputMsg.fire().weapon2() << "-");
    LOG4CXX_DEBUG(_loggerPtr, "===End Input Message===");

    RawMessage aReply(aDestination, "Input", anInputMsg.SerializeAsString());
    _ctx->getPublisher()->send( aReply );
}

}}
