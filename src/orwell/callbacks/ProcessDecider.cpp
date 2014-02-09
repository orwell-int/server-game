/*  */

#include "ProcessDecider.hpp"

//messages
#include "controller.pb.h"
#include "robot.pb.h"

#include <string>
#include <iostream>
#include <log4cxx/logger.h>

#include "Game.hpp"
//#include "processRegisterRobot.hpp"
#include "ProcessHello.hpp"
#include "ProcessInput.hpp"
#include "ProcessRobotState.hpp"

using std::string;
using std::endl;
using std::cout;
using orwell::com::RawMessage;

namespace orwell {
namespace callbacks {

template <typename MessageType>
static MessageType BuildProtobuf(RawMessage const & iMessage)
{
    log4cxx::LoggerPtr aLogger = log4cxx::Logger::getLogger("orwell.log");

    MessageType aMessage;
    bool aSuccess = aMessage.ParsePartialFromString( iMessage._payload );
    LOG4CXX_DEBUG(aLogger, "BuildProtobuf success : " << aSuccess << "-" );
    LOG4CXX_DEBUG(aLogger, "Resulting protobuf : size=" << aMessage.ByteSize() << "-" );
    return aMessage;
};

void ProcessDecider::Process( RawMessage const & iMessage,
		game::Game & ioGame,
		std::shared_ptr< com::Sender > ioPublisher)
{
    log4cxx::LoggerPtr aLogger = log4cxx::Logger::getLogger("orwell.log");

    if ( iMessage._type == string("Hello") )
    {
        messages::Hello aDecodedMsg = BuildProtobuf<messages::Hello>( iMessage );
        ProcessHello aProcess (iMessage._routingId, aDecodedMsg, ioGame, ioPublisher);
        aProcess.execute();
    }
    else if ( iMessage._type == string("Input") )
    {
        messages::Input aDecodedMsg = BuildProtobuf<messages::Input>( iMessage );
        ProcessInput aProcess (iMessage._routingId, aDecodedMsg, ioGame, ioPublisher);
        aProcess.execute();
    }
    else if ( iMessage._type == string("RobotState") )
    {
        messages::RobotState aDecodedMsg = BuildProtobuf<messages::RobotState>( iMessage );
        ProcessRobotState aProcess (iMessage._routingId, aDecodedMsg, ioGame, ioPublisher);
        aProcess.execute();
    }
    else
    {
        LOG4CXX_INFO(aLogger, "unkown message type : " << iMessage._type << "-" );
    }


}




}} // namespaces

