#include "orwell/Application.hpp"
#include "orwell/proxy/AgentProxy.hpp"
#include "orwell/Server.hpp"

#include "orwell/com/RawMessage.hpp"

#include "controller.pb.h"

#include "Common.hpp"

#include <gtest/gtest.h>

#include <boost/lexical_cast.hpp>

#include <sys/types.h>


uint32_t const gGameDuration = 10;

class TestAgentProxyJson: public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
	}

	static void TearDownTestSuite()
	{
		orwell::Application::GetInstance().stop();
	}

	TestAgentProxyJson()
		: m_application(orwell::Application::GetInstance())
		, m_agentPort(9003)
		, m_replierPort(9002)
	{

		orwell::Application::CommandLineParameters aCommandLineArguments;
		aCommandLineArguments.m_publisherPort = 9001;
		aCommandLineArguments.m_pullerPort = 9000;
		aCommandLineArguments.m_replierPort = m_replierPort;
		aCommandLineArguments.m_agentPort = m_agentPort;
		aCommandLineArguments.m_tickInterval = 500;
		aCommandLineArguments.m_gameDuration = 300;
		aCommandLineArguments.m_dryRun = true;
		aCommandLineArguments.m_broadcast = false;
		aCommandLineArguments.m_gameDuration = gGameDuration;

		Arguments aArguments = Common::GetArguments(
				aCommandLineArguments, true);
		orwell::Application::Parameters aParameters;
		orwell::Application::ReadParameters(
				aArguments.m_argc,
				aArguments.m_argv,
				aParameters);
		m_application.run(aParameters);
	}

	~TestAgentProxyJson()
	{
	}

	orwell::Application & m_application;
	uint16_t const m_agentPort;
	uint16_t const m_replierPort;
};

