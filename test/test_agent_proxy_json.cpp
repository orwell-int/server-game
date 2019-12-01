#include <sys/types.h>

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
	// json view team {
	EXPECT_TRUE(aAgentProxy.step("json view team TEAM", aAgentReply));
	std::string aExpectedTeam = R"({"Team":{"name":"TEAM","robots":[],"score":0}})";
	EXPECT_EQ(aAgentReply, aExpectedTeam) << "empty team KO";
	// } json view team
	EXPECT_TRUE(aAgentProxy.step("add player Player1", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("add robot Robot1 TEAM", aAgentReply));
	// json list team {
	EXPECT_TRUE(aAgentProxy.step("json list team", aTeamList));
	ORWELL_LOG_DEBUG("aTeamList = " << aTeamList);
	std::string aExpectedTeamList(R"({"Teams":["TEAM"]})");
	ASSERT_EQ(aTeamList, aExpectedTeamList) << "json list team KO";
	// } json list team
	// json list player {
	EXPECT_TRUE(aAgentProxy.step("json list player", aPlayerList));
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	std::string aExpectedPlayerList(R"({"Players":[{"name":"Player1","robot":""}]})");
	EXPECT_EQ(aPlayerList, aExpectedPlayerList) << "json list player KO";
	// } json list player
	// json list robot {
	EXPECT_TRUE(aAgentProxy.step("json list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotList(
			R"({"Robots":[{"name":"Robot1","player":"","registered":false,"team":"TEAM","video_url":""}]})");
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "json list robot KO";
	// } json list robot
	// register robot {
	EXPECT_TRUE(aAgentProxy.step("register robot Robot1", aAgentReply));
	// make sure that Robot1 is now registered
	EXPECT_TRUE(aAgentProxy.step("json list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList =
		R"({"Robots":[{"name":"Robot1","player":"","registered":true,"team":"TEAM","video_url":""}]})";
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "register KO";
	// } register robot
	// set robot {
	EXPECT_TRUE(aAgentProxy.step("set robot Robot1 video_url titi", aAgentReply));
	// } set robot
	// unregister robot {
	EXPECT_TRUE(aAgentProxy.step("unregister robot Robot1", aAgentReply));
	// } unregister robot
	EXPECT_TRUE(aAgentProxy.step("start game", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("stop game", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("remove robot Robot1", aAgentReply));
	// add robot with space in the name {
	EXPECT_TRUE(aAgentProxy.step("add robot \"Robot One\" TEAM", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("json list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string const aExpectedRobotListWithSpace(
			R"({"Robots":[{"name":"Robot One","player":"","registered":false,"team":"TEAM","video_url":""}]})");
	EXPECT_EQ(aRobotList, aExpectedRobotListWithSpace) << "json list robot KO";
	EXPECT_TRUE(aAgentProxy.step("remove robot \"Robot One\"", aAgentReply));
	// } add robot with space in the name
	// json view team {
	EXPECT_TRUE(aAgentProxy.step("json view team TEAM", aAgentReply));
	aExpectedTeam =
			R"({"Team":{"name":"TEAM","robots":["Robot1","Robot One"],"score":0}})";
	EXPECT_EQ(aAgentReply, aExpectedTeam) << "json view team KO";
	// } json view team
	EXPECT_TRUE(aAgentProxy.step("remove player Player1", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("remove team TEAM", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("json list team", aTeamList));
	ORWELL_LOG_DEBUG("aTeamList = " << aTeamList);
	aExpectedTeamList = (R"({"Teams":[]})");
	EXPECT_EQ(aTeamList, aExpectedTeamList) << "empty team KO";
	EXPECT_TRUE(aAgentProxy.step("json list player", aPlayerList));
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	aExpectedPlayerList = (R"({"Players":[]})");
	EXPECT_EQ(aPlayerList, aExpectedPlayerList) << "empty player KO";
	EXPECT_TRUE(aAgentProxy.step("json list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"({"Robots":[]})");
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "empty robot KO";
	EXPECT_TRUE(aAgentProxy.step("json view team TEAM", aAgentReply));
	aExpectedTeam = R"({"Team":null})";
	EXPECT_EQ(aAgentReply, aExpectedTeam) << "null team KO";
	EXPECT_TRUE(aAgentProxy.step("stop application", aAgentReply));
}

int main(int argc, char **argv)
{
	return RunTest(argc, argv, "test_agent_proxy_json");
}
