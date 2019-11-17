#include <sys/types.h>

#include <boost/lexical_cast.hpp>

#include <log4cxx/ndc.h>

#include <gtest/gtest.h>

#include "orwell/Application.hpp"
#include "orwell/AgentProxy.hpp"
#include "orwell/Server.hpp"

#include "Common.hpp"

uint32_t const gGameDuration = 10;

class TestAgentProxyJson: public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
	}

	static void TearDownTestSuite()
	{
	}

	TestAgentProxyJson()
		: m_application(orwell::Application::GetInstance())
		, m_agentPort(9003)
	{

		orwell::Application::CommandLineParameters aCommandLineArguments;
		aCommandLineArguments.m_publisherPort = 9001;
		aCommandLineArguments.m_pullerPort = 9000;
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
};

TEST_F(TestAgentProxyJson, Test1)
{
	TestAgent aTestAgent(m_agentPort);
	ORWELL_LOG_DEBUG("test_1");
	orwell::AgentProxy aAgentProxy(m_application);
	std::string aAgentReply;
	std::string aTeamList;
	std::string aPlayerList;
	std::string aRobotList;
	std::string const aTeamName = "TEAM";
	EXPECT_TRUE(aAgentProxy.step("add team " + aTeamName, aAgentReply));
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
	Player1 -> name = Player1 ; robot = 
)");
	EXPECT_EQ(aPlayerList, aExpectedPlayerList) << "list player KO";
	// } list player
	// list robot {
	EXPECT_TRUE(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotList(R"(Robots:
	Robot1 -> name = Robot1 ; not registered ; video_url =  ; player = 
)");
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "list robot KO";
	// } list robot
	// register robot {
	EXPECT_TRUE(aAgentProxy.step("register robot Robot1", aAgentReply));
	// make sure that Robot1 is now registered
	EXPECT_TRUE(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
	Robot1 -> name = Robot1 ; registered ; video_url =  ; player = 
)");
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "register KO";
	// } register robot
	// set robot {
	EXPECT_TRUE(aAgentProxy.step("set robot Robot1 video_url titi", aAgentReply));
	// } set robot
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
	Robot1 -> name = Robot1 ; not registered ; video_url = titi ; player = 
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
	Robot One -> name = Robot One ; not registered ; video_url =  ; player = 
)");
	EXPECT_EQ(aRobotList, aExpectedRobotListWithSpace) << "list robot KO";
	EXPECT_TRUE(aAgentProxy.step("remove robot \"Robot One\"", aAgentReply));
	// } add robot with space in the name
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
	// get and set game duration {
	EXPECT_TRUE(aAgentProxy.step("get game duration", aAgentReply));
	ASSERT_EQ(boost::lexical_cast< std::string >(gGameDuration), aAgentReply);
	std::string const aNewGameDuration = "30";
	EXPECT_TRUE(aAgentProxy.step("set game duration " + aNewGameDuration, aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("get game duration", aAgentReply));
	ASSERT_EQ(aNewGameDuration, aAgentReply);
	// } get and set game duration
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "empty robot KO";
	EXPECT_TRUE(aAgentProxy.step("stop application", aAgentReply));
}

int main(int argc, char **argv)
{
	orwell::support::GlobalLogger::Create(
			"test_agent_proxy", "test_agent_proxy.log", true);
	log4cxx::NDC ndc("test_agent_proxy");
	::testing::InitGoogleTest(&argc, argv);
	::testing::TestEventListeners& listeners =
		::testing::UnitTest::GetInstance()->listeners();
	// Adds a listener to the end.  googletest takes the ownership.
	listeners.Append(new MinimalistPrinter);
	return RUN_ALL_TESTS();
}
