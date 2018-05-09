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
#include "orwell/callbacks/ProcessPing.hpp"
#include "orwell/callbacks/ProcessPong.hpp"
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
	bool aSuccess = aMessage.ParsePartialFromString(iMessage._payload);
	ORWELL_LOG_DEBUG("BuildProtobuf success : " << aSuccess);
	ORWELL_LOG_DEBUG("Resulting protobuf : size = " << aMessage.ByteSize());
	return aMessage;
}

ProcessDecider::ProcessDecider(
		game::Game & ioGame,
		std::shared_ptr< com::Sender > ioPublisher,
		std::shared_ptr< com::Socket > ioReplier)
{
	_map["Hello"] = std::unique_ptr<InterfaceProcess>(new ProcessHello(ioGame, ioPublisher, ioReplier));
	_map["Input"] = std::unique_ptr<InterfaceProcess>(new ProcessInput(ioGame, ioPublisher, ioReplier));
	_map["Register"] = std::unique_ptr<InterfaceProcess>(new ProcessRegister(ioGame, ioPublisher, ioReplier));
	_map["ServerRobotState"] = std::unique_ptr<InterfaceProcess>(new ProcessRobotState(ioGame, ioPublisher, ioReplier));
	_map["Ping"] = std::unique_ptr<InterfaceProcess>(new ProcessPing(ioGame, ioPublisher, ioReplier));
	_map["Pong"] = std::unique_ptr<InterfaceProcess>(new ProcessPong(ioGame, ioPublisher, ioReplier));
}

ProcessDecider::~ProcessDecider()
{
}

void ProcessDecider::process(
		com::RawMessage const & iMessage,
		com::Channel const iChannel)
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
	else if (iMessage._type == "ServerRobotState")
	{
		aMsg = new messages::ServerRobotState(BuildProtobuf<messages::ServerRobotState>(iMessage));
	}
	else if (iMessage._type == "Ping")
	{
		aMsg = new messages::Ping(BuildProtobuf<messages::Ping>(iMessage));
	}
	else if (iMessage._type == "Pong")
	{
		aMsg = new messages::Pong(BuildProtobuf<messages::Pong>(iMessage));
	}
	else if (iMessage._type == "Register")
	{
		aMsg = new messages::Register(BuildProtobuf<messages::Register>(iMessage));
	}

	if (aMsg != nullptr && aProcess != nullptr)
	{
		aProcess->insertArgument("RoutingID", iMessage._routingId);
		aProcess->insertArgument("Type", iMessage._type);
		aProcess->init(aMsg, iChannel);
		aProcess->execute();

		delete aMsg;
	}
	else
	{
		ORWELL_LOG_INFO("unkown message type : '" << iMessage._type << "'");
	}
}

}} // namespaces

