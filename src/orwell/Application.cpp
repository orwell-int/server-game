
#include "orwell/Application.hpp"
#include "orwell/Server.hpp"
#include "orwell/BroadcastServer.hpp"
#include "orwell/support/GlobalLogger.hpp"

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

using namespace orwell;
using namespace boost::program_options;
using namespace boost::property_tree;
using namespace log4cxx;
using std::make_shared;
using std::string;

Application & Application::GetInstance()
{
	static Application m_application;
	return m_application;
}

Application::Application()
: m_server(nullptr)
, m_broadcastServer(nullptr)
{
}

bool Application::ReadParameters(
		int argc,
		char * argv[],
		Parameters & oParam)
{
	std::string aConfigFilePath;

	if ( not ParseParametersFromCommandLine(argc, argv, oParam, aConfigFilePath) )
	{
		return false;
	}
	if ( not aConfigFilePath.empty())
	{
		if ( not ParseParametersFromConfigFile(oParam, aConfigFilePath) )
		{
			return false;
		}
	}

	// Default values
	if (not oParam.m_publisherPort)
	{
		oParam.m_publisherPort = 9000;
		ORWELL_LOG_DEBUG("by default, publisher-port = " << oParam.m_publisherPort);
	}
	if (not oParam.m_pullerPort)
	{
		oParam.m_pullerPort = 9001;
		ORWELL_LOG_DEBUG("by default, puller-port = " << oParam.m_pullerPort);
	}
	if (not oParam.m_tickInterval)
	{
		oParam.m_tickInterval = 500;
		ORWELL_LOG_DEBUG("by default, tick interval = " << oParam.m_tickInterval);
	}

	return CheckParametersConsistency(oParam);
}

bool Application::ParseParametersFromCommandLine(
		int argc, char * argv[],
		Application::Parameters & oParam,
		string & oConfigFile)
{
	// Parse the command line arguments
	options_description aDescription("Usage: " + std::string(argv[0]) + " [PpAvTdrhn]");

	// ??? : Do we want to have default values or not? Feel free to add them when integrating.
	aDescription.add_options()
				("help,h",                                 "Produce help message and exits")
				("publisher-port,P", value<uint16_t>(),    "Publisher port")
				("puller-port,p",    value<uint16_t>(),    "Puller port")
				("agent-port,A",     value<uint16_t>(),    "Agent Port")
				("orwellrc,r",       value<std::string>(), "Load configuration from rc file")
				("tick-interval,T",  value<uint32_t>(),    "Interval in ticks between GameState messages")
				("version,v",                              "Print version number and exits")
				("debug-log,d",                            "Print debug logs on the console")
				("dry-run,n",                              "Do not start the server.");

	variables_map aVariablesMap;
	try
	{
	store(parse_command_line(argc, argv, aDescription), aVariablesMap);
	}
	catch (boost::program_options::error const & aParseException)
	{
		std::cerr << "ERROR: " << aParseException.what() << std::endl << std::endl;
		return false;
	}
	notify(aVariablesMap);

	// do we log the debug information on the console ?
	orwell::support::GlobalLogger::Create("server_game", "orwell.log", aVariablesMap.count("debug-log"));

	if (aVariablesMap.count("orwellrc"))
	{
		oConfigFile = aVariablesMap["orwellrc"].as<std::string>();
	}

	if (aVariablesMap.count("help"))
	{
		std::cout << aDescription << std::endl;
		return false;
	}

	if (aVariablesMap.count("version"))
	{
		std::cout << argv[0] << " " << VERSION << " - codename '" << CODENAME << "'" << std::endl;
		std::cout << "Compiled " << __DATE__ << " at " << __TIME__ << std::endl;
		std::cout << "Compiler used: " << __VERSION__ << std::endl;
		return false;
	}

	if (aVariablesMap.count("publisher-port"))
	{
		oParam.m_publisherPort = aVariablesMap["publisher-port"].as<uint16_t>();
		ORWELL_LOG_DEBUG("publisher-port from command line = " << oParam.m_publisherPort);
	}

	if (aVariablesMap.count("puller-port"))
	{
		oParam.m_pullerPort = aVariablesMap["puller-port"].as<uint16_t>();
		ORWELL_LOG_DEBUG("puller-port from command line = " << oParam.m_pullerPort);
	}

	if (aVariablesMap.count("agent-port"))
	{
		oParam.m_agentPort = aVariablesMap["agent-port"].as<uint16_t>();
		ORWELL_LOG_DEBUG("agent-port from command line = " << oParam.m_agentPort);
	}

	if (aVariablesMap.count("tick-interval"))
	{
		oParam.m_tickInterval = aVariablesMap["tick-interval"].as<uint32_t>();
		ORWELL_LOG_DEBUG("tick interval from command line = " << oParam.m_tickInterval);
	}

	if (aVariablesMap.count("dry-run"))
	{
		oParam.m_dryRun = aVariablesMap["dry-run"].as<bool>();
		ORWELL_LOG_DEBUG("this is a dry run");
	}
	return true;
}

