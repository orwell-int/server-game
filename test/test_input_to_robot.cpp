#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "controller.pb.h"

#include "orwell/Application.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/RawMessage.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/call_traits.hpp>

#include "Common.hpp"

bool gInputReceived;

static void const ClientSendsInput(
	int32_t iServerPullerPort,
	int32_t iServerPublisherPort,
	std::string const & iRobotId)
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
	aInputMessage.mutable_fire()->set_weapon1(true);
	aInputMessage.mutable_fire()->set_weapon2(false);

	ORWELL_LOG_INFO("message built (size=" << aInputMessage.ByteSize() << ")");
	ORWELL_LOG_INFO("message built : left" << aInputMessage.move().left() << "-right" << aInputMessage.move().right());
	ORWELL_LOG_INFO("message built : w1:" << aInputMessage.fire().weapon1() << "-w2:" << aInputMessage.fire().weapon2());
	std::string aType = "Input";
	RawMessage aMessage(iRobotId, aType, aInputMessage.SerializeAsString());
	aPusher.send(aMessage);
	if (not Common::ExpectMessage(aType, aSubscriber, aMessage, 100))
	{
		gInputReceived = false;
	}
	else
	{
		gInputReceived = true;
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
	orwell::support::GlobalLogger::Create("test_input_to_robot", "test_input_to_robot.log", true);
	log4cxx::NDC ndc("test_input_to_robot");
	ORWELL_LOG_INFO("Test starts\n");
	orwell::Application::Parameters aParameters;

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 9000;
	aCommandLineArguments.m_pullerPort = 9001;
	aCommandLineArguments.m_agentPort = 9003;
	aCommandLineArguments.m_tickInterval = 10;
	aCommandLineArguments.m_gameDuration = 200;
	aCommandLineArguments.m_dryRun = false;
	aCommandLineArguments.m_broadcast = false;
	aCommandLineArguments.m_gameFilePath = "orwell-game.ini";

	Arguments aArguments = Common::GetArguments(
			aCommandLineArguments, true);
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			aParameters);
	TestAgent aTestAgent(aParameters.m_commandLineParameters.m_agentPort.get());
	usleep(100);

	std::string aReply;
	std::thread aApplicationThread(Application, aParameters);
	aTestAgent.sendCommand("ping", std::string("pong"));

	aTestAgent.sendCommand("add team TEAM");
	aTestAgent.sendCommand("add robot toto TEAM");
	std::string aRobotId = aTestAgent.sendCommand("get robot toto id", boost::none);
	assert("KO" != aRobotId);
	std::thread aClientSendsInputThread(
			ClientSendsInput,
			*aParameters.m_commandLineParameters.m_pullerPort,
			*aParameters.m_commandLineParameters.m_publisherPort,
			aRobotId);
	aClientSendsInputThread.join();
	// because the game is not started yet, the Input message is supposed to be dropped by the server
	assert(not gInputReceived);
	// The video is now going through a different channel
	// There is no solution for video captures with current architecture (nc)
	// Going back to processes started by the game server might be needed
	// For now the tests are deactivated
	aTestAgent.sendCommand("set robot toto video_url nc:fake");
	aTestAgent.sendCommand("start game");
	ClientSendsInput(
			*aParameters.m_commandLineParameters.m_pullerPort,
			*aParameters.m_commandLineParameters.m_publisherPort,
			aRobotId);

	while (not gInputReceived)
	{
		usleep(100);
	}
	assert(gInputReceived);
	aTestAgent.sendCommand("stop game");
	//aFakeClientConnector.sendCommand("stop", std::string("stopping"));
	ClientSendsInput(
			*aParameters.m_commandLineParameters.m_pullerPort,
			*aParameters.m_commandLineParameters.m_publisherPort,
			aRobotId);
	assert(not gInputReceived);
	aTestAgent.sendCommand("stop application");
	aApplicationThread.join();
	ORWELL_LOG_INFO("Test ends\n");
	orwell::support::GlobalLogger::Clear();
	return 0;
}
