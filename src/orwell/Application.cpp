
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
	, m_state(State::CREATED)
{
}

bool Application::ReadParameters(
		int argc,
		char * argv[],
		Parameters & oParam)
{
	if ( not ParseParametersFromCommandLine(argc, argv, oParam) )
	{
		return false;
	}
	if (oParam.m_rcFilePath and (not (*oParam.m_rcFilePath).empty()))
	{
		if ( not ParseParametersFromConfigFile(oParam) )
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
	if (not oParam.m_broadcast)
	{
		oParam.m_broadcast = true;
	}
	if (not oParam.m_dryRun)
	{
		oParam.m_dryRun = false;
	}

	return CheckParametersConsistency(oParam);
}

bool Application::ParseParametersFromCommandLine(
		int argc, char * argv[],
		Application::Parameters & oParam)
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
				("no-broadcast",                           "Do not start the broadcast.")
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
		oParam.m_rcFilePath = aVariablesMap["orwellrc"].as<std::string>();
		ORWELL_LOG_DEBUG("orwellrc from command line = " << oParam.m_rcFilePath);
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
		oParam.m_dryRun = true;
		ORWELL_LOG_DEBUG("this is a dry run");
	}
	else
	{
		ORWELL_LOG_DEBUG("this is NOT a dry run");
	}

	if (aVariablesMap.count("no-broadcast"))
	{
		oParam.m_broadcast = false;
		ORWELL_LOG_DEBUG("do not start broadcast server");
	}
	return true;
}

bool Application::ParseParametersFromConfigFile(
		Parameters & ioParam)
{
	ptree aPtree;
	ini_parser::read_ini(*ioParam.m_rcFilePath, aPtree);

	if (not ioParam.m_publisherPort)
	{
		boost::optional<uint16_t> aPublisherPort = aPtree.get_optional<uint16_t>("server.publisher-port");
		if (aPublisherPort)
		{
			ioParam.m_publisherPort = aPublisherPort;
			ORWELL_LOG_DEBUG("publisher-port from config file = " << ioParam.m_publisherPort );
		}
	}
	if (not ioParam.m_pullerPort)
	{
		boost::optional<uint16_t> aPullerPort = aPtree.get_optional<uint16_t>("server.puller-port");
		if (aPullerPort)
		{
			ioParam.m_pullerPort = aPullerPort;
			ORWELL_LOG_DEBUG("puller-port from config file = " << ioParam.m_pullerPort );
		}
	}
	if (not ioParam.m_agentPort)
	{
		boost::optional<uint16_t> aAgentPort = aPtree.get_optional<uint16_t>("server.agent-port");
		if (aAgentPort)
		{
			ioParam.m_agentPort = aAgentPort;
			ORWELL_LOG_DEBUG("agent-port from config file = " << ioParam.m_agentPort );
		}
	}
	if (not ioParam.m_tickInterval)
	{
		boost::optional<uint16_t> aTickInterval = aPtree.get_optional<uint16_t>("server.tic-interval");
		if (aTickInterval)
		{
			ioParam.m_tickInterval = aTickInterval;
			ORWELL_LOG_DEBUG("tick interval from config file = " << ioParam.m_tickInterval );
		}
	}

	// Now get the optional things regarding the game itself
	boost::optional<std::string> aGameRobots = aPtree.get_optional<std::string>("game.robots");
	ioParam.m_gameType = aPtree.get_optional<std::string>("game.gametype");
	ioParam.m_gameName = aPtree.get_optional<std::string>("game.gamename");

	// If we have some robots, we need to retrieve them from the ini file itself and add them in the Server's context
	if (aGameRobots)
	{
		std::vector<std::string> aRobotList;
		Application::TokenizeRobots(aGameRobots.get(), aRobotList);

		for (std::string const & iRobot : aRobotList)
		{
			std::string aRobotName = aPtree.get<std::string>(iRobot + ".name");
			std::string aRobotTeam = aPtree.get<std::string>(iRobot + ".team");
			ORWELL_LOG_INFO("Pushing robot: " << aRobotName << " ; in team " << aRobotTeam);
			ioParam.m_robots[iRobot] = Parameters::Robot{aRobotName, aRobotTeam};
			ioParam.m_teams.insert(aRobotTeam);
		}
	}
	
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
	if ((*iParam.m_publisherPort) == 0 or (*iParam.m_pullerPort == 0))
	{
		ORWELL_LOG_ERROR("Invalid port information. Ports are \n Puller=" << iParam.m_pullerPort << "\n Publisher=" << iParam.m_publisherPort);
		return false;
	}
	return true;
}

Application::~Application()
{
	// this is for debugging purpose
	//std::cout << "Application::~Application()" << std::endl;
	clean();
}

