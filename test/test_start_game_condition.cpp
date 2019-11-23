#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "controller.pb.h"
#include "server-game.pb.h"

#include <sys/wait.h>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/support/ISystemProxy.hpp"
#include "orwell/com/Url.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Ruleset.hpp"

#include "Common.hpp"

#include <log4cxx/ndc.h>

#include <unistd.h>
#include <mutex>
#include <thread>

int g_status = 0;

static void ExpectGameState(
		bool const iGameStarted,
		orwell::com::Receiver & ioSubscriber)
{
	orwell::com::RawMessage aResponse;
	bool aGotWhatExpected = false;
	int const aMaxLoops = 100;
	int aCurrentLoop = 0;

	while (not aGotWhatExpected)
	{
		if (not Common::ExpectMessage("GameState", ioSubscriber, aResponse))
		{
			ORWELL_LOG_DEBUG("Expected gamestate but we got something else : " << aResponse._type);
		}
		else
		{
			orwell::messages::GameState aGameState;
			aGameState.ParsePartialFromString(aResponse._payload);

			if (aGameState.playing() == iGameStarted)
			{
				ORWELL_LOG_DEBUG("State of the game is what was expected :" << aGameState.playing());
				aGotWhatExpected = true;
			}
		}
		++aCurrentLoop;
		if (aCurrentLoop > aMaxLoops)
		{
			ORWELL_LOG_ERROR("State of the game is not what was expected");
			g_status = -1;
			break;
		}
	}
}


static void client(
		uint16_t const iPusherPort,
		uint16_t const iSubscriberPort,
		uint16_t const iFakeAgentPort)
{
	log4cxx::NDC ndc("client");
	ORWELL_LOG_INFO("client ...");
	zmq::context_t aContext(1);
	ORWELL_LOG_INFO("create pusher");
	orwell::com::Sender aPusher(
			orwell::com::Url("tcp", "localhost", iPusherPort).toString(),
			ZMQ_PUSH,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	ORWELL_LOG_INFO("create subscriber");
	orwell::com::Receiver aSubscriber(
			orwell::com::Url("tcp", "localhost", iSubscriberPort).toString(),
			ZMQ_SUB,
			orwell::com::ConnectionMode::CONNECT,
			aContext);

	TestAgent aTestAgent(iFakeAgentPort);
	ORWELL_LOG_INFO("Synchronizing ping sent");
	aTestAgent.sendCommand("ping", std::string("pong"));
	ORWELL_LOG_INFO("Synchronizing pong received");

	orwell::messages::Hello aHelloMessage;
	aHelloMessage.set_name("playername");
	aHelloMessage.set_ready(true);
	orwell::com::RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
	aPusher.send(aMessage);

	ExpectGameState(true, aSubscriber);

	aPusher.send(aMessage);

	ORWELL_LOG_INFO("quit client");
}


static void const server(
		std::shared_ptr< orwell::Server > ioServer,
		uint16_t const iFakeAgentPort)
{
	log4cxx::NDC ndc("server");
	ORWELL_LOG_INFO("server ...");
	zmq::context_t aContext(1);
	orwell::com::Socket aFakeAgentSocket(
				orwell::com::Url("tcp", "*", iFakeAgentPort).toString(),
				ZMQ_REP,
				orwell::com::ConnectionMode::BIND,
				aContext,
				0);
	std::string aPing;
	aFakeAgentSocket.receiveString(aPing, true);
	ORWELL_LOG_INFO("Synchronizing ping received");
	aFakeAgentSocket.sendString("pong");
	ORWELL_LOG_INFO("Synchronizing pong sent");
	for (int i = 0 ; i < 2 ; ++i)
	{
		usleep(3 * 1000); // sleep for 3 ms
		ORWELL_LOG_INFO("server loop " << i);
		ioServer->loopUntilOneMessageIsProcessed();
	}
	ORWELL_LOG_INFO("quit server");
}

int main()
{
	using ::testing::_;
	orwell::support::GlobalLogger::Create(
			"test_start_game_condition",
			"test_start_game_condition.log",
			true);
	log4cxx::NDC ndc("test_start_game_condition");
	uint16_t const aPusherPort(9000);
	uint16_t const aSubscriberPort(9001);
	uint16_t const aFakeAgentPort(9004);
	FakeAgentProxy aFakeAgentProxy;
	orwell::game::Ruleset aRuleset;
	aRuleset.m_scoreToWin = 1;
	FakeSystemProxy aFakeSystemProxy;
	EXPECT_CALL(aFakeSystemProxy, mkstemp(_)).Times(0);
	EXPECT_CALL(aFakeSystemProxy, system(_)).Times(0);
	std::shared_ptr< orwell::Server > aServer =
		std::make_shared< orwell::Server >(
			aFakeSystemProxy,
			aFakeAgentProxy,
			aRuleset,
			"tcp://*:9003",
			orwell::com::Url("tcp", "*", aPusherPort).toString(),
			orwell::com::Url("tcp", "*", aSubscriberPort).toString(),
			"tcp://*:9002");
	ORWELL_LOG_INFO("server created");
	std::vector< std::string > aRobots = {"Gipsy Danger"};
	std::string const aTeamName("TEAM");
	aServer->accessContext().addTeam(aTeamName);
	aServer->accessContext().addRobot(aRobots[0], aTeamName, 8001, 8004, "robot1");
	aServer->accessContext().accessRobot(aRobots[0])->setHasRealRobot(true);
	ORWELL_LOG_INFO("robot added 1");
	std::thread aServerThread(server, aServer, aFakeAgentPort);
	std::thread aClientThread(
			client,
			aPusherPort,
			aSubscriberPort,
			aFakeAgentPort);
	aClientThread.join();
	aServerThread.join();
	orwell::support::GlobalLogger::Clear();
	return g_status;
}
