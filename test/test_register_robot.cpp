#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "server-game.pb.h"
#include "robot.pb.h"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/support/ISystemProxy.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Ruleset.hpp"

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

static void ExpectRegistered(
		std::string const & iTemporaryRobotId,
		std::string const & iExpectedRobotId,
		std::string const & iExpectedTeam,
		Sender & ioPusher,
		Receiver & ioSubscriber)
{
	Register aRegisterMessage;
	aRegisterMessage.set_temporary_robot_id(iTemporaryRobotId);
	aRegisterMessage.set_video_url("http://localhost:80");
	aRegisterMessage.set_image("this is a photo of the robot.jpg");
	RawMessage aMessage(
			iTemporaryRobotId,
			"Register",
			aRegisterMessage.SerializeAsString());
	ioPusher.send(aMessage);

	RawMessage aResponse;
	if (not Common::ExpectMessage("Registered", ioSubscriber, aResponse))
	{
		ORWELL_LOG_ERROR("Expected Registered but received " << aResponse._type);
		g_status = -1;
	}
	else
	{
		Registered aRegistered;
		aRegistered.ParsePartialFromString(aResponse._payload);

		if (aRegistered.robot_id() != iExpectedRobotId)
		{
			ORWELL_LOG_ERROR("Expected robot ID '" << iExpectedRobotId
					<< "' but received '" << aRegistered.robot_id() << "'");
			g_status = -2;
		}
		if (iExpectedTeam.length() > 0)
		{
			if (aRegistered.has_team())
			{
				ORWELL_LOG_INFO("The robot will be in team: " << aRegistered.team());
				if (iExpectedTeam != aRegistered.team())
				{
					ORWELL_LOG_ERROR("Expected team '" << iExpectedTeam
							<< "' but received '" << aRegistered.team() << "'");
					g_status = -4;
				}
			}
			else
			{
				ORWELL_LOG_ERROR("Expected a team but none found.");
				g_status = -3;
			}
		}
	}
}

static void proxy()
{
	log4cxx::NDC ndc("proxy");
	ORWELL_LOG_INFO("proxy ...");
	zmq::context_t aContext(1);
	usleep(6 * 1000);
	ORWELL_LOG_INFO("create pusher");
	Sender aPusher(
			"tcp://127.0.0.1:9000",
			ZMQ_PUSH,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	ORWELL_LOG_INFO("create subscriber");
	Receiver aSubscriber(
			"tcp://127.0.0.1:9001",
			ZMQ_SUB,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	usleep(6 * 1000);

	ExpectRegistered("jambon", "robot1", "TEAM", aPusher, aSubscriber);

	// this tests the case where the same robot_id tries to register twice
	ExpectRegistered("jambon", "robot1", "TEAM", aPusher, aSubscriber);

	ExpectRegistered("fromage", "robot2", "TEAM", aPusher, aSubscriber);
	ExpectRegistered("poulet", "robot3", "TEAM", aPusher, aSubscriber);

	// this tests the case where there is no robot available
	ExpectRegistered("rutabagas", "", "", aPusher, aSubscriber);
	ORWELL_LOG_INFO("quit proxy");
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
	ORWELL_LOG_INFO("quit server");
}

int main()
{
	using ::testing::_;
	orwell::support::GlobalLogger::Create("test_register_robot", "test_register_robot.log");
	log4cxx::NDC ndc("test_register_robot");
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
	std::thread aServerThread(server, aServer);
	std::thread aClientThread(proxy);
	aClientThread.join();
	aServerThread.join();
	orwell::support::GlobalLogger::Clear();
	return g_status;
}

