#include <string>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/Application.hpp"
#include "orwell/game/Game.hpp"

#include "Common.hpp"
#include "ItemCommon.hpp"

#include <log4cxx/ndc.h>

using namespace log4cxx;

using namespace orwell::com;

int main()
{
	orwell::support::GlobalLogger::Create(
		"test_capture_one_multiflag", "test_capture_one_multiflag.log", true);
	log4cxx::NDC ndc("test_capture_one_multiflag");

	TempFile aSrvConfFile(std::string(R"(
[server]
video-ports    = 9004-9005
)"));

	TempFile aGameConfigFile(std::string(R"(
[game]
teams = team_A
items = item_RedFlag item_GreenFlag
ruleset = ruleset
duration = 10

[ruleset]
game_name = game
points_on_capture = 1
score_to_win = 2

[team_A]
name = TEAM
robots = robot_A

[robot_A]
name = toto

[item_RedFlag]
name = Red Flag
type = flag
rfid = A B C
colour = -1 

[item_GreenFlag]
name = Freen Flag
type = flag
rfid = XX
colour = -1 

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

	ItemTester aTester(
			aTestParameters.getParameters(),
			{{"A", 1, false}, {"B", 2, false}, {"XX", 3, true}},
			2);
	aTester.run();
	ORWELL_LOG_INFO("Test ends\n");
	orwell::support::GlobalLogger::Clear();
	return 0;
}

