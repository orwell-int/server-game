#include "orwell/Server.hpp"
#include "orwell/BroadcastServer.hpp"

#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <cassert>

#include <log4cxx/ndc.h>

#include "orwell/Application.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/call_traits.hpp>

#include <vector>
#include <functional>

#include "Common.hpp"


static void test_ReadParameters(
		Status const iTestStatus,
		Arguments iArguments,
		boost::optional< orwell::Application::Parameters > iExpectedParameters
			= boost::none)
{
	ORWELL_LOG_DEBUG("arguments:" << iArguments);
	orwell::Application::Parameters aParameters;
	bool result = orwell::Application::ReadParameters(
			iArguments.m_argc, iArguments.m_argv, aParameters);
	assert((Status::PASS == iTestStatus) == result);
	if (iExpectedParameters)
	{
		orwell::Application::Parameters & aExpectedParameters = *iExpectedParameters;
		if (aExpectedParameters != aParameters)
		{
			//// epic failure
			//ORWELL_LOG_DEBUG("expected parameters: " << aExpectedParameters);
			{
				std::ostringstream aStream;
				aStream << aExpectedParameters;
				ORWELL_LOG_DEBUG("expected parameters: " << aStream.str());
			}
			{
				std::ostringstream aStream;
				aStream << aParameters;
				ORWELL_LOG_DEBUG("computed parameters: " << aStream.str());
			}
			throw false;
		}
	}
}

static void test_nothing()
{
	ORWELL_LOG_DEBUG("test_nothing");
	// we get default arguments
	test_ReadParameters(Status::PASS, Common::GetArguments(orwell::Application::CommandLineParameters()));
}

static void test_wrong_port_range_publisher_1()
{
	ORWELL_LOG_DEBUG("test_wrong_port_range_publisher_1");

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 0;
	aCommandLineArguments.m_pullerPort = 42;
	aCommandLineArguments.m_agentPort = 43;

	test_ReadParameters(Status::FAIL, Common::GetArguments(aCommandLineArguments, false, false));
}

static void test_wrong_port_range_publisher_2()
{
	ORWELL_LOG_DEBUG("test_wrong_port_range_publisher_2");

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = -1024;
	aCommandLineArguments.m_pullerPort = 42;
	aCommandLineArguments.m_agentPort = 43;

	// !! careful !  Boost converts -1024 to ... something, and it passes
	test_ReadParameters(Status::PASS, Common::GetArguments(aCommandLineArguments, false, false));
}

static void test_wrong_port_range_publisher_3()
{
	ORWELL_LOG_DEBUG("test_wrong_port_range_publisher_3");

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 99999;
	aCommandLineArguments.m_pullerPort = 42;
	aCommandLineArguments.m_agentPort = 43;

	// !! careful !  Boost converts 99999 to ... something, and it passes
	test_ReadParameters(Status::PASS, Common::GetArguments(aCommandLineArguments, false, false));
}

static void test_same_ports_agent_publisher()
{
	ORWELL_LOG_DEBUG("test_same_ports_agent_publisher");

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 41;
	aCommandLineArguments.m_pullerPort = 42;
	aCommandLineArguments.m_agentPort = 41;

	test_ReadParameters(Status::FAIL, Common::GetArguments(aCommandLineArguments, false, false));
}

static void test_same_ports_puller_publisher()
{
	ORWELL_LOG_DEBUG("test_same_ports_puller_publisher");

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 41;
	aCommandLineArguments.m_pullerPort = 41;
	aCommandLineArguments.m_agentPort = 42;

	test_ReadParameters(Status::FAIL, Common::GetArguments(aCommandLineArguments, false, false));
}

static void test_same_ports_puller_agent()
{
	ORWELL_LOG_DEBUG("test_same_ports_puller_agent");

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 41;
	aCommandLineArguments.m_pullerPort = 42;
	aCommandLineArguments.m_agentPort = 42;

	test_ReadParameters(Status::FAIL, Common::GetArguments(aCommandLineArguments, false, false));
}

