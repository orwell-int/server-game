#include <sys/types.h>

#include <gtest/gtest.h>

#include "orwell/Application.hpp"
#include "orwell/proxy/AgentProxy.hpp"
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
		orwell::Application::GetInstance().stop();
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
	orwell::proxy::AgentProxy aAgentProxy(m_application);
	std::string aAgentReply;
	std::string aTeamList;
	std::string aPlayerList;
	std::string aRobotList;
	std::string const aFakeAddress("5.6.7.8");
	std::string const aTeamName = "TEAM";
	EXPECT_TRUE(aAgentProxy.step("add team " + aTeamName, aAgentReply));
	// json view team {
	EXPECT_TRUE(aAgentProxy.step("json view team TEAM", aAgentReply));
	std::string aExpectedTeam = R"({"Team":{"name":"TEAM","robots":[],"score":0}})";
	EXPECT_EQ(aAgentReply, aExpectedTeam) << "empty team KO";
	// } json view team
	// json get team score {
	EXPECT_TRUE(aAgentProxy.step("json get team TEAM score", aAgentReply));
	EXPECT_EQ(aAgentReply, R"({"score":0})");
	// } json get team score
	EXPECT_TRUE(aAgentProxy.step("add player Player1", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("add robot Robot1 TEAM", aAgentReply));
	// json get robot id {
	EXPECT_TRUE(aAgentProxy.step("json get robot Robot1 id", aAgentReply));
	EXPECT_EQ(aAgentReply, R"({"id":"robot_0"})");
	// } json get robot id
	// json list team {
	EXPECT_TRUE(aAgentProxy.step("json list team", aTeamList));
	ORWELL_LOG_DEBUG("aTeamList = " << aTeamList);
	std::string aExpectedTeamList(R"({"Teams":["TEAM"]})");
	ASSERT_EQ(aTeamList, aExpectedTeamList) << "json list team KO";
	// } json list team
	// json list player {
	EXPECT_TRUE(aAgentProxy.step("json list player", aPlayerList));
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	std::string aExpectedPlayerList(R"({"Players":[{"address":"","name":"Player1","robot":""}]})");
	EXPECT_EQ(aPlayerList, aExpectedPlayerList) << "json list player KO";
	// } json list player
	// json get player {
	EXPECT_TRUE(aAgentProxy.step("json get player Player1", aAgentReply));
	ORWELL_LOG_DEBUG("Player1 = " << aAgentReply);
	{
		std::string const aExpectedPlayer(R"({"address":"","name":"Player1","robot":""})");
		EXPECT_EQ(aAgentReply, aExpectedPlayer) << "json get player KO";
	}
	EXPECT_TRUE(aAgentProxy.step("set player Player1 address " + aFakeAddress, aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("json get player Player1", aAgentReply));
	ORWELL_LOG_DEBUG("Player1 = " << aAgentReply);
	{
		std::string const aExpectedPlayer = Common::Replace(
				R"({"address":"%address%","name":"Player1","robot":""})",
				std::vector< Common::Replacement >{
				Common::Replacement { "%address%", aFakeAddress },
				}
				);
		EXPECT_EQ(aAgentReply, aExpectedPlayer) << "json get player (with address) KO";
	}
	// } json get player
	// json list robot {
	EXPECT_TRUE(aAgentProxy.step("json list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotList =
		R"({"Robots":[{"id":"robot_0","name":"Robot1","player":"","registered":false,"team":"TEAM","video_url":""}]})";
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "json list robot KO";
	// } json list robot
	// register robot {
	EXPECT_TRUE(aAgentProxy.step("register robot Robot1", aAgentReply));
	// make sure that Robot1 is now registered
	EXPECT_TRUE(aAgentProxy.step("json list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList =
		R"({"Robots":[{"id":"robot_0","name":"Robot1","player":"","registered":true,"team":"TEAM","video_url":""}]})";
	EXPECT_EQ(aRobotList, aExpectedRobotList) << "register KO";
	// } register robot
	// set robot {
	EXPECT_TRUE(aAgentProxy.step("set robot Robot1 video_url titi", aAgentReply));
	// } set robot
	// unregister robot {
	EXPECT_TRUE(aAgentProxy.step("unregister robot Robot1", aAgentReply));
	// } unregister robot
	EXPECT_TRUE(aAgentProxy.step("remove robot Robot1", aAgentReply));
	// add robot with space in the name {
	EXPECT_TRUE(aAgentProxy.step("add robot \"Robot One\" TEAM", aAgentReply));
	EXPECT_TRUE(aAgentProxy.step("json list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string const aExpectedRobotListWithSpace =
		R"({"Robots":[{"id":"robot_0","name":"Robot One","player":"","registered":false,"team":"TEAM","video_url":""}]})";
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
	// get game properties {
	// Since the game has not run, time and duration are equal
	std::string const aExpectedGame = Common::Replace(
			R"({"duration":%duration%,"running":false,"time":%time%})",
			std::vector< Common::Replacement >{
			Common::Replacement { "%duration%", std::to_string(gGameDuration) },
			Common::Replacement { "%time%", std::to_string(gGameDuration) }
			}
			);
	EXPECT_TRUE(aAgentProxy.step("json get game", aAgentReply));
	EXPECT_EQ(aAgentReply, aExpectedGame);
	// } get game properties
	EXPECT_TRUE(aAgentProxy.step("stop application", aAgentReply));
}

int main(int argc, char **argv)
{
	return RunTest(argc, argv, "test_agent_proxy_json");
}
