#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

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

	RawMessage aMessage2("Idonotexist", "ServerRobotState", aRobotState.SerializeAsString());
	aPusher.send(aMessage2);
	ORWELL_LOG_INFO("message sent with wrong id = " + aRobotState.SerializeAsString());

	RawMessage aMessage(iRobotId, "ServerRobotState", aRobotState.SerializeAsString());
	aPusher.send(aMessage);
	ORWELL_LOG_INFO("batman message sent = " + aRobotState.SerializeAsString());
	RawMessage aResponse;
	bool aGotWhatExpected(false);
	while (not aGotWhatExpected)
	{
		if (not Common::ExpectMessage("GameState", aSubscriber, aResponse))
		{
			ORWELL_LOG_DEBUG("Expected gamestate but we got something else : " << aResponse._type);
		}
		else
		{
			GameState aGameState;
			aGameState.ParsePartialFromString(aResponse._payload);

			if ((not aGameState.playing()) and (aGameState.winner() == "TEAM"))
			{
				ORWELL_LOG_DEBUG("Game stopped as expected and team TEAM won.");
				aGotWhatExpected = true;
			}
		}
	}
}

static void Application(orwell::Application::Parameters const & aParameters)
{
	orwell::Application & aApplication = orwell::Application::GetInstance();
	ORWELL_LOG_INFO("application gonna start\n");
	aApplication.run(aParameters);
}


class TestParameters
{
public:
	TestParameters(
			orwell::Application::CommandLineParameters const & iCommandLineArguments);
	orwell::Application::Parameters const & getParameters() const;
private:
	orwell::Application::Parameters m_parameters;
};

TestParameters::TestParameters(
		orwell::Application::CommandLineParameters const & iCommandLineArguments)
{
	Arguments aArguments = Common::GetArguments(
			iCommandLineArguments, true);
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			m_parameters);
}

orwell::Application::Parameters const & TestParameters::getParameters() const
{
	return m_parameters;
}


class ItemTester
{
public:
	ItemTester(orwell::Application::Parameters const & aParameters);
	void run();
private:
	orwell::Application::Parameters const & m_parameters;
	TestAgent m_testAgent;
	std::thread m_applicationThread;
	std::thread m_proxySendsRobotStateThread;
};

ItemTester::ItemTester(orwell::Application::Parameters const & iParameters)
	: m_parameters(iParameters)
	, m_testAgent(iParameters.m_commandLineParameters.m_agentPort.get())
{
	std::string aReply;
	m_applicationThread = std::thread(Application, m_parameters);
	m_testAgent.sendCommand("ping", std::string("pong"));
	std::string aRobotId = m_testAgent.sendCommand("get robot toto id", boost::none);
	assert("KO" != aRobotId);
	ORWELL_LOG_INFO("batman robotId = " + aRobotId);
	m_testAgent.sendCommand("get team TEAM score", std::string("0"));
	m_proxySendsRobotStateThread = std::thread(
			ProxySendsRobotState,
			*m_parameters.m_commandLineParameters.m_pullerPort,
			*m_parameters.m_commandLineParameters.m_publisherPort,
			aRobotId);
}

void ItemTester::run()
{
	m_testAgent.sendCommand("start game");
	m_proxySendsRobotStateThread.join();
	usleep(3 * *m_parameters.m_commandLineParameters.m_tickInterval * 1000);
	m_testAgent.sendCommand("get team TEAM score", std::string("1"));
	m_testAgent.sendCommand("stop application");
	m_applicationThread.join();
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
teams = team_A
items = item_RedFlag
ruleset = ruleset
duration = 10

[ruleset]
game_name = game
points_on_capture = 1
score_to_win = 1

[team_A]
name = TEAM
robots = robot_A

[robot_A]
name = toto

[item_RedFlag]
name = Red Flag
type = flag
rfid = myrfidredflag
color = -1 

)"));
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

	TestParameters aTestParameters(aCommandLineArguments);

	ItemTester aTester(aTestParameters.getParameters());
	aTester.run();
	ORWELL_LOG_INFO("Test ends\n");
	orwell::support::GlobalLogger::Clear();
	return 0;
}