void Application::run(Parameters const & iParam)
{
	if (State::CREATED != m_state)
	{
		ORWELL_LOG_WARN("run can only be called when in state CREATED");
		return;
	}
	/***************************************
	 *  Run the server only if all is set  *
	 ***************************************/
	if (initServer(iParam))
	{
		// temporary hack
		for (auto aPair : iParam.m_robots)
		{
			this->m_server->accessContext().addRobot(aPair.second.m_name);
		}
		//m_state = State::INITIALISED;
		m_state = State::RUNNING;
		if ((iParam.m_dryRun) and (*iParam.m_dryRun))
		{
			ORWELL_LOG_INFO("Exit without starting (dry-run).");
			return;
		}
		if ((iParam.m_broadcast) and (*iParam.m_broadcast))
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
		if (nullptr != m_broadcastServer)
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
	if (nullptr != m_server)
	{
		delete m_server;
		m_server = nullptr;
	}
	if (nullptr != m_broadcastServer)
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


bool Application::initServer(Parameters const & iParam)
{
	ORWELL_LOG_INFO("Initialize server : publisher tcp://*:" << iParam.m_publisherPort << " puller tcp://*:" << iParam.m_pullerPort);

	std::string aPublisherAddress = "tcp://*:" + boost::lexical_cast<std::string>( iParam.m_publisherPort);
	std::string aPullerAddress = "tcp://*:" + boost::lexical_cast<std::string>( iParam.m_pullerPort) ;

	m_server = new orwell::Server(aPullerAddress, aPublisherAddress, iParam.m_tickInterval.get());
	if ((iParam.m_broadcast) and (*iParam.m_broadcast))
	{
		m_broadcastServer = new orwell::BroadcastServer(aPullerAddress, aPublisherAddress);
	}
	return true;
}

void Application::TokenizeRobots(
		std::string const & iRobotsString,
		std::vector<std::string> & oRobotList)
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
				oRobotList.push_back(aToken);
				aToken.clear();
			}
		}
	});

	if (not aToken.empty())
	{
		oRobotList.push_back(aToken);
	}
}

bool operator!=(
		orwell::Application::Parameters const & iLeft,
		orwell::Application::Parameters const & iRight)
{
	return (not (iLeft == iRight));
}

bool operator==(
		orwell::Application::Parameters const & iLeft,
		orwell::Application::Parameters const & iRight)
{
	// comparing maps directly does not work
	bool aSameRobots = (iLeft.m_robots.size() == iRight.m_robots.size());
	auto aRobotIterator = iLeft.m_robots.begin();
	while ((aSameRobots) and (iLeft.m_robots.end() != aRobotIterator))
	{
		std::string const & aKey = aRobotIterator->first;
		auto const aFound = iRight.m_robots.find(aKey);
		if ((aSameRobots = (iRight.m_robots.end() != aFound)))
		{
			aSameRobots = (aRobotIterator->second == aFound->second);
		}
		++aRobotIterator;
	}
	return ((iLeft.m_pullerPort == iRight.m_pullerPort)
		and (iLeft.m_publisherPort == iRight.m_publisherPort)
		and (iLeft.m_agentPort == iRight.m_agentPort)
		and (iLeft.m_tickInterval == iRight.m_tickInterval)
		and (iLeft.m_rcFilePath == iRight.m_rcFilePath)
		and (iLeft.m_dryRun == iRight.m_dryRun)
		and (iLeft.m_broadcast == iRight.m_broadcast)
		and (aSameRobots)
		and (iLeft.m_teams == iRight.m_teams)
		and (iLeft.m_gameType == iRight.m_gameType)
		and (iLeft.m_gameName == iRight.m_gameName));
}

std::ostream & operator<<(
		std::ostream & ioOstream,
		orwell::Application::Parameters const & iParameters)
{
	ioOstream << "puller port [" << iParameters.m_pullerPort << "] ; ";
	ioOstream << "publisher port [" << iParameters.m_publisherPort << "] ; ";
	ioOstream << "agent port [" << iParameters.m_agentPort << "] ; ";
	ioOstream << "tick interval [" << iParameters.m_tickInterval << "] ; ";
	ioOstream << "rc file path [" << iParameters.m_rcFilePath << "] ; ";
	ioOstream << "dry run [" << iParameters.m_dryRun << "] ; ";
	ioOstream << "broadcast [" << iParameters.m_broadcast << "] ";
	ioOstream << "robots [";
	for (auto const aPair : iParameters.m_robots)
	{
		std::string const & aKey = aPair.first;
		orwell::Application::Parameters::Robot const & aRobot = aPair.second;
		ioOstream << aKey << ":" << aRobot.m_name << "@" << aRobot.m_team << ", ";
	}
	ioOstream << "] ; ";
	ioOstream << "teams [";
	std::ostream_iterator< std::string > aOut(ioOstream,", ");
	std::copy(iParameters.m_teams.begin(), iParameters.m_teams.end(), aOut);
	ioOstream << "] ; ";
	ioOstream << "game type [" << iParameters.m_gameType << "] ; ";
	ioOstream << "game name [" << iParameters.m_gameName << "]";
	return ioOstream;
}

bool operator==(
		orwell::Application::Parameters::Robot const & iLeft,
		orwell::Application::Parameters::Robot const & iRight)
{
	return ((iLeft.m_name == iRight.m_name)
		and (iLeft.m_team == iRight.m_team));
}