bool Application::ParseParametersFromConfigFile(
		Parameters & oParam,
		std::string const & iConfigFilePath)
{
	ptree aPtree;
	ini_parser::read_ini(iConfigFilePath, aPtree);

	if (not oParam.m_publisherPort)
	{
		boost::optional<uint16_t> aPublisherPort = aPtree.get_optional<uint16_t>("server.publisher-port");
		if (aPublisherPort)
		{
			oParam.m_publisherPort = aPublisherPort;
			ORWELL_LOG_DEBUG("publisher-port from config file = " << oParam.m_publisherPort );
		}
	}
	if (not oParam.m_pullerPort)
	{
		boost::optional<uint16_t> aPullerPort = aPtree.get_optional<uint16_t>("server.puller-port");
		if (aPullerPort)
		{
			oParam.m_pullerPort = aPullerPort;
			ORWELL_LOG_DEBUG("puller-port from config file = " << oParam.m_pullerPort );
		}
	}
	if (not oParam.m_tickInterval)
	{
		boost::optional<uint16_t> aTickInterval = aPtree.get_optional<uint16_t>("server.tic-interval");
		if (aTickInterval)
		{
			oParam.m_tickInterval = aTickInterval;
			ORWELL_LOG_DEBUG("tick interval from config file = " << oParam.m_tickInterval );
		}
	}
/*
	// Now get the optional things regarding the game itself
	boost::optional<std::string> aGameRobots = aPtree.get_optional<std::string>("game.robots");
	boost::optional<std::string> aGameType = aPtree.get_optional<std::string>("game.gametype");
	boost::optional<std::string> aGameName = aPtree.get_optional<std::string>("game.gamename");

	// If we have some robots, we need to retrieve them from the ini file itself and add them in the Server's context
	if (aGameRobots)
	{
		tokenizeRobots(aGameRobots.get(), oParam); // this fills oParam.m_robotsList

		for (std::string const & iRobot : oParam.m_robotsList)
		{
			std::string aRobotName = aPtree.get<std::string>(iRobot + ".name");
			ORWELL_LOG_INFO("Pushing robot: " << aRobotName);
			m_server->accessContext().addRobot(aRobotName);
		}
	}
	*/
	return true;
}

bool Application::CheckParametersConsistency(Parameters const & iParam)
{
	if (iParam.m_publisherPort == iParam.m_pullerPort)
	{
		ORWELL_LOG_ERROR("Publisher and puller ports have the same value (" << iParam.m_pullerPort << ") which is not allowed.");
		return false;
	}
	if (iParam.m_publisherPort == iParam.m_agentPort)
	{
		ORWELL_LOG_ERROR("Publisher and agent ports have the same value (" << iParam.m_agentPort << ") which is not allowed.");
		return false;
	}
	if (iParam.m_pullerPort == iParam.m_agentPort)
	{
		ORWELL_LOG_ERROR("Puller and agent ports have the same value (" << iParam.m_agentPort << ") which is not allowed.");
		return false;
	}
	if (*iParam.m_publisherPort == 0 or *iParam.m_pullerPort == 0)
	{
		ORWELL_LOG_ERROR("Invalid port information. Ports are \n Puller=" << iParam.m_pullerPort << "\n Publisher=" << iParam.m_publisherPort);
		return false;
	}
	return true;
}

void Application::run(Parameters const & iParam)
{
	/***************************************
	 *  Run the server only if all is set  *
	 ***************************************/
	if (initServer(iParam))
	{
		// Broadcast receiver and main loop are run in separated threads
		pid_t aChildProcess = fork();

		switch (aChildProcess)
		{
		case 0:
			ORWELL_LOG_INFO("Child started");
			m_broadcastServer->runBroadcastReceiver();
			return;
		default:
			ORWELL_LOG_INFO("Father started, child's pid: " << aChildProcess);
			m_server->loop();
			break;
		}

		ORWELL_LOG_INFO("Father continued");

		// Here the father will be waiting for the child to be over
		int aStatus;
		while (waitpid(aChildProcess, &aStatus, WNOHANG) == 0)
		{
			ORWELL_LOG_INFO("Waiting for process: " << aChildProcess << " to be over");
			sleep(1);
		}
	}
}

bool Application::stop()
{
	m_server->stop();
	m_broadcastServer->stop();
	return true;
}

void Application::clean()
{
	if (m_server != nullptr)
	{
		delete m_server;
		m_server = nullptr;
	}
	if (m_broadcastServer != nullptr)
	{
		delete m_broadcastServer;
		m_broadcastServer = nullptr;
	}
}

bool Application::initServer(Parameters const & iParam)
{
	ORWELL_LOG_INFO("Initialize server : publisher tcp://*:" << iParam.m_publisherPort << " puller tcp://*:" << iParam.m_pullerPort);

	std::string aPublisherAddress = "tcp://*:" + boost::lexical_cast<std::string>( iParam.m_publisherPort);
	std::string aPullerAddress = "tcp://*:" + boost::lexical_cast<std::string>( iParam.m_pullerPort) ;

	m_server = new orwell::Server(aPullerAddress, aPublisherAddress, iParam.m_tickInterval.get());
	m_broadcastServer = new orwell::BroadcastServer(aPullerAddress, aPublisherAddress);
	return true;
}

std::vector<std::string> & Application::tokenizeRobots(std::string const & iRobotsString, Parameters & ioParameters)
{
	std::string aToken;

	/* 
	 * This is kind of a hack: I know there are some boost libraries doing the
	 * tokenization but I really wanted to use a lambda function here..
	 */
	std::for_each(iRobotsString.begin(), iRobotsString.end(), [&](char iChar) {
		if (iChar != '|' and iChar != ' ')
		{
			aToken += iChar;
		}
		else
		{
			if (not aToken.empty())
			{
				ioParameters.m_robotsList.push_back(aToken);
				aToken.clear();
			}
		}
	});

	if (not aToken.empty())
	{
		ioParameters.m_robotsList.push_back(aToken);
	}

	return ioParameters.m_robotsList;
}


