/*  */

#include "orwell/callbacks/ProcessDecider.hpp"

//messages
#include "controller.pb.h"
#include "robot.pb.h"

#include <string>
#include <iostream>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/callbacks/ProcessHello.hpp"
#include "orwell/callbacks/ProcessInput.hpp"
#include "orwell/callbacks/ProcessRegister.hpp"
#include "orwell/callbacks/ProcessRobotState.hpp"
#include <map>

using std::string;
using std::endl;
using std::cout;
using orwell::com::RawMessage;

namespace orwell {
namespace callbacks {

template <typename MessageType>
static MessageType BuildProtobuf(
		RawMessage const & iMessage)
{
	MessageType aMessage;
	bool aSuccess = aMessage.ParsePartialFromString( iMessage._payload );
	ORWELL_LOG_DEBUG("BuildProtobuf success : " << aSuccess << "-" );
	ORWELL_LOG_DEBUG("Resulting protobuf : size=" << aMessage.ByteSize() << "-" );
	return aMessage;
}

ProcessDecider::ProcessDecider(
		game::Game & ioGame,
		std::shared_ptr< com::Sender > ioPublisher)
{
	_map["Hello"] = std::unique_ptr<InterfaceProcess>(new ProcessHello(ioPublisher, ioGame));
	_map["Input"] = std::unique_ptr<InterfaceProcess>(new ProcessInput(ioPublisher, ioGame));
	_map["Register"] = std::unique_ptr<InterfaceProcess>(new ProcessRegister(ioPublisher, ioGame));
	_map["RobotState"] = std::unique_ptr<InterfaceProcess>(new ProcessRobotState(ioPublisher, ioGame));
}

ProcessDecider::~ProcessDecider()
{
}

void ProcessDecider::process(com::RawMessage const & iMessage)
{
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
	else if (iMessage._type == "Register")
	{
		aMsg = new messages::Register(BuildProtobuf<messages::Register>(iMessage));
	}

	if (aMsg != nullptr && aProcess != nullptr)
	{
		aProcess->insertArgument("RoutingID", iMessage._routingId);
		aProcess->insertArgument("Type", iMessage._type);
		aProcess->init(aMsg);
		aProcess->execute();

		delete aMsg;
	}
	else
	{
		ORWELL_LOG_INFO("unkown message type : " << iMessage._type << "-");
	}
}

}} // namespaces

