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
	test_ReadParameters(Status::PASS, Common::GetArguments());
}

static void test_wrong_port_range_publisher_1()
{
	ORWELL_LOG_DEBUG("test_wrong_port_range_publisher_1");
	test_ReadParameters(Status::FAIL, Common::GetArguments(false, 0, 42, 43));
}

static void test_wrong_port_range_publisher_2()
{
	ORWELL_LOG_DEBUG("test_wrong_port_range_publisher_2");
	test_ReadParameters(Status::PASS, Common::GetArguments(false, -1024, 42, 43));
}

static void test_wrong_port_range_publisher_3()
{
	ORWELL_LOG_DEBUG("test_wrong_port_range_publisher_3");
	test_ReadParameters(Status::FAIL, Common::GetArguments(false, 99999, 42, 43));
}

static void test_same_ports_agent_publisher()
{
	ORWELL_LOG_DEBUG("test_same_ports_agent_publisher");
	test_ReadParameters(Status::FAIL, Common::GetArguments(false, 41, 42, 41));
}

static void test_same_ports_puller_publisher()
{
	ORWELL_LOG_DEBUG("test_same_ports_puller_publisher");
	test_ReadParameters(Status::FAIL, Common::GetArguments(false, 41, 41, 43));
}

static void test_same_ports_puller_agent()
{
	ORWELL_LOG_DEBUG("test_same_ports_puller_agent");
	test_ReadParameters(Status::FAIL, Common::GetArguments(false, 41, 42, 42));
}

static void test_most_arguments()
{
	ORWELL_LOG_DEBUG("test_most_arguments");
	test_ReadParameters(Status::PASS, Common::GetArguments(
			false, // help
			41, // publisher port
			42, // puller port
			43, // agent port
			boost::none, // std::string("orwell.rc"), // orwellrc
			boost::none, // game config file path
			666, // tick interval
			false, // version
			true, // debug log
			true, // no broadcast
			false)); // dry run
}

static void test_parse_command_line()
{
	ORWELL_LOG_DEBUG("test_parse_command_line");
	orwell::Application::Parameters aExpectedParameters = {
		2, // puller port
		1, // publisher port
		3, // agent port
		666, // tick interval
		boost::none, // rc file path
		boost::none, // game config file path
		true, // dry run
		false, // broadcast
		//{}, // robot ids
		{}, // robot descriptors
		{}, // teams
	};
	test_ReadParameters(Status::PASS, Common::GetArguments(
				false, // help
				1, // publisher port
				2, // puller port
				3, // agent port
				boost::none, // std::string("orwell.rc"), // orwellrc
				boost::none, // game config file path
				666, // tick interval
				false, // version
				true, // debug log
				true, // no broadcast
				true), // dry run
			aExpectedParameters);
}

#include <stdio.h>

struct TempFile
{
	std::string m_fileName;

	TempFile(std::string const & iContent);

	~TempFile();
};

TempFile::TempFile(std::string const & iContent)
{
	char aFileName[L_tmpnam];
	tmpnam(aFileName);
	FILE * aFile = fopen(aFileName, "w");
	size_t const aWritten = fputs(iContent.c_str(), aFile);
	if (aWritten != iContent.size())
	{
		std::cerr << "Temporary file not create properly." << std::endl;
	}
	fclose(aFile);
	m_fileName = std::string(aFileName);
}

TempFile::~TempFile()
{
	if (not m_fileName.empty())
	{
		remove(m_fileName.c_str());
		m_fileName.erase();
	}
}

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
)"));
	orwell::Application::Parameters aExpectedParameters = {
		2, // puller port
		1, // publisher port
		3, // agent port
		666, // tick interval
		aTempFile.m_fileName, // rc file path
		boost::none, // game config file path
		true, // dry run
		false, // broadcast
		//{}, // robot ids
		{}, // robot descriptors
		{}, // teams
	};
	test_ReadParameters(Status::PASS, Common::GetArguments(
				false, // help
				1, // publisher port
				2, // puller port
				3, // agent port
				aTempFile.m_fileName, // orwellrc
				boost::none, // game config file path
				666, // tick interval
				false, // version
				true, // debug log
				true, // no broadcast
				true), // dry run
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
)"));
	orwell::Application::Parameters aExpectedParameters = {
		901, // puller port
		900, // publisher port
		903, // agent port
		50, // tick interval
		aTempFile.m_fileName, // rc file path
		boost::none, // game config file path
		false, // dry run
		true, // broadcast
		//{}, // robot ids
		{}, // robot descriptors
		{}, // teams
	};
	test_ReadParameters(Status::PASS, Common::GetArguments(
				false, // help
				boost::none, // publisher port
				boost::none, // puller port
				boost::none, // agent port
				aTempFile.m_fileName, // orwellrc
				boost::none, // game config file path
				boost::none), // tick interval)
		aExpectedParameters);
}

