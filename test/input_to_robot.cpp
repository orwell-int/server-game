#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "controller.pb.h"

#include "orwell/Application.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/RawMessage.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/call_traits.hpp>

#include "Common.hpp"

bool gOK;

static void const ClientSendsInput(int32_t iServerPullerPort, int32_t iServerPublisherPort)
{
	using namespace orwell::com;
	using namespace orwell::messages;
	log4cxx::NDC ndc("client");
	zmq::context_t aContext(1);

	std::string aPusherUrl = "tcp://127.0.0.1:" + boost::lexical_cast<std::string>(iServerPullerPort);
	std::string aSubscriberUrl = "tcp://127.0.0.1:" + boost::lexical_cast<std::string>(iServerPublisherPort);

	usleep(6 * 1000);
	Sender aPusher(aPusherUrl, ZMQ_PUSH, orwell::com::ConnectionMode::CONNECT, aContext);
	Receiver aSubscriber(aSubscriberUrl, ZMQ_SUB, orwell::com::ConnectionMode::CONNECT, aContext);
	usleep(6 * 1000);

	Input aInputMessage;
	aInputMessage.mutable_move()->set_left(1);
	aInputMessage.mutable_move()->set_right(1);
	aInputMessage.mutable_fire()->set_weapon1(false);
	aInputMessage.mutable_fire()->set_weapon2(false);

	ORWELL_LOG_INFO("message built (size=" << aInputMessage.ByteSize() << ")");
	ORWELL_LOG_INFO("message built : left" << aInputMessage.move().left() << "-right" << aInputMessage.move().right());
	ORWELL_LOG_INFO("message built : w1:" << aInputMessage.fire().weapon1() << "-w2:" << aInputMessage.fire().weapon2());
	std::string aType = "Input";
	RawMessage aMessage("TANK_0", "Input", aInputMessage.SerializeAsString());
	aPusher.send(aMessage);
	if ( not Common::ExpectMessage(aType, aSubscriber, aMessage, 100) )
	{
		gOK = false;
	}
	else
	{
		gOK = true;
	}
}

static void Application(orwell::Application::Parameters const & aParameters)
{
	orwell::Application & aApplication = orwell::Application::GetInstance();
	ORWELL_LOG_INFO("application gonna start\n");
	aApplication.run(aParameters);
}

int main()
{
	orwell::support::GlobalLogger::Create("test_input", "test_input.log", true);
	log4cxx::NDC ndc("test_input");
	ORWELL_LOG_INFO("Test starts\n");
	orwell::Application::Parameters aParameters;
	Arguments aArguments = Common::GetArguments(
			false,
			9000,
			9001,
			9004,
			boost::none,
			boost::none,
			10,
			200,
			false,
			true,
			true,
			false);
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			aParameters);
	TestAgent aTestAgent(aParameters.m_agentPort.get());
	//usleep(2000 * Common::GetWaitLoops());
	uint64_t aLoops = Common::GetWaitLoops();
	//usleep(2000 * (40 + aLoops));
	usleep(100);

	std::string aReply;
	std::thread aApplicationThread(Application, aParameters);
	aReply = aTestAgent.sendCommand("ping", "pong");

	std::thread aClientSendsInputThread(ClientSendsInput, *aParameters.m_pullerPort, *aParameters.m_publisherPort);
	aClientSendsInputThread.join();
	assert(not gOK); // because the game is not started yet, the Input message must be dropped by the server
	aReply = aTestAgent.sendCommand("start game");
	//usleep(2000 * (1 + aLoops));
	ClientSendsInput(*aParameters.m_pullerPort, *aParameters.m_publisherPort);
	assert(gOK);
	aReply = aTestAgent.sendCommand("stop game");
	//usleep(2000 * (1 + aLoops));
	ClientSendsInput(*aParameters.m_pullerPort, *aParameters.m_publisherPort);
	assert(not gOK);
	aReply = aTestAgent.sendCommand("stop application");
	aApplicationThread.join();
	ORWELL_LOG_INFO("Test ends\n");
	orwell::support::GlobalLogger::Clear();
	return 0;
}
