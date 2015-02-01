#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

//#include "server-game.pb.h"
#include "robot.pb.h"

#include "orwell/Application.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Robot.hpp"

#include "Common.hpp"

#include <boost/lexical_cast.hpp>

#include <log4cxx/ndc.h>

#include <unistd.h>
#include <mutex>
#include <thread>

using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;

int g_status = 0;

static void const ProxySendsRobotState(
	int32_t iServerPullerPort,
	int32_t iServerPublisherPort,
	std::string const & iRobotId)
{
	log4cxx::NDC ndc("client");
	zmq::context_t aContext(1);

	std::string aPusherUrl = "tcp://127.0.0.1:" + boost::lexical_cast<std::string>(iServerPullerPort);
	std::string aSubscriberUrl = "tcp://127.0.0.1:" + boost::lexical_cast<std::string>(iServerPublisherPort);

	usleep(6 * 1000);
	Sender aPusher(aPusherUrl, ZMQ_PUSH, orwell::com::ConnectionMode::CONNECT, aContext);
	Receiver aSubscriber(aSubscriberUrl, ZMQ_SUB, orwell::com::ConnectionMode::CONNECT, aContext);
	usleep(6 * 1000);

	ServerRobotState aRobotState;
	Rfid * aRfidMessage = aRobotState.add_rfid();

	aRfidMessage->set_rfid("myrfidredflag");
	aRfidMessage->set_status(ON);
	aRfidMessage->set_timestamp(0);

	RawMessage aMessage(iRobotId, "ServerRobotState", aRobotState.SerializeAsString());
	aPusher.send(aMessage);

	ORWELL_LOG_INFO("batman message sent = " + aRobotState.SerializeAsString());
}

static void Application(orwell::Application::Parameters const & aParameters)
{
	orwell::Application & aApplication = orwell::Application::GetInstance();
	ORWELL_LOG_INFO("application gonna start\n");
	aApplication.run(aParameters);
}

int main()
{
	orwell::support::GlobalLogger::Create("test_capture_one_flag", "test_capture_one_flag.log", true);
	log4cxx::NDC ndc("test_capture_one_flag");

	TempFile aSrvConfFile(std::string(R"(
[server]
video-ports    = 9004-9005
)"));

	TempFile aGameConfigFile(std::string(R"(
[game]
robots = robot_A
items = item_RedFlag
ruleset = ruleset

[ruleset]
game_name = game

[robot_A]
name = toto
team = TEAM

[item_RedFlag]
name = Red Flag
type = flag
rfid = myrfidredflag
color = -1 

)"));
	ORWELL_LOG_INFO("batman ");
	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 9000;
	aCommandLineArguments.m_pullerPort = 9001;
	aCommandLineArguments.m_agentPort = 9003;
	aCommandLineArguments.m_tickInterval = 10;
	aCommandLineArguments.m_rcFilePath = aSrvConfFile.m_fileName;
	aCommandLineArguments.m_gameFilePath = aGameConfigFile.m_fileName;
	aCommandLineArguments.m_gameDuration = 274;
	aCommandLineArguments.m_dryRun = false;
	aCommandLineArguments.m_broadcast = false;

	Arguments aArguments = Common::GetArguments(
			aCommandLineArguments, true);
	orwell::Application::Parameters aParameters;
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			aParameters);
	TestAgent aTestAgent(aParameters.m_commandLineParameters.m_agentPort.get());
	usleep(100);

	std::string aReply;
	std::thread aApplicationThread(Application, aParameters);
	aTestAgent.sendCommand("ping", std::string("pong"));

	//aTestAgent.sendCommand("add team TEAM");
	//aTestAgent.sendCommand("add robot toto TEAM");
	std::string aRobotId = aTestAgent.sendCommand("get robot toto id", boost::none);
	assert("KO" != aRobotId);
	ORWELL_LOG_INFO("batman robotId = " + aRobotId);
	aTestAgent.sendCommand("get team TEAM score", std::string("0"));

	std::thread aProxySendsRobotStateThread(
			ProxySendsRobotState,
			*aParameters.m_commandLineParameters.m_pullerPort,
			*aParameters.m_commandLineParameters.m_publisherPort,
			aRobotId);
	aProxySendsRobotStateThread.join();
	 // because the game is not started yet, the Input message is supposed to be dropped by the server
//	assert(not gOK);
//	char aPath[FILENAME_MAX];
//	getcwd(aPath, sizeof(aPath));
//	std::string aFullPath = std::string(aPath) + PATH_SEPARATOR + "master.part";
//	ORWELL_LOG_INFO("Path to stream file: " + aFullPath);
//	aTestAgent.sendCommand("set robot toto video_url " + aFullPath);
//	aTestAgent.sendCommand("start game");
//	{
//		aReply = aTestAgent.sendCommand("get robot toto video_port", boost::none);
//		std::string aCommand("cd server-web && make client ARGS='-u http://127.0.0.1:" + aReply + " -l 9020 -d 8'");
//		int aCode = system(aCommand.c_str());
//		ORWELL_LOG_INFO("fake client started (return code is " << aCode << ").");
//	}
//	TestAgent aFakeClientConnector(9020);
//	aFakeClientConnector.sendCommand("ping", std::string("pong"));
//	{
//		aReply = aTestAgent.sendCommand("get robot toto video_command_port", boost::none);
//		size_t aIndex = aReply.rfind(":");
//		uint16_t aPort = boost::lexical_cast< uint16_t >(aReply.substr(aIndex + 1));
//
//		TestAgent aTestClient(aPort);
//		ORWELL_LOG_INFO("ping video server");
//		aTestClient.sendCommand("ping", std::string("pong"));
//		ClientSendsInput(
//				*aParameters.m_commandLineParameters.m_pullerPort,
//				*aParameters.m_commandLineParameters.m_publisherPort,
//				aRobotId);
//		ORWELL_LOG_INFO("make sure one image was captured.");
//		aTestClient.sendCommand("status", std::string("captured = 1"));
//	}
//
//	assert(gOK);
//	aTestAgent.sendCommand("stop game");
//	aFakeClientConnector.sendCommand("stop", std::string("stopping"));
//	ClientSendsInput(
//			*aParameters.m_commandLineParameters.m_pullerPort,
//			*aParameters.m_commandLineParameters.m_publisherPort,
//			aRobotId);
//	assert(not gOK);
	usleep(3 * *aCommandLineArguments.m_tickInterval * 1000);
	aTestAgent.sendCommand("get team TEAM score", std::string("1"));
	aTestAgent.sendCommand("stop application");
	aApplicationThread.join();
	ORWELL_LOG_INFO("Test ends\n");
	orwell::support::GlobalLogger::Clear();
	return 0;
}