TEST_F(TestAgentProxyJson, Test1)
{
	TestAgent aTestAgent(m_agentPort);
	ORWELL_LOG_DEBUG("test_1");
	orwell::proxy::AgentProxy aAgentProxy(m_application);
	std::string aAgentReply;
	std::string aTeamList;
	std::string aPlayerList;
	std::string aRobotList;
	std::string const aFakeAddress("1.2.3.4");
	std::string const aTeamName = "TEAM";
	EXPECT_TRUE(aAgentProxy.step("add team " + aTeamName, aAgentReply));
	// view team {
	EXPECT_TRUE(aAgentProxy.step("view team TEAM", aAgentReply));
	std::string aExpectedTeam(R"(Team TEAM:
	score = 0 ; robots = [])");
	EXPECT_EQ(aAgentReply, aExpectedTeam) << "view team KO";
	// } view team
	EXPECT_TRUE(aAgentProxy.step("add player Player1", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("add robot Robot1 TEAM", aAgentReply));
	// list team {
	EXPECT_TRUE(aAgentProxy.step("list team", aTeamList));
	ORWELL_LOG_DEBUG("aTeamList = " << aTeamList);
	std::string aExpectedTeamList(R"(Teams:
	TEAM
)");
	ASSERT_EQ(aTeamList, aExpectedTeamList) << "list team KO";
	// } list team
	// list player {
	EXPECT_TRUE(aAgentProxy.step("list player", aPlayerList));
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	std::string aExpectedPlayerList(R"(Players:
	Player1 -> Player name = Player1 ; address =  ; robot = 
)");
	EXPECT_EQ(aPlayerList, aExpectedPlayerList) << "list player KO";
	// } list player
	// get / set player {
	EXPECT_TRUE(aAgentProxy.step("get player Player1", aAgentReply));
	ORWELL_LOG_DEBUG("Player1 = " << aAgentReply);
	{
		std::string const aExpectedPlayer(
				R"(Player name = Player1 ; address =  ; robot = )");
		EXPECT_EQ(aAgentReply, aExpectedPlayer) << "get player KO";
	}
	EXPECT_TRUE(
			aAgentProxy.step("set player Player1 address " + aFakeAddress,
				aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("get player Player1", aAgentReply));
	ORWELL_LOG_DEBUG("Player1 = " << aAgentReply);
	{
		std::string const aExpectedPlayer = Common::Replace(
				R"(Player name = Player1 ; address = %address% ; robot = )",
				std::vector< Common::Replacement >{
				Common::Replacement { "%address%", aFakeAddress }
				}
				);
		EXPECT_EQ(aAgentReply, aExpectedPlayer)
			<< "get player (with address) KO";
	}
	// } get / set player
	// list robot {
	EXPECT_TRUE(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotList(R"(Robots:
	Robot1 -> Robot name = Robot1 ; not registered ; video_url =  ; player = 
)");
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "list robot KO";
	// } list robot
	// register robot {
	EXPECT_TRUE(aAgentProxy.step("register robot Robot1", aAgentReply));
	// make sure that Robot1 is now registered
	EXPECT_TRUE(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
	Robot1 -> Robot name = Robot1 ; registered ; video_url =  ; player = 
)");
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "register KO";
	// } register robot
	// get robot {
	EXPECT_TRUE(aAgentProxy.step("get robot Robot1", aAgentReply));
	ORWELL_LOG_DEBUG("Robot1 = " << aAgentReply);
	std::string const aExpectedRobot =
		(R"(Robot name = Robot1 ; registered ; video_url =  ; player = )");
	EXPECT_EQ(aAgentReply, aExpectedRobot) << "get robot KO";
	// } get robot
	// set robot {
	EXPECT_TRUE(aAgentProxy.step("set robot Robot1 video_url titi", aAgentReply));
	// } set robot

	orwell::messages::Hello aHelloMessage;
	aHelloMessage.set_name("PlayerWithAddress");
	aHelloMessage.set_address(aFakeAddress);
	orwell::com::RawMessage aMessage(
			"randomid", "Hello", aHelloMessage.SerializeAsString());
	std::string const aRequesterUrl =
		"tcp://127.0.0.1:" + std::to_string(m_replierPort);
	ORWELL_LOG_INFO("create requester");
	zmq::context_t aContext;
	orwell::com::Socket aRequester(
			aRequesterUrl,
			ZMQ_REQ,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	aRequester.send(aMessage);

	// make the application handle the Hello message
	m_application.loopUntilOneMessageIsProcessed();
	orwell::com::RawMessage aResponse;
	aRequester.receive(aResponse, true);
	EXPECT_EQ(aResponse._type, "Welcome")
		<< "The response to Hello should be Welcome";
	std::string aPlayerAddress;
	EXPECT_TRUE(aAgentProxy.step(
				"get player PlayerWithAddress address", aPlayerAddress));
	EXPECT_EQ(aPlayerAddress, aFakeAddress);
	EXPECT_TRUE(aAgentProxy.step(
				"remove player PlayerWithAddress", aAgentReply));

	// get / set team score {
	std::string const aScore = "2";
	EXPECT_TRUE(aAgentProxy.step("get team " + aTeamName + " score 0", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("set team " + aTeamName + " score " + aScore, aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("get team " + aTeamName + " score " + aScore, aAgentReply));
	EXPECT_EQ(aScore, aAgentReply);
	// } get / set team score
	// unregister robot {
	EXPECT_TRUE(aAgentProxy.step("unregister robot Robot1", aAgentReply));
	// make sure that Robot1 is now unregistered
	EXPECT_TRUE(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
	Robot1 -> Robot name = Robot1 ; not registered ; video_url = titi ; player = 
)");
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "unregister KO";
	// } unregister robot
	EXPECT_TRUE(aAgentProxy.step("start game", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("stop game", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("remove robot Robot1", aAgentReply));
	// add robot with space in the name {
	EXPECT_TRUE(aAgentProxy.step("add robot \"Robot One\" TEAM", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotListWithSpace(R"(Robots:
	Robot One -> Robot name = Robot One ; not registered ; video_url =  ; player = 
)");
	EXPECT_EQ(aRobotList, aExpectedRobotListWithSpace) << "list robot KO";
	EXPECT_TRUE(aAgentProxy.step("remove robot \"Robot One\"", aAgentReply));
	// } add robot with space in the name
	// view team {
	EXPECT_TRUE(aAgentProxy.step("view team TEAM", aAgentReply));
	aExpectedTeam = R"(Team TEAM:
	score = 0 ; robots = ["Robot1", "Robot One"])";
	EXPECT_EQ(aAgentReply, aExpectedTeam) << "view team KO";
	// } view team

	EXPECT_TRUE(aAgentProxy.step("remove player Player1", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("remove team TEAM", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("list team", aTeamList));
	ORWELL_LOG_DEBUG("aTeamList = " << aTeamList);
	aExpectedTeamList = (R"(Teams:
)");
	EXPECT_EQ(aTeamList, aExpectedTeamList) << "empty team KO";
	EXPECT_TRUE(aAgentProxy.step("list player", aPlayerList));
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	aExpectedPlayerList = (R"(Players:
)");
	EXPECT_EQ(aPlayerList, aExpectedPlayerList) << "empty player KO";
	EXPECT_TRUE(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
)");
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "empty robot KO";
	// get and set game duration {
	EXPECT_TRUE(aAgentProxy.step("get game duration", aAgentReply));
	ASSERT_EQ(aAgentReply, std::to_string(gGameDuration));
	std::string const aNewGameDuration = "30";
	EXPECT_TRUE(aAgentProxy.step("set game duration " + aNewGameDuration, aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("get game duration", aAgentReply));
	ASSERT_EQ(aNewGameDuration, aAgentReply);
	EXPECT_TRUE(aAgentProxy.step("get game", aAgentReply));
	std::string const aExpectedGame = Common::Replace(
			R"(Game time = %time% ; running = 0 ; duration = %duration%)",
			std::vector< Common::Replacement >{
			Common::Replacement { "%duration%", aNewGameDuration },
			Common::Replacement { "%time%", aNewGameDuration }
			}
			);
	ASSERT_EQ(aExpectedGame, aAgentReply);
	// } get and set game duration
	EXPECT_TRUE(aAgentProxy.step("stop application", aAgentReply));
}

int main(int argc, char **argv)
{
	return RunTest(argc, argv, "test_agent_proxy");
}
