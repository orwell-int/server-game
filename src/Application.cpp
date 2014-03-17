
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

void Application::run(int argc, char **argv)
{
	/***************************************
	*  Run the server only if all is set  *
	***************************************/
	if (initApplication(argc, argv) and initServer() and initConfigurationFile())
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
		while(waitpid(aChildProcess, &aStatus, WNOHANG) == 0) 
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

bool Application::initServer()
{
	ORWELL_LOG_INFO("Initialize server : publisher tcp://*:" << m_publisherPort << " puller tcp://*:" << m_pullerPort);

	std::string aPublisherAddress = "tcp://*:" + boost::lexical_cast<std::string>(m_publisherPort);
	std::string aPullerAddress = "tcp://*:" + boost::lexical_cast<std::string>(m_pullerPort);

	m_server = new orwell::Server(aPullerAddress, aPublisherAddress, 500);
	m_broadcastServer = new orwell::BroadcastServer(aPullerAddress, aPublisherAddress);
	return true;
}

bool Application::initConfigurationFile()
{
	// If the user didn't specify any .ini file, let's just go
	if (m_rcFilePath.empty()) {
		return true;
	}
	
	ptree aPtree;
	ini_parser::read_ini(m_rcFilePath, aPtree);
	
	// First of all get the server related things (if not provided already)
	if (m_publisherPort == 0)
	{
		boost::optional<uint32_t> aPublisherPort = aPtree.get_optional<uint32_t>("server.publisher-port");
		m_publisherPort = aPublisherPort? aPublisherPort.get() : 0;
		ORWELL_LOG_DEBUG("Using Publisher Port: " << m_publisherPort);
	}
	
	if (m_pullerPort == 0)
	{
		boost::optional<uint32_t> aPullerPort = aPtree.get_optional<uint32_t>("server.puller-port");
		m_pullerPort = aPullerPort? aPullerPort.get() : 0;
		ORWELL_LOG_DEBUG("Using Puller Port: " << m_pullerPort);
	}
	
	if (m_ticInterval == 0)
	{
		boost::optional<uint32_t> aTicInterval = aPtree.get_optional<uint32_t>("server.tic-interval");
		m_ticInterval = aTicInterval? aTicInterval.get() : 0;
		ORWELL_LOG_DEBUG("Using Tic Interval: " << m_ticInterval);
	}
	
	// Now get the optional things regarding the game itself
	boost::optional<std::string> aGameRobots = aPtree.get_optional<std::string>("game.robots");
	boost::optional<std::string> aGameType = aPtree.get_optional<std::string>("game.gametype");
	boost::optional<std::string> aGameName = aPtree.get_optional<std::string>("game.gamename");
	
	// If we have some robots, we need to retrieve them from the ini file itself and add them in the Server's context
	if (aGameRobots)
	{
		tokenizeRobots(aGameRobots.get());
		
		for (std::string const & iRobot : m_robotsList)
		{
			std::string aRobotName = aPtree.get<std::string>(iRobot + ".name");
			ORWELL_LOG_INFO("Pushing robot: " << aRobotName);
			m_server->accessContext().addRobot(aRobotName);
		}
	}
	
	return true;
}

std::vector<std::string> & Application::tokenizeRobots(std::string const & iRobotsString)
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
				m_robotsList.push_back(aToken);
				aToken.clear();
			}
		}
	});
	
	if (not aToken.empty())
	{
		m_robotsList.push_back(aToken);
	}
	
	return m_robotsList;
}

bool Application::initApplication(int argc, char **argv)
{
	// Parse the command line arguments
	options_description aDescription("Usage: " + std::string(argv[0]) + " [PpAvTdrh]");

	// ??? : Do we want to have default values or not? Feel free to add them when integrating.
	aDescription.add_options()
	    ("help,h",                                 "Produce help message and exits")
	    ("publisher-port,P", value<uint32_t>(),    "Publisher port")
	    ("puller-port,p",    value<uint32_t>(),    "Puller port")
	    ("agent-port,A",     value<uint32_t>(),    "Agent Port")
		("orwellrc,r",       value<std::string>(), "Load configuration from rc file")
		("tic-interval,T",   value<uint32_t>(),    "Interval in tics between GameState messages")
	    ("version,v",                              "Print version number and exits")
		("debug-log,d",                            "Print debug logs on the console");

	variables_map aVariablesMap;
	store(parse_command_line(argc, argv, aDescription), aVariablesMap);
	notify(aVariablesMap);
	
	if (aVariablesMap.count("orwellrc"))
	{
		m_rcFilePath = aVariablesMap["orwellrc"].as<std::string>();
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
		m_publisherPort = aVariablesMap["publisher-port"].as<uint32_t>();
	}
	
	if (aVariablesMap.count("puller-port"))
	{
		m_pullerPort = aVariablesMap["puller-port"].as<uint32_t>();
	}
	
	if (aVariablesMap.count("agent-port"))
	{
		m_agentPort = aVariablesMap["agent-port"].as<uint32_t>();
	}
	
	if (aVariablesMap.count("tic-interval"))
	{
		m_ticInterval = aVariablesMap["tic-interval"].as<uint32_t>();
	}
	
	if (aVariablesMap.count("debug-log"))
	{
		m_consoleDebugLogs = true;
	}


	if (m_publisherPort == 0 or m_pullerPort == 0)
	{
		std::cerr << "Missing ports informations" << std::endl;
		std::cout << aDescription << std::endl;
		return false;
	}
	
	return true;
}