// make sure a file containing unrelated configuration does nothing wrong.
static void test_parse_command_line_and_file_3()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_3");
	TempFile aTempFile(std::string(R"(
[server]
[its a trap]
puller-port = 42
)"));
	orwell::Application::Parameters aExpectedParameters = {
		2, // puller port
		1, // publisher port
		3, // agent port
		666, // tick interval
		aTempFile.m_fileName, // rc file path
		boost::none, // game config file path
		true, // dry run
		false, // broadcast
		//{}, // robot ids
		{}, // robot descriptors
		{}, // teams
	};
	test_ReadParameters(Status::PASS, Common::GetArguments(
				false, // help
				1, // publisher port
				2, // puller port
				3, // agent port
				aTempFile.m_fileName, // orwellrc
				boost::none, // game config file path
				666, // tick interval
				false, // version
				true, // debug log
				true, // no broadcast
				true), // dry run
			aExpectedParameters);
}

// check the content of the game and the different robots.
static void test_parse_command_line_and_file_4()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_4");
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
	orwell::Application::Parameters aExpectedParameters = {
		2, // puller port
		1, // publisher port
		3, // agent port
		666, // tick interval
		boost::none, // rc file path
		aTempFile.m_fileName, // game config file path
		true, // dry run
		false, // broadcast
		//{"robot_A", "robot_B"}, // robot ids
		{
			{"robot_A", {"Aristotle", "Philosophers"}},
			{"robot_B", {"Bourbaki", "Mathematicians"}},
		}, // robot descriptors
		{"Mathematicians", "Philosophers"}, // teams
		std::string("Problem"), // game type
		std::string("Enigma"), // game name
	};
	test_ReadParameters(Status::PASS, Common::GetArguments(
				false, // help
				1, // publisher port
				2, // puller port
				3, // agent port
				boost::none, // orwellrc
				aTempFile.m_fileName, // game config file path
				666, // tick interval
				false, // version
				true, // debug log
				true, // no broadcast
				true), // dry run
			aExpectedParameters);
}

// make sure we detect errors
static void test_parse_command_line_and_file_5()
{
	ORWELL_LOG_DEBUG("test_parse_command_line_and_file_5");
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
	orwell::Application::Parameters aExpectedParameters = {
		2, // puller port
		1, // publisher port
		3, // agent port
		666, // tick interval
		boost::none, // rc file path
		aTempFile.m_fileName, // game config file path
		true, // dry run
		false, // broadcast
		//{"robot_A", "robot_B"}, // robot ids
		{
			{"robot_A", {"Aristotle", "Philosophers"}},
			{"robot_B", {"Bourbaki", "Mistake!!!"}}, // voluntary mistake
		}, // robot descriptors
		{"Mathematicians", "Philosophers"}, // teams
		std::string("Problem"), // game type
		std::string("Enigma"), // game name
	};
	bool aThrown = false;
	try
	{
		test_ReadParameters(Status::PASS, Common::GetArguments(
					false, // help
					1, // publisher port
					2, // puller port
					3, // agent port
					boost::none, // orwellrc
					aTempFile.m_fileName, // game config file path
					666, // tick interval
					false, // version
					true, // debug log
					true, // no broadcast
					true), // dry run
				aExpectedParameters);
	}
	catch (bool const aBool)
	{
		ORWELL_LOG_DEBUG("Expected exception cought");
		aThrown = true;
	}
	assert(aThrown);
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

	orwell::support::GlobalLogger::Clear();
	return 0;
}

