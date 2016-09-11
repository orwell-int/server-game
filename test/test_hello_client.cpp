#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "controller.pb.h"
#include "server-game.pb.h"

#include "orwell/support/GlobalLogger.hpp"
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

using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using namespace std;

int g_status = 0;

static void ExpectWelcome(
		string const & iPlayerName,
		string const & iExpectedRobotName,
		Sender & ioPusher,
		Receiver & ioSubscriber)
{
	Hello aHelloMessage;
	aHelloMessage.set_name( iPlayerName );
	RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
	ioPusher.send(aMessage);

	RawMessage aResponse;
	if ( not Common::ExpectMessage("Welcome", ioSubscriber, aResponse) )
	{
		ORWELL_LOG_ERROR("Expected Welcome but received " << aResponse._type);
		g_status = -1;
	}
	else
	{
		Welcome aWelcome;
		aWelcome.ParsePartialFromString(aResponse._payload);

		if ( aWelcome.robot() != iExpectedRobotName )
		{
			ORWELL_LOG_ERROR("Expected robot name '" << iExpectedRobotName
					<< "' but received '" << aWelcome.robot() << "'");
			g_status = -2;
		}
	}
}

static void client()
{
	log4cxx::NDC ndc("client");
	ORWELL_LOG_INFO("client ...");
	zmq::context_t aContext(1);
	usleep(6 * 1000);
	ORWELL_LOG_INFO("create pusher");
	Sender aPusher("tcp://127.0.0.1:9000", ZMQ_PUSH, orwell::com::ConnectionMode::CONNECT, aContext);
	ORWELL_LOG_INFO("create subscriber");
	Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, orwell::com::ConnectionMode::CONNECT, aContext);
	usleep(6 * 1000);

	ExpectWelcome("jambon", "Gipsy Danger", aPusher, aSubscriber);

	//this tests the case where the same player name tries to retrieve a robot 2 times
	ExpectWelcome("jambon", "Gipsy Danger", aPusher, aSubscriber);

	ExpectWelcome("fromage", "Goldorak", aPusher, aSubscriber);
	ExpectWelcome("poulet", "Securitron", aPusher, aSubscriber);

	// this tests the case where there is no longer any available robot
	Hello aHelloMessage2;
	aHelloMessage2.set_name("rutabagas");

	RawMessage aMessage2("randomid", "Hello", aHelloMessage2.SerializeAsString());
	aPusher.send(aMessage2);

	RawMessage aResponse2;
	if ( not Common::ExpectMessage("Goodbye", aSubscriber, aResponse2) )
	{
		ORWELL_LOG_ERROR("Expected Goodbye but received " << aResponse2._type);
		g_status = -1;
	}
	ORWELL_LOG_INFO("quit client");
}


static void const server(std::shared_ptr< orwell::Server > ioServer)
{
	log4cxx::NDC ndc("server");
	ORWELL_LOG_INFO("server ...");
	for (int i = 0 ; i < 5 ; ++i)
	{
		ORWELL_LOG_INFO("server loop " << i);
		ioServer->loopUntilOneMessageIsProcessed();
	}
	usleep(100 * 1000);
	ioServer->accessContext().stop();
	ORWELL_LOG_INFO("quit server");
}

int main()
{
	using ::testing::_;
	orwell::support::GlobalLogger::Create("test_hello_client", "test_hello_client.log");
	log4cxx::NDC ndc("test_hello_client");
	FakeAgentProxy aFakeAgentProxy;
	orwell::game::Ruleset aRuleset;
	FakeSystemProxy aFakeSystemProxy;
	EXPECT_CALL(aFakeSystemProxy, mkstemp(_)).Times(0);
	EXPECT_CALL(aFakeSystemProxy, system(_)).Times(0);
	std::shared_ptr< orwell::Server > aServer =
		std::make_shared< orwell::Server >(
			aFakeSystemProxy,
			aFakeAgentProxy,
			aRuleset,
			"tcp://*:9003",
			"tcp://*:9000",
			"tcp://*:9001",
			500);
	ORWELL_LOG_INFO("server created");
	std::vector< std::string > aRobots = {"Gipsy Danger", "Goldorak", "Securitron"};
	std::string const aTeamName("TEAM");
	aServer->accessContext().addTeam(aTeamName);
	aServer->accessContext().addRobot(aRobots[0], aTeamName, 8001, 8004, "robot1");
	aServer->accessContext().addRobot(aRobots[1], aTeamName, 8002, 8005, "robot2");
	aServer->accessContext().addRobot(aRobots[2], aTeamName, 8003, 8006, "robot3");
	aServer->accessContext().accessRobot(aRobots[0])->setHasRealRobot(true);
	aServer->accessContext().accessRobot(aRobots[0])->setVideoUrl("http://dummyurl.fr/8008");
	aServer->accessContext().accessRobot(aRobots[1])->setHasRealRobot(true);
	aServer->accessContext().accessRobot(aRobots[1])->setVideoUrl("http://dummyurl.fr/8008");
	aServer->accessContext().accessRobot(aRobots[2])->setHasRealRobot(true);
	aServer->accessContext().accessRobot(aRobots[2])->setVideoUrl("http://dummyurl.fr/8008");
	ORWELL_LOG_INFO("number of robots added: 3");
	std::thread aServerThread(server, aServer);
	std::thread aClientThread(client);
	aClientThread.join();
	aServerThread.join();
	orwell::support::GlobalLogger::Clear();
	return g_status;
}

