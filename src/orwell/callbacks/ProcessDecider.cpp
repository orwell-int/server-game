/*  */

#include "orwell/callbacks/ProcessDecider.hpp"

//messages
#include "controller.pb.h"
#include "robot.pb.h"

#include <string>
#include <iostream>
#include <log4cxx/logger.h>

#include "orwell/game/Game.hpp"
#include "orwell/callbacks/ProcessHello.hpp"
#include "orwell/callbacks/ProcessInput.hpp"
#include "orwell/callbacks/ProcessRobotState.hpp"
#include <map>

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
    
ProcessDecider::~ProcessDecider()
{

}

ProcessDecider::ProcessDecider()
{
    _map["Hello"] = std::unique_ptr<InterfaceProcess>(new ProcessHello());
    _map["Input"] = std::unique_ptr<InterfaceProcess>(new ProcessInput());
    _map["RobotState"] = std::unique_ptr<InterfaceProcess>(new ProcessRobotState());
}
    
void ProcessDecider::process(com::RawMessage const & iMessage, game::Game & ioCtx)
{
    log4cxx::LoggerPtr aLogger = log4cxx::Logger::getLogger("orwell.log");
    std::unique_ptr<InterfaceProcess> & aProcess = _map[iMessage._type];
    ::google::protobuf::MessageLite * aMsg = nullptr;

    // TODO: this can be done in a proper way using pointers the right way
    if (iMessage._type == "Hello")
    {
        aMsg = new messages::Hello(BuildProtobuf<messages::Hello>(iMessage));
    }
    else if (iMessage._type == "Input")
    {
        aMsg = new messages::Input(BuildProtobuf<messages::Input>(iMessage));
    }
    else if (iMessage._type == "RobotState")
    {
        aMsg = new messages::RobotState(BuildProtobuf<messages::RobotState>(iMessage));   
    }
    
    if (aMsg != nullptr && aProcess != nullptr)
    {
        aProcess->insertArgument("RoutingID", iMessage._routingId);
        aProcess->insertArgument("Type", iMessage._type);
        aProcess->setGameContext(ioCtx);
        aProcess->init(aMsg, aLogger);
        aProcess->execute();
        
        delete aMsg;
    }
    else
    {
        LOG4CXX_INFO(aLogger, "unkown message type : " << iMessage._type << "-" );
    }
}

void ProcessDecider::Process( RawMessage const & iMessage, game::Game & ioCtx)
{
}




}} // namespaces

