/*  */

#include <ProcessDecider.hpp>

//messages
#include <controller.pb.h>
#include <robot.pb.h>

#include <string>
#include <iostream>

//#include <processRegisterRobot.hpp>
#include <ProcessHello.hpp>
#include <ProcessInput.hpp>

using std::string;
using std::endl;
using std::cout;
using orwell::com::RawMessage;

namespace orwell {
namespace tasks {

template <typename MessageType>
static MessageType BuildProtobuf(RawMessage const & iMessage)
{
    MessageType aMessage;
    aMessage.ParsePartialFromString( iMessage._payload );
    return aMessage;
};

void processDecider::Process( RawMessage const & iMessage, GlobalContext & ioCtx)
{

    if ( iMessage._type == string("Hello") )
    {
        messages::Hello aDecodedMsg = BuildProtobuf<messages::Hello>( iMessage );
        ProcessHello aProcess (aDecodedMsg, ioCtx);
        aProcess.execute();
    }
    else if ( iMessage._type == string("Input") )
    {
        messages::Input aDecodedMsg = BuildProtobuf<messages::Input>( iMessage );
        ProcessInput aProcess (aDecodedMsg, ioCtx);
        aProcess.execute();
    }
    else
    {
        cout << "Unknown message type : -" << iMessage._type << "-" << endl;
    }


}




}} // namespaces

