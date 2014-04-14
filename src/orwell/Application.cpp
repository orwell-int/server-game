
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
	, m_pullerPort(0)
	, m_publisherPort(0)
	, m_agentPort(0)
	, m_ticInterval(500)
	, m_consoleDebugLogs(false)
	, m_broadcast(true)
	, m_dryRun(false)
	, m_state(State::CREATED)
{

}

Application::~Application()
{
	std::cout << "Application::~Application()" << std::endl;
	clean();
}

void Application::run(int argc, char * argv[])
{
	if (State::CREATED != m_state)
	{
		ORWELL_LOG_WARN("run can only be called when in state CREATED");
		return;
	}
	/***************************************
	*  Run the server only if all is set  *
	***************************************/
	if (initApplication(argc, argv))
	{
		if (not (initServer() and initConfigurationFile()))
		{
			return;
		}
		m_state = State::INITIALISED;
		if (m_dryRun)
		{
			ORWELL_LOG_INFO("Exit without starting (dry-run).");
			m_state = State::RUNNING;
			return;
		}

		if (m_broadcast)
		{
			// Broadcast receiver and main loop are run in separated threads
			pid_t aChildProcess = fork();

			m_state = State::RUNNING;
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
		else
		{
			m_server->loop();
		}
	}
}

bool Application::stop()
{
	bool aStopped = false;
	if (State::RUNNING == m_state)
	{
		m_server->stop();
		if (m_broadcast)
		{
			m_broadcastServer->stop();
		}
		aStopped = true;
		m_state = State::STOPPED;
	}
	else
	{
		ORWELL_LOG_WARN("stop can only be called when in state RUNNING");
	}
	return aStopped;
}

void Application::clean()
{
	std::cout << "Application::clean()" << std::endl;
	if (m_server != nullptr)
	{
		std::cout << "delete m_server" << std::endl;
		delete m_server;
		m_server = nullptr;
	}
	if (m_broadcastServer != nullptr)
	{
		delete m_broadcastServer;
		m_broadcastServer = nullptr;
	}
}

orwell::Server * Application::accessServer(bool const iUnsafe)
{
	if (not iUnsafe)
	{
		assert(nullptr != m_server);
	}
	return m_server;
}


bool Application::initServer()
{
	ORWELL_LOG_INFO("Initialize server : publisher tcp://*:" << m_publisherPort << " puller tcp://*:" << m_pullerPort);

	std::string aPublisherAddress = "tcp://*:" + boost::lexical_cast<std::string>(m_publisherPort);
	std::string aPullerAddress = "tcp://*:" + boost::lexical_cast<std::string>(m_pullerPort);

	m_server = new orwell::Server(aPullerAddress, aPublisherAddress, m_ticInterval);
	if (m_broadcast)
	{
		m_broadcastServer = new orwell::BroadcastServer(aPullerAddress, aPublisherAddress);
	}
	return true;
}

bool Application::initConfigurationFile()
{
	// If the user didn't specify any .ini file, let's just go
	if (m_rcFilePath.empty())
	{
		return true;
	}
	
	ptree aPtree;
	ini_parser::read_ini(m_rcFilePath, aPtree);
	
	// First of all get the server related things (if not provided already)
	if (m_publisherPort == 0)
	{
		boost::optional<uint16_t> aPublisherPort = aPtree.get_optional<uint16_t>("server.publisher-port");
		m_publisherPort = aPublisherPort? aPublisherPort.get() : 0;
		ORWELL_LOG_DEBUG("Using Publisher Port: " << m_publisherPort);
	}
	
	if (m_pullerPort == 0)
	{
		boost::optional<uint16_t> aPullerPort = aPtree.get_optional<uint16_t>("server.puller-port");
		m_pullerPort = aPullerPort? aPullerPort.get() : 0;
		ORWELL_LOG_DEBUG("Using Puller Port: " << m_pullerPort);
	}
	
	if (m_ticInterval == 0)
	{
		boost::optional<uint16_t> aTicInterval = aPtree.get_optional<uint16_t>("server.tic-interval");
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

bool Application::initApplication(int argc, char * argv[])
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
		("tic-interval,T",   value<uint32_t>(),    "Interval in tics between GameState messages")
		("version,v",                              "Print version number and exits")
		("debug-log,d",                            "Print debug logs on the console")
		("no-broadcast",                           "Do not start the broadcast.")
		("dry-run,n",                              "Do not start the server.");

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
		m_publisherPort = aVariablesMap["publisher-port"].as<uint16_t>();
	}
	
	if (aVariablesMap.count("puller-port"))
	{
		m_pullerPort = aVariablesMap["puller-port"].as<uint16_t>();
	}
	
	if (aVariablesMap.count("agent-port"))
	{
		m_agentPort = aVariablesMap["agent-port"].as<uint16_t>();
	}
	
	if (aVariablesMap.count("tic-interval"))
	{
		m_ticInterval = aVariablesMap["tic-interval"].as<uint32_t>();
	}
	
	if (aVariablesMap.count("debug-log"))
	{
		m_consoleDebugLogs = true;
	}

	m_dryRun = aVariablesMap.count("dry-run");
	m_broadcast = not aVariablesMap.count("no-broadcast");
	ORWELL_LOG_DEBUG("m_pullerPort = " << m_pullerPort);
	ORWELL_LOG_DEBUG("m_publisherPort = " << m_publisherPort);
	ORWELL_LOG_DEBUG("m_agentPort = " << m_agentPort);
	if (m_publisherPort == m_pullerPort)
	{
		std::cerr << "Publisher and puller ports have the same value ("
			<< m_pullerPort <<  ") which is not allowed." << std::endl;
		return false;
	}
	if (m_publisherPort == m_agentPort)
	{
		std::cerr << "Publisher and agent ports have the same value ("
			<< m_agentPort <<  ") which is not allowed." << std::endl;
		return false;
	}
	if (m_pullerPort == m_agentPort)
	{
		std::cerr << "Puller and agent ports have the same value ("
			<< m_agentPort <<  ") which is not allowed." << std::endl;
		return false;
	}

	if (m_publisherPort == 0 or m_pullerPort == 0)
	{
		std::cerr << "Missing ports informations" << std::endl;
		std::cout << aDescription << std::endl;
		return false;
	}

	return true;
}

