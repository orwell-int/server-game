#include <ProcessHello.hpp>

#include <RawMessage.hpp>

#include <controller.pb.h>
#include <server-game.pb.h>


#include <unistd.h>

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Hello;
using orwell::messages::Welcome;
using orwell::com::RawMessage;

namespace orwell{
namespace tasks{

ProcessHello::ProcessHello(Hello const & iHelloMsg, GlobalContext & ioCtx) : InterfaceProcess(ioCtx), _hello(iHelloMsg)
{

}

ProcessHello::~ProcessHello ()
{

}

void ProcessHello::execute()
{
    Welcome aWelcome;
    aWelcome.set_robot( "42" );
    aWelcome.set_team( orwell::messages::RED );

    RawMessage aReply("Welcome", aWelcome.SerializeAsString() );
    _ctx.getPublisher().send( "", aReply );

}

}}