static void test_most_arguments()
{
	TempFile aTempFile(std::string(R"(
[server]
video-ports    = 9001
)"));

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 41;
	aCommandLineArguments.m_pullerPort = 42;
	aCommandLineArguments.m_agentPort = 43;
	aCommandLineArguments.m_rcFilePath = aTempFile.m_fileName;
	aCommandLineArguments.m_tickInterval = 666;
	aCommandLineArguments.m_gameDuration = 274;
	aCommandLineArguments.m_dryRun = false;
	aCommandLineArguments.m_broadcast = true;

	ORWELL_LOG_DEBUG("test_most_arguments");
	test_ReadParameters(
			Status::PASS,
			Common::GetArguments(
				aCommandLineArguments,
				true // debug log
			)
	);
}

static void test_parse_command_line()
{
	ORWELL_LOG_DEBUG("test_parse_command_line");

	TempFile aTempFile(std::string(R"(
[server]
video-ports    = 9001
)"));

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 1;
	aCommandLineArguments.m_pullerPort = 2;
	aCommandLineArguments.m_agentPort = 3;
	aCommandLineArguments.m_rcFilePath = aTempFile.m_fileName;
	aCommandLineArguments.m_tickInterval = 666;
	aCommandLineArguments.m_gameDuration = 274;
	aCommandLineArguments.m_dryRun = true;
	aCommandLineArguments.m_broadcast = false;

	orwell::Application::Parameters aExpectedParameters ;
	aExpectedParameters.m_commandLineParameters = aCommandLineArguments;
	aExpectedParameters.m_videoPorts = {9001};

	test_ReadParameters(
			Status::PASS,
			Common::GetArguments(
				aCommandLineArguments,
				true // debug log
			),
			aExpectedParameters);
}

#include <stdio.h>

// command line has priority over the file
static void test_parse_command_line_and_file_1()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_1");
	TempFile aTempFile(std::string(R"(
[server]
publisher-port = 19000
puller-port    = 19001
agent-port     = 19003
tic-interval   = 1500
video-ports    = 9001:9004
)"));

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 1;
	aCommandLineArguments.m_pullerPort = 2;
	aCommandLineArguments.m_agentPort = 3;
	aCommandLineArguments.m_rcFilePath = aTempFile.m_fileName;
	aCommandLineArguments.m_tickInterval = 666;
	aCommandLineArguments.m_gameDuration = 274;
	aCommandLineArguments.m_dryRun = true;
	aCommandLineArguments.m_broadcast = false;

	orwell::Application::Parameters aExpectedParameters ;
	aExpectedParameters.m_commandLineParameters = aCommandLineArguments;
	aExpectedParameters.m_videoPorts = {9004, 9003, 9002, 9001};

	test_ReadParameters(
			Status::PASS,
			Common::GetArguments(
				aCommandLineArguments,
				true // debug log
			),
			aExpectedParameters);
}

// read some values from the file only
static void test_parse_command_line_and_file_2()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_2");
	TempFile aTempFile(std::string(R"(
[server]
publisher-port = 900
puller-port    = 901
agent-port     = 903
tic-interval   = 50
video-ports    = 9001
)"));

	orwell::Application::CommandLineParameters aInputCommandLineArguments;
	aInputCommandLineArguments.m_rcFilePath = aTempFile.m_fileName;

	orwell::Application::CommandLineParameters aExpectedCommandLineArguments;
	aExpectedCommandLineArguments.m_publisherPort = 900;
	aExpectedCommandLineArguments.m_pullerPort = 901;
	aExpectedCommandLineArguments.m_agentPort = 903;
	aExpectedCommandLineArguments.m_rcFilePath = aTempFile.m_fileName;
	aExpectedCommandLineArguments.m_tickInterval = 50;
	aExpectedCommandLineArguments.m_gameDuration = 300;
	aExpectedCommandLineArguments.m_dryRun = false;
	aExpectedCommandLineArguments.m_broadcast = true;

	orwell::Application::Parameters aExpectedParameters ;
	aExpectedParameters.m_commandLineParameters = aExpectedCommandLineArguments;
	aExpectedParameters.m_videoPorts = {9001};

	test_ReadParameters(
			Status::PASS,
			Common::GetArguments(
				aInputCommandLineArguments,
				true // debug log
			),
			aExpectedParameters);
}

// make sure a file containing unrelated configuration does nothing wrong.
static void test_parse_command_line_and_file_3()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_3");
	TempFile aTempFile(std::string(R"(
[server]
video-ports = 9001
[its a trap]
puller-port = 42
)"));
	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 1;
	aCommandLineArguments.m_pullerPort = 2;
	aCommandLineArguments.m_agentPort = 3;
	aCommandLineArguments.m_rcFilePath = aTempFile.m_fileName;
	aCommandLineArguments.m_tickInterval = 666;
	aCommandLineArguments.m_gameDuration = 274;
	aCommandLineArguments.m_dryRun = true;
	aCommandLineArguments.m_broadcast = false;

	orwell::Application::Parameters aExpectedParameters ;
	aExpectedParameters.m_commandLineParameters = aCommandLineArguments;
	aExpectedParameters.m_videoPorts = {9001};

	test_ReadParameters(
			Status::PASS,
			Common::GetArguments(
				aCommandLineArguments,
				true // debug log
			),
			aExpectedParameters);
}

// check the content of the game and the different robots.
static void test_parse_command_line_and_file_4()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_4");
	TempFile aTechConfigFile(std::string(R"(
[server]
video-ports    = 9001:9004
)"));
	TempFile aTempFile(std::string(R"(
[game]
robots = robot_A | robot_B
gametype = Problem
gamename = Enigma
duration = 999

[robot_A]
name = Aristotle
team = Philosophers

[robot_B]
name = Bourbaki
team = Mathematicians

)"));

	orwell::Application::CommandLineParameters aInputCommandLineArguments;
	aInputCommandLineArguments.m_publisherPort = 1;
	aInputCommandLineArguments.m_pullerPort = 2;
	aInputCommandLineArguments.m_agentPort = 3;
	aInputCommandLineArguments.m_rcFilePath = aTechConfigFile.m_fileName;
	aInputCommandLineArguments.m_gameFilePath = aTempFile.m_fileName;
	aInputCommandLineArguments.m_tickInterval = 666;
	aInputCommandLineArguments.m_dryRun = true;
	aInputCommandLineArguments.m_broadcast = false;

	orwell::Application::CommandLineParameters aExpectedCommandLineArguments = aInputCommandLineArguments;
	aExpectedCommandLineArguments.m_gameDuration = 999;

	orwell::Application::Parameters aExpectedParameters ;
	aExpectedParameters.m_commandLineParameters = aExpectedCommandLineArguments;
	aExpectedParameters.m_gameName = "Enigma";
	aExpectedParameters.m_gameType = "Problem";
	aExpectedParameters.m_robots = {
			{"robot_A", {"Aristotle", "Philosophers"}},
			{"robot_B", {"Bourbaki", "Mathematicians"}},
		};
	aExpectedParameters.m_teams = {"Mathematicians", "Philosophers"};
	aExpectedParameters.m_videoPorts = {9004, 9003, 9002, 9001};

	test_ReadParameters(
			Status::PASS,
			Common::GetArguments(
				aInputCommandLineArguments,
				true // debug log
			),
			aExpectedParameters);
}

// make sure we detect errors
static void test_parse_command_line_and_file_5()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_5");
	TempFile aTechConfigFile(std::string(R"(
[server]
video-ports    = 9001:9004
)"));

	TempFile aTempFile(std::string(R"(
[game]
robots = robot_A | robot_B
gametype = Problem
gamename = Enigma

[robot_A]
name = Aristotle
team = Philosophers

[robot_B]
name = Bourbaki
team = Mathematicians

)"));

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 1;
	aCommandLineArguments.m_pullerPort = 2;
	aCommandLineArguments.m_agentPort = 3;
	aCommandLineArguments.m_rcFilePath = aTechConfigFile.m_fileName;
	aCommandLineArguments.m_gameFilePath = aTempFile.m_fileName;
	aCommandLineArguments.m_tickInterval = 666;
	aCommandLineArguments.m_gameDuration = 274;
	aCommandLineArguments.m_dryRun = true;
	aCommandLineArguments.m_broadcast = false;

	orwell::Application::Parameters aExpectedParameters ;
	aExpectedParameters.m_commandLineParameters = aCommandLineArguments;
	aExpectedParameters.m_gameName = "Enigma";
	aExpectedParameters.m_gameType = "Problem";
	aExpectedParameters.m_robots = {
			{"robot_A", {"Aristotle", "Philosophers"}},
			{"robot_B", {"Bourbaki", "Mistake!!"}}, // voluntary mistake
		};
	aExpectedParameters.m_teams = {"Mathematicians", "Philosophers"};
	aExpectedParameters.m_videoPorts = {9004, 9003, 9002, 9001};

	bool aThrown = false;
	try
	{
		test_ReadParameters(
				Status::PASS,
				Common::GetArguments(
					aCommandLineArguments,
					true // debug log
				),
				aExpectedParameters);
	}
	catch (bool const aBool)
	{
		ORWELL_LOG_DEBUG("Expected exception cought");
		aThrown = true;
	}
	assert(aThrown);
}

static void test_parse_command_line_and_file_6_badConfigFile()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_6_badConfigFile");

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 1;
	aCommandLineArguments.m_pullerPort = 2;
	aCommandLineArguments.m_agentPort = 3;
	aCommandLineArguments.m_rcFilePath = std::string("thisfiledoesnotexist");
	aCommandLineArguments.m_tickInterval = 666;
	aCommandLineArguments.m_gameDuration = 274;
	aCommandLineArguments.m_dryRun = true;
	aCommandLineArguments.m_broadcast = false;

	test_ReadParameters(
			Status::FAIL,
			Common::GetArguments(
					aCommandLineArguments,
					true // debug log
			));
}

static void test_parse_command_line_and_file_7_moreRobotsThanVideoPorts()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_7_moreRobotsThanVideoPorts");
	TempFile aTechConfigFile(std::string(R"(
[server]
video-ports    = 9001
)"));

	TempFile aGameConfigFile(std::string(R"(
[game]
robots = robot_A | robot_B
gametype = Problem
gamename = Enigma

[robot_A]
name = Aristotle
team = Philosophers

[robot_B]
name = Bourbaki
team = Mathematicians
)"));

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 1;
	aCommandLineArguments.m_pullerPort = 2;
	aCommandLineArguments.m_agentPort = 3;
	aCommandLineArguments.m_rcFilePath = aTechConfigFile.m_fileName;
	aCommandLineArguments.m_gameFilePath = aGameConfigFile.m_fileName;
	aCommandLineArguments.m_tickInterval = 666;
	aCommandLineArguments.m_gameDuration = 274;
	aCommandLineArguments.m_dryRun = true;
	aCommandLineArguments.m_broadcast = false;

	test_ReadParameters(
			Status::FAIL,
			Common::GetArguments(
				aCommandLineArguments,
				true // debug log
			));
}

static void test_parse_game_file_with_flag()
{
	ORWELL_LOG_DEBUG("test_parse_game_file_with_flag");
	TempFile aTechConfigFile(std::string(R"(
[server]
video-ports    = 9001:9004
)"));

	TempFile aGameConfigFile(std::string(R"(
[game]
robots = robot_A | robot_B
items = item_RedFlag

[robot_A]
name = Aristotle
team = Philosophers

[robot_B]
name = Bourbaki
team = Mathematicians

[item_RedFlag]
name = Red Flag
type = flag
rfid = myrfidredflag
color = -1 

)"));

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 1;
	aCommandLineArguments.m_pullerPort = 2;
	aCommandLineArguments.m_agentPort = 3;
	aCommandLineArguments.m_rcFilePath = aTechConfigFile.m_fileName;
	aCommandLineArguments.m_gameFilePath = aGameConfigFile.m_fileName;
	aCommandLineArguments.m_tickInterval = 666;
	aCommandLineArguments.m_gameDuration = 274;
	aCommandLineArguments.m_dryRun = true;
	aCommandLineArguments.m_broadcast = false;

	orwell::Application::Parameters aExpectedParameters ;
	aExpectedParameters.m_commandLineParameters = aCommandLineArguments;
	aExpectedParameters.m_robots = {
			{"robot_A", {"Aristotle", "Philosophers"}},
			{"robot_B", {"Bourbaki", "Mathematicians"}},
	};
	aExpectedParameters.m_items = {
			{"item_RedFlag", {"Red_Flag", "flag", "myrfidredflag", -1}},
	};
	aExpectedParameters.m_teams = {"Mathematicians", "Philosophers"};
	aExpectedParameters.m_videoPorts = {9004, 9003, 9002, 9001};

	test_ReadParameters(
			Status::PASS,
			Common::GetArguments(
				aCommandLineArguments,
				true // debug log
			),
			aExpectedParameters);

	// second test. This time item_RedFlag has both a rfid and a color... which is a no go
	TempFile aGameConfigFile2(std::string(R"(
[game]
robots = robot_A | robot_B
items = item_RedFlag

[robot_A]
name = Aristotle
team = Philosophers

[robot_B]
name = Bourbaki
team = Mathematicians

[item_RedFlag]
name = Red Flag
type = flag
rfid = myrfidredflag
color = 2

)"));
	aCommandLineArguments.m_gameFilePath = aGameConfigFile2.m_fileName;
	aExpectedParameters.m_items = {
			{"item_RedFlag", {"Red_Flag", "flag", "myrfidredflag", 2}},
	};
	aExpectedParameters.m_commandLineParameters = aCommandLineArguments;

	test_ReadParameters(
			Status::FAIL,
			Common::GetArguments(
				aCommandLineArguments,
				true // debug log
			),
			aExpectedParameters);
}

int main()
{
	orwell::support::GlobalLogger::Create("test_application", "test_application.log", true);
	log4cxx::NDC ndc("test_application");

	test_nothing();
	test_wrong_port_range_publisher_1();
	test_wrong_port_range_publisher_2();
	test_wrong_port_range_publisher_3();
	test_same_ports_agent_publisher();
	test_same_ports_puller_publisher();
	test_same_ports_puller_agent();
	test_most_arguments();
	test_parse_command_line();
	test_parse_command_line_and_file_1();
	test_parse_command_line_and_file_2();
	test_parse_command_line_and_file_3();
	test_parse_command_line_and_file_4();
	test_parse_command_line_and_file_5();
	test_parse_command_line_and_file_6_badConfigFile();
	test_parse_command_line_and_file_7_moreRobotsThanVideoPorts();
	test_parse_game_file_with_flag();

	orwell::support::GlobalLogger::Clear();
	return 0;
}

