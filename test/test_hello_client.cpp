#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "controller.pb.h"
#include "server-game.pb.h"

#include "orwell/support/GlobalLogger.hpp"
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

static void ExpectWelcome(
		std::string const & iPlayerName,
		std::string const & iExpectedRobotName,
		orwell::com::Socket & ioRequester,
		uint32_t const iMaxRetry = 0)
{
	int aLocalStatus;
	uint32_t aRetry = 0;
	do
	{
		aLocalStatus = 0;
		orwell::messages::Hello aHelloMessage;
		aHelloMessage.set_name(iPlayerName);
		orwell::com::RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
		ioRequester.send(aMessage);

		orwell::com::RawMessage aResponse;
		ioRequester.receive(aResponse, true);
		if ("Welcome" != aResponse._type)
		{
			ORWELL_LOG_ERROR(
					iPlayerName << " - Expected Welcome but received '"
					<< aResponse._type << "'");
			aLocalStatus = -1;
		}
		else
		{
			orwell::messages::Welcome aWelcome;
			aWelcome.ParsePartialFromString(aResponse._payload);

			if (aWelcome.robot() != iExpectedRobotName)
			{
				ORWELL_LOG_ERROR(
						iPlayerName << " - Expected robot name '"
						<< iExpectedRobotName << "' but received '"
						<< aWelcome.robot() << "'");
				aLocalStatus = -2;
			}
		}
	}
	while ((aLocalStatus < 0) and (++aRetry < iMaxRetry));
	if (aLocalStatus < 0)
	{
		g_status = aLocalStatus;
	}
}

static void client(
	uint16_t const iAgentPort,
	uint16_t const iPullerPort,
	uint16_t const iPublisherPort,
	uint16_t const iReplierPort)
{
	log4cxx::NDC ndc("client");
	ORWELL_LOG_INFO("client ...");
	zmq::context_t aContext(1);
	ORWELL_LOG_INFO("create subscriber");
	orwell::com::Receiver aSubscriber(
			orwell::com::Url("tcp", "localhost", iPublisherPort).toString(),
			ZMQ_SUB,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	ORWELL_LOG_INFO("create pusher");
	orwell::com::Sender aPusher(
			orwell::com::Url("tcp", "localhost", iPullerPort).toString(),
			ZMQ_PUSH,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	ORWELL_LOG_INFO("create requester");
	orwell::com::Socket aRequester(
			orwell::com::Url("tcp", "localhost", iReplierPort).toString(),
			ZMQ_REQ,
			orwell::com::ConnectionMode::CONNECT,
			aContext);

	ExpectWelcome("jambon", "Gipsy Danger", aRequester, 1);

	//this tests the case where the same player name tries to retrieve a robot 2 times
	ExpectWelcome("jambon", "Gipsy Danger", aRequester);

	ExpectWelcome("fromage", "Goldorak", aRequester);
	ExpectWelcome("poulet", "Securitron", aRequester);

	// this tests the case where there is no longer any available robot
	orwell::messages::Hello aHelloMessage2;
	aHelloMessage2.set_name("rutabagas");

	orwell::com::RawMessage aMessage2("randomid", "Hello", aHelloMessage2.SerializeAsString());
	aPusher.send(aMessage2);

	orwell::com::RawMessage aResponse2;
	if (not Common::ExpectMessage("Goodbye", aSubscriber, aResponse2))
	{
		ORWELL_LOG_ERROR("Expected Goodbye but received '" << aResponse2._type << "'");
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
	ORWELL_LOG_INFO("Test starts.");
	FakeAgentProxy aFakeAgentProxy;
	orwell::game::Ruleset aRuleset;
	FakeSystemProxy aFakeSystemProxy;
	uint16_t const aAgentPort{9003};
	uint16_t const aPullerPort{9000};
	uint16_t const aPublisherPort{9001};
	uint16_t const aReplierPort{9002};
	EXPECT_CALL(aFakeSystemProxy, mkstemp(_)).Times(0);
	EXPECT_CALL(aFakeSystemProxy, system(_)).Times(0);
	std::shared_ptr< orwell::Server > aServer =
		std::make_shared< orwell::Server >(
			aFakeSystemProxy,
			aFakeAgentProxy,
			aRuleset,
			orwell::com::Url("tcp", "*", aAgentPort).toString(),
			orwell::com::Url("tcp", "*", aPullerPort).toString(),
			orwell::com::Url("tcp", "*", aPublisherPort).toString(),
			orwell::com::Url("tcp", "*", aReplierPort).toString(),
			500);
	ORWELL_LOG_INFO("server created");
	std::vector< std::string > aRobots = {"Gipsy Danger", "Goldorak", "Securitron"};
	std::string const aTeamName("TEAM");
	aServer->accessContext().addTeam(aTeamName);
	aServer->accessContext().addRobot(aRobots[0], aTeamName, 8001, 8004, "robot1");
	aServer->accessContext().addRobot(aRobots[1], aTeamName, 8002, 8005, "robot2");
	aServer->accessContext().addRobot(aRobots[2], aTeamName, 8003, 8006, "robot3");
	std::string const aFakeUrl{"http://dummyurl.fr/8008"};
	aServer->accessContext().accessRobot(aRobots[0])->setHasRealRobot(true);
	aServer->accessContext().accessRobot(aRobots[0])->setVideoUrl(aFakeUrl);
	aServer->accessContext().accessRobot(aRobots[1])->setHasRealRobot(true);
	aServer->accessContext().accessRobot(aRobots[1])->setVideoUrl(aFakeUrl);
	aServer->accessContext().accessRobot(aRobots[2])->setHasRealRobot(true);
	aServer->accessContext().accessRobot(aRobots[2])->setVideoUrl(aFakeUrl);
	ORWELL_LOG_INFO("number of robots added: 3");
	std::thread aServerThread(server, aServer);
	std::thread aClientThread(client,
			aAgentPort,
			aPullerPort,
			aPublisherPort,
			aReplierPort);
	aClientThread.join();
	aServerThread.join();
	orwell::support::GlobalLogger::Clear();
	return g_status;
}
