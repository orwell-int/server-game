#include <zmq.hpp>

#include <sys/types.h>
#include <cassert>

#include <boost/lexical_cast.hpp>

#include <log4cxx/ndc.h>

#include <gtest/gtest.h>

#include "orwell/Application.hpp"
#include "orwell/AgentProxy.hpp"
#include "orwell/Server.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/Url.hpp"

#include "Common.hpp"

bool gOK;

static void test_1(orwell::Application & ioApplication)
{
	TestAgent aTestAgent(9003);
	gOK = false;
	ORWELL_LOG_DEBUG("test_1");
	orwell::AgentProxy aAgentProxy(ioApplication);
	std::string aAgentReply;
	std::string aTeamList;
	std::string aPlayerList;
	std::string aRobotList;
	assert(aAgentProxy.step("add team TEAM", aAgentReply));
	assert(aAgentProxy.step("add player Player1", aAgentReply));
	assert(aAgentProxy.step("add robot Robot1 TEAM", aAgentReply));
	// list team {
	assert(aAgentProxy.step("list team", aTeamList));
	ORWELL_LOG_DEBUG("aTeamList = " << aTeamList);
	std::string aExpectedTeamList(R"(Teams:
	TEAM
)");
	ORWELL_ASSERT(aExpectedTeamList, aTeamList, "list team KO");
	// } list team
	// list player {
	assert(aAgentProxy.step("list player", aPlayerList));
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	std::string aExpectedPlayerList(R"(Players:
	Player1 -> name = Player1 ; robot = 
)");
	ORWELL_ASSERT(aExpectedPlayerList, aPlayerList, "list player KO");
	// } list player
	// list robot {
	assert(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotList(R"(Robots:
	Robot1 -> name = Robot1 ; not registered ; video_url =  ; player = 
)");
	ORWELL_ASSERT(aExpectedRobotList, aRobotList, "list robot KO");
	// } list robot
	// register robot {
	assert(aAgentProxy.step("register robot Robot1", aAgentReply));
	// make sure that Robot1 is now registered
	assert(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
	Robot1 -> name = Robot1 ; registered ; video_url =  ; player = 
)");
	ORWELL_ASSERT(aExpectedRobotList, aRobotList, "register KO");
	// } register robot
	// set robot {
	assert(aAgentProxy.step("set robot Robot1 video_url titi", aAgentReply));
	// } set robot
	// unregister robot {
	assert(aAgentProxy.step("unregister robot Robot1", aAgentReply));
	// make sure that Robot1 is now unregistered
	assert(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
	Robot1 -> name = Robot1 ; not registered ; video_url = titi ; player = 
)");
	ORWELL_ASSERT(aExpectedRobotList, aRobotList, "unregister KO");
	// } unregister robot
	assert(aAgentProxy.step("start game", aAgentReply));
	assert(aAgentProxy.step("stop game", aAgentReply));
	assert(aAgentProxy.step("remove robot Robot1", aAgentReply));
	// add robot with space in the name {
	assert(aAgentProxy.step("add robot \"Robot One\" TEAM", aAgentReply));
	assert(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotListWithSpace(R"(Robots:
	Robot One -> name = Robot One ; not registered ; video_url =  ; player = 
)");
	ORWELL_ASSERT(aExpectedRobotListWithSpace, aRobotList, "list robot KO");
	assert(aAgentProxy.step("remove robot \"Robot One\"", aAgentReply));
	// } add robot with space in the name
	assert(aAgentProxy.step("remove player Player1", aAgentReply));
	assert(aAgentProxy.step("remove team TEAM", aAgentReply));
	assert(aAgentProxy.step("list team", aTeamList));
	ORWELL_LOG_DEBUG("aTeamList = " << aTeamList);
	aExpectedTeamList = (R"(Teams:
)");
	ORWELL_ASSERT(aExpectedTeamList, aTeamList, "empty team KO");
	assert(aAgentProxy.step("list player", aPlayerList));
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	aExpectedPlayerList = (R"(Players:
)");
	ORWELL_ASSERT(aExpectedPlayerList, aPlayerList, "empty player KO");
	assert(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
)");
	ORWELL_ASSERT(aExpectedRobotList, aRobotList, "empty robot KO");
	assert(aAgentProxy.step("stop application", aAgentReply));
	gOK = true;
}


int main()
{
	orwell::support::GlobalLogger::Create("test_agent_proxy", "test_agent_proxy.log", true);
	log4cxx::NDC ndc("test_agent_proxy");
	ORWELL_LOG_INFO("Test starts\n");
	{
		orwell::Application & aApplication = orwell::Application::GetInstance();

		orwell::Application::CommandLineParameters aCommandLineArguments;
		aCommandLineArguments.m_publisherPort = 9001;
		aCommandLineArguments.m_pullerPort = 9000;
		aCommandLineArguments.m_agentPort = 9003;
		aCommandLineArguments.m_tickInterval = 500;
		aCommandLineArguments.m_gameDuration = 300;
		aCommandLineArguments.m_dryRun = true;
		aCommandLineArguments.m_broadcast = false;

		Arguments aArguments = Common::GetArguments(
				aCommandLineArguments, true);
		orwell::Application::Parameters aParameters;
		orwell::Application::ReadParameters(
				aArguments.m_argc,
				aArguments.m_argv,
				aParameters);
		aApplication.run(aParameters);
		//usleep(40 * 1000); // sleep for 0.040 s
		test_1(aApplication);
		assert(gOK);
	}
	orwell::support::GlobalLogger::Clear();
	return 0;
}
