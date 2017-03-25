
#include "orwell/Application.hpp"
#include "orwell/Server.hpp"
#include "orwell/BroadcastServer.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "orwell/support/SystemProxy.hpp"
#include "orwell/game/Item.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <log4cxx/ndc.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <sstream>

using namespace boost::program_options;
using namespace boost::property_tree;
using namespace log4cxx;
using std::make_shared;
using std::string;

namespace orwell
{

Application & Application::GetInstance()
{
	static support::SystemProxy m_systemProxy;
	static Application m_application(m_systemProxy);
	return m_application;
}

Application::Application(support::ISystemProxy const & iSystemProxy)
	: m_systemProxy(iSystemProxy)
	, m_server(nullptr)
	, m_broadcastServer(nullptr)
	, m_state(State::CREATED)
	, m_agentProxy(*this)
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

	if (not oParam.m_commandLineParameters.m_rcFilePath)
	{
		oParam.m_commandLineParameters.m_rcFilePath = "orwell-config.ini";
		ORWELL_LOG_DEBUG("by default, config file = " << oParam.m_commandLineParameters.m_rcFilePath);
	}

	if (oParam.m_commandLineParameters.m_rcFilePath )
	{
		if ( not ParseParametersFromConfigFile(oParam) )
		{
			return false;
		}
	}
	if (oParam.m_commandLineParameters.m_gameFilePath and (not (*oParam.m_commandLineParameters.m_gameFilePath).empty()))
	{
		ParseGameConfigFromFile(oParam);
	}

	// Default values
	if (not oParam.m_commandLineParameters.m_publisherPort)
	{
		oParam.m_commandLineParameters.m_publisherPort = 9000;
		ORWELL_LOG_DEBUG("by default, publisher-port = " << oParam.m_commandLineParameters.m_publisherPort);
	}
	if (not oParam.m_commandLineParameters.m_pullerPort)
	{
		oParam.m_commandLineParameters.m_pullerPort = 9001;
		ORWELL_LOG_DEBUG("by default, puller-port = " << oParam.m_commandLineParameters.m_pullerPort);
	}
	if (not oParam.m_commandLineParameters.m_agentPort)
	{
		oParam.m_commandLineParameters.m_agentPort = 9003;
		ORWELL_LOG_DEBUG("by default, agent-port = " << oParam.m_commandLineParameters.m_agentPort);
	}
	if (not oParam.m_commandLineParameters.m_tickInterval)
	{
		oParam.m_commandLineParameters.m_tickInterval = 500;
		ORWELL_LOG_DEBUG("by default, tick interval = " << oParam.m_commandLineParameters.m_tickInterval);
	}
	if (not oParam.m_commandLineParameters.m_gameDuration)
	{
		oParam.m_commandLineParameters.m_gameDuration = 300;
		ORWELL_LOG_DEBUG("by default, game duration = " << oParam.m_commandLineParameters.m_gameDuration);
	}
	if (not oParam.m_commandLineParameters.m_broadcast)
	{
		oParam.m_commandLineParameters.m_broadcast = true;
	}
	if (not oParam.m_commandLineParameters.m_dryRun)
	{
		oParam.m_commandLineParameters.m_dryRun = false;
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
				("orwellrc,r",       value<std::string>(), "Load technical configuration from rc file")
				("gamefile,g",       value<std::string>(), "Load game configuration from game file")
				("tick-interval,T",  value<uint32_t>(),    "Interval in ticks between GameState messages")
				("game-duration,D",  value<uint32_t>(),    "Override the game duration from the game configuration file (in seconds)")
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
	ORWELL_LOG_INFO("Start application.\n");

	if (aVariablesMap.count("orwellrc"))
	{
		oParam.m_commandLineParameters.m_rcFilePath = aVariablesMap["orwellrc"].as<std::string>();
		ORWELL_LOG_DEBUG("orwellrc from command line = " << oParam.m_commandLineParameters.m_rcFilePath);
	}
	if (aVariablesMap.count("gamefile"))
	{
		oParam.m_commandLineParameters.m_gameFilePath = aVariablesMap["gamefile"].as<std::string>();
		ORWELL_LOG_DEBUG("game file from command line = " << oParam.m_commandLineParameters.m_gameFilePath);
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
		oParam.m_commandLineParameters.m_publisherPort = aVariablesMap["publisher-port"].as<uint16_t>();
		ORWELL_LOG_DEBUG("publisher-port from command line = " << oParam.m_commandLineParameters.m_publisherPort);
	}

	if (aVariablesMap.count("puller-port"))
	{
		oParam.m_commandLineParameters.m_pullerPort = aVariablesMap["puller-port"].as<uint16_t>();
		ORWELL_LOG_DEBUG("puller-port from command line = " << oParam.m_commandLineParameters.m_pullerPort);
	}

	if (aVariablesMap.count("agent-port"))
	{
		oParam.m_commandLineParameters.m_agentPort = aVariablesMap["agent-port"].as<uint16_t>();
		ORWELL_LOG_DEBUG("agent-port from command line = " << oParam.m_commandLineParameters.m_agentPort);
	}

	if (aVariablesMap.count("tick-interval"))
	{
		oParam.m_commandLineParameters.m_tickInterval = aVariablesMap["tick-interval"].as<uint32_t>();
		ORWELL_LOG_DEBUG("tick interval from command line = " << oParam.m_commandLineParameters.m_tickInterval);
	}

	if (aVariablesMap.count("game-duration"))
	{
		oParam.m_commandLineParameters.m_gameDuration = aVariablesMap["game-duration"].as< uint32_t >();
		ORWELL_LOG_DEBUG("game duratin from command line = " << oParam.m_commandLineParameters.m_gameDuration);
	}

	if (aVariablesMap.count("dry-run"))
	{
		oParam.m_commandLineParameters.m_dryRun = true;
		ORWELL_LOG_DEBUG("this is a dry run");
	}
	else
	{
		ORWELL_LOG_DEBUG("this is NOT a dry run");
	}

	if (aVariablesMap.count("no-broadcast"))
	{
		oParam.m_commandLineParameters.m_broadcast = false;
		ORWELL_LOG_DEBUG("do not start broadcast server");
	}
	return true;
}

bool Application::ParseParametersFromConfigFile(
		Parameters & ioParam)
{
	ptree aPtree;
	try
	{
		ini_parser::read_ini(*ioParam.m_commandLineParameters.m_rcFilePath, aPtree);
	}
	catch (std::exception const & aExc)
	{
		ORWELL_LOG_ERROR("Could not read technical config file at " << *(ioParam.m_commandLineParameters.m_rcFilePath));
		ORWELL_LOG_DEBUG(aExc.what());
		return false;
	}

	if (not ioParam.m_commandLineParameters.m_publisherPort)
	{
		boost::optional<uint16_t> aPublisherPort = aPtree.get_optional<uint16_t>("server.publisher-port");
		if (aPublisherPort)
		{
			ioParam.m_commandLineParameters.m_publisherPort = aPublisherPort;
			ORWELL_LOG_DEBUG("publisher-port from config file = " << ioParam.m_commandLineParameters.m_publisherPort);
		}
	}
	if (not ioParam.m_commandLineParameters.m_pullerPort)
	{
		boost::optional<uint16_t> aPullerPort = aPtree.get_optional<uint16_t>("server.puller-port");
		if (aPullerPort)
		{
			ioParam.m_commandLineParameters.m_pullerPort = aPullerPort;
			ORWELL_LOG_DEBUG("puller-port from config file = " << ioParam.m_commandLineParameters.m_pullerPort);
		}
	}
	if (not ioParam.m_commandLineParameters.m_agentPort)
	{
		boost::optional<uint16_t> aAgentPort = aPtree.get_optional<uint16_t>("server.agent-port");
		if (aAgentPort)
		{
			ioParam.m_commandLineParameters.m_agentPort = aAgentPort;
			ORWELL_LOG_DEBUG("agent-port from config file = " << ioParam.m_commandLineParameters.m_agentPort);
		}
	}
	if (not ioParam.m_commandLineParameters.m_tickInterval)
	{
		boost::optional<uint16_t> aTickInterval = aPtree.get_optional<uint16_t>("server.tic-interval");
		if (aTickInterval)
		{
			ioParam.m_commandLineParameters.m_tickInterval = aTickInterval;
			ORWELL_LOG_DEBUG("tick interval from config file = " << ioParam.m_commandLineParameters.m_tickInterval );
		}
	}
	if ( ioParam.m_videoPorts.empty() )
	{
		std::string aVideoPortRange;
		try
		{
			aVideoPortRange = aPtree.get< std::string >("server.video-ports");
		}
		catch (std::exception const & iExc)
		{
			ORWELL_LOG_ERROR("video port has not been defined in config file");
			return false;
		}
		std::istringstream aStream(aVideoPortRange);
		uint16_t aBeginPortRange = 0;
		char aSeparator;
		uint16_t aEndPortRange = 0;
		aStream >> aBeginPortRange;
		aStream >> aSeparator;
		aStream >> aEndPortRange;

		if (aEndPortRange == 0)
		{
			aEndPortRange = aBeginPortRange;
		}

		if ( aBeginPortRange > aEndPortRange )
		{
			ORWELL_LOG_ERROR("bad video ports range");
			return false;
		}

		ORWELL_LOG_DEBUG("video port range from config file = " << aBeginPortRange << " to " << aEndPortRange );
		for (uint16_t i = aBeginPortRange ; i <= aEndPortRange ; ++i)
		{
			// insert ports in reverse order
			ioParam.m_videoPorts.insert(ioParam.m_videoPorts.begin(), i);
		}
	}

	return true;
}

void Application::ParseGameConfigFromFile(
		Parameters & ioParam)
{
	ptree aPtree;
	ini_parser::read_ini(*ioParam.m_commandLineParameters.m_gameFilePath, aPtree);

	if (not ioParam.m_commandLineParameters.m_gameDuration)
	{
		ioParam.m_commandLineParameters.m_gameDuration = aPtree.get_optional< uint32_t >("game.duration");
	}

	// deal with the Ruleset
	ioParam.m_ruleset.parseConfig(aPtree.get<std::string>("game.ruleset"), aPtree);

	// list of all teams to add
	boost::optional<std::string> aGameTeams = aPtree.get_optional<std::string>("game.teams");
	// If we have some teams, we need to retrieve them from the ini file itself and add them in the Server's context
	if (aGameTeams)
	{
		std::vector<std::string> aTeamList;
		Application::Tokenize(aGameTeams.get(), aTeamList);

		for (std::string const & aTeam : aTeamList)
		{
			std::string aTeamName = aPtree.get<std::string>(aTeam + ".name");
			ORWELL_LOG_INFO("Pushing team: " << aTeamName);
			ioParam.m_teams.insert(aTeamName);
			// list of all robots to add
			boost::optional<std::string> aGameRobots = aPtree.get_optional<std::string>(aTeam + ".robots");
			// If we have some robots, we need to retrieve them from the ini file itself and add them in the Server's context
			if (aGameRobots)
			{
				std::vector<std::string> aRobotList;
				Application::Tokenize(aGameRobots.get(), aRobotList);

				for (std::string const & aRobot : aRobotList)
				{
					std::string aRobotName = aPtree.get<std::string>(aRobot + ".name");
					ORWELL_LOG_INFO("Pushing robot: " << aRobotName << " ; in team " << aTeamName);
					ioParam.m_robots[aRobot] = Parameters::Robot{aRobotName, aTeamName};
				}
			}
		}
	}

	// list of all items to add
	boost::optional<std::string> aGameItems = aPtree.get_optional<std::string>("game.items");
	if (aGameItems)
	{
		std::vector<std::string> aItemList;
		Application::Tokenize(aGameItems.get(), aItemList);

		for (std::string const & iItem : aItemList)
		{
			std::string aItemName = aPtree.get<std::string>(iItem + ".name");
			std::string aItemType = aPtree.get<std::string>(iItem + ".type");
			std::string aItemRfid = aPtree.get<std::string>(iItem + ".rfid");
			int32_t aItemColour = aPtree.get<int32_t>(iItem + ".colour");
			ORWELL_LOG_INFO("Pushing item: " << aItemName << " (" << aItemType << ")");
			ORWELL_LOG_INFO(" colour: " << aItemColour);
			ORWELL_LOG_INFO(" rfid: '" << aItemRfid << "'");
			std::set< std::string > aSetItemRfid;
			if (not aItemRfid.empty())
			{
				boost::split(aSetItemRfid, aItemRfid, boost::is_any_of(" "));
				for (std::string const & aStr : aSetItemRfid)
				{
					ORWELL_LOG_INFO("  - '" << aStr << "'");
				}
			}
			ioParam.m_items[iItem] = Parameters::Item{aItemName, aItemType, aSetItemRfid, aItemColour};
		}
	}
	boost::optional< std::string > aMapLimitsDeclaration = aPtree.get_optional< std::string >("game.map_limits");
	if (aMapLimitsDeclaration)
	{
		std::vector< std::string > aMapLimitList;
		Application::Tokenize(aMapLimitsDeclaration.get(), aMapLimitList);

		for (std::string const & aMapLimit : aMapLimitList)
		{
			ioParam.m_mapLimits.push_back(game::Landmark::ParseConfig(aMapLimit, aPtree));
		}
	}
}

bool Application::CheckParametersConsistency(Parameters const & iParam)
{
	if (iParam.m_commandLineParameters.m_publisherPort == iParam.m_commandLineParameters.m_pullerPort)
	{
		ORWELL_LOG_ERROR("Publisher and puller ports have the same value (" << iParam.m_commandLineParameters.m_pullerPort << ") which is not allowed.");
		return false;
	}
	if (iParam.m_commandLineParameters.m_publisherPort == iParam.m_commandLineParameters.m_agentPort)
	{
		ORWELL_LOG_ERROR("Publisher and agent ports have the same value (" << iParam.m_commandLineParameters.m_agentPort << ") which is not allowed.");
		return false;
	}
	if (iParam.m_commandLineParameters.m_pullerPort == iParam.m_commandLineParameters.m_agentPort)
	{
		ORWELL_LOG_ERROR("Puller and agent ports have the same value (" << iParam.m_commandLineParameters.m_agentPort << ") which is not allowed.");
		return false;
	}
	if ((*iParam.m_commandLineParameters.m_publisherPort) == 0 or (*iParam.m_commandLineParameters.m_pullerPort == 0))
	{
		ORWELL_LOG_ERROR("Invalid port information. Ports are \n Puller=" << iParam.m_commandLineParameters.m_pullerPort << "\n Publisher=" << iParam.m_commandLineParameters.m_publisherPort);
		return false;
	}

	//each robot needs 1 port for the video retransmission and 1 port to send commands to the associated server
	if (iParam.m_videoPorts.size() < 2 * iParam.m_robots.size())
	{
		ORWELL_LOG_ERROR("Only " << iParam.m_videoPorts.size() << " ports for " << iParam.m_robots.size() << " robots");
		return false;
	}

	bool aFirstRfid;
	std::string aRfidString;
	for (auto aPair : iParam.m_items)
	{
		Parameters::Item aItem = aPair.second;
		if (aItem.m_colour != -1 and not aItem.m_rfids.empty())
		{
			aFirstRfid = true;
			aRfidString = "";
			for (std::string const & aRfid : aItem.m_rfids)
			{
				if (aFirstRfid)
				{
					aFirstRfid = false;
				}
				else
				{
					aRfidString += " ";
				}
				aRfidString += aRfid;
			}
			ORWELL_LOG_ERROR(
					"Item " << aPair.first << " is badly configured. rfid="
					<< aRfidString << ", colour=" << aItem.m_colour);
			return false;
		}
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
	if ((iParam.m_commandLineParameters.m_dryRun) and (*iParam.m_commandLineParameters.m_dryRun))
	{
		initServer(iParam);
		m_state = State::RUNNING;
		ORWELL_LOG_INFO("Exit without starting (dry-run).");
		return;
	}
	if ((iParam.m_commandLineParameters.m_broadcast) and (*iParam.m_commandLineParameters.m_broadcast))
	{
		// Broadcast receiver and main loop are run in separated threads
		pid_t aChildProcess = fork();

		switch (aChildProcess)
		{
			case 0:
			{
				log4cxx::NDC ndc("broadcast");
				ORWELL_LOG_INFO("Child started");
				initBroadcastServer(iParam);
				m_state = State::RUNNING;
				m_broadcastServer->runBroadcastReceiver();
				ORWELL_LOG_INFO("Exit from broadcast server.");
				exit(0);
				//return;
			}
			default:
			{
				log4cxx::NDC ndc("server-game");
				ORWELL_LOG_INFO("Father started, child's pid: " << aChildProcess);
				initServer(iParam);
				m_state = State::RUNNING;
				m_server->loop();
				break;
			}
		}

		ORWELL_LOG_INFO("Father continued");

		if (0 != kill(aChildProcess, SIGTERM))
		{
			ORWELL_LOG_WARN("Try to abort child as terminate failed.");
			kill(aChildProcess, SIGABRT);
		}
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
		log4cxx::NDC ndc("server-game");
		initServer(iParam);
		m_state = State::RUNNING;
		m_server->loop();
	}
	ORWELL_LOG_INFO("Exit normally.");
}

bool Application::stop()
{
	bool aStopped = false;
	if (State::RUNNING == m_state)
	{
		if (nullptr != m_server)
		{
			m_server->stop();
		}
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


void Application::initServer(Parameters const & iParam)
{
	ORWELL_LOG_INFO("Initialize server : publisher tcp://*:" << iParam.m_commandLineParameters.m_publisherPort << " puller tcp://*:" << iParam.m_commandLineParameters.m_pullerPort);

	std::string aAgentAddress = "tcp://*:" + boost::lexical_cast<std::string>(*iParam.m_commandLineParameters.m_agentPort);
	std::string aPublisherAddress = "tcp://*:" + boost::lexical_cast<std::string>(*iParam.m_commandLineParameters.m_publisherPort);
	std::string aPullerAddress = "tcp://*:" + boost::lexical_cast<std::string>(*iParam.m_commandLineParameters.m_pullerPort);

	m_server = new orwell::Server(
			m_systemProxy,
			m_agentProxy,
			iParam.m_ruleset,
			aAgentAddress,
			aPullerAddress,
			aPublisherAddress,
			iParam.m_commandLineParameters.m_tickInterval.get(),
			iParam.m_commandLineParameters.m_gameDuration.get());

	m_availablePorts = iParam.m_videoPorts;
	// temporary hack
	for (auto const & aTeamName : iParam.m_teams)
	{
		m_server->accessContext().addTeam(aTeamName);
	}
	for (auto aPair : iParam.m_robots)
	{
		m_server->accessContext().addRobot(
				aPair.second.m_name,
				aPair.second.m_team,
				popPort(),
				popPort(),
				aPair.first);
	}
	ORWELL_LOG_INFO("number of items found in configuration file: " << iParam.m_items.size());
	for (auto aPair : iParam.m_items)
	{
		Parameters::Item aItem = aPair.second;
		std::shared_ptr<game::Item> aNewItem = game::Item::CreateItem(
				aItem.m_type,
				aItem.m_name,
				aItem.m_rfids,
				aItem.m_colour,
				iParam.m_ruleset);
		ORWELL_LOG_INFO("new item in game config file : " << aNewItem->toLogString());
	}
	m_server->accessContext().setMapLimits(iParam.m_mapLimits);
}

void Application::initBroadcastServer(Parameters const & iParam)
{
	if ((iParam.m_commandLineParameters.m_broadcast) and (*iParam.m_commandLineParameters.m_broadcast))
	{
		std::string aPublisherAddress =
			"tcp://*:" + boost::lexical_cast< std::string >(*iParam.m_commandLineParameters.m_publisherPort);
		std::string aPullerAddress =
			"tcp://*:" + boost::lexical_cast< std::string >(*iParam.m_commandLineParameters.m_pullerPort);
		m_broadcastServer = new orwell::BroadcastServer(aPullerAddress, aPublisherAddress);
	}
}

void Application::Tokenize(
		std::string const & iString,
		std::vector<std::string> & oList)
{
	std::string aToken;

	/* 
	 * This is kind of a hack: I know there are some boost libraries doing the
	 * tokenization but I really wanted to use a lambda function here..
	 */
	std::for_each(iString.begin(), iString.end(), [&](char iChar)
	{
		if (iChar != '|' and iChar != ' ')
		{
			aToken += iChar;
		}
		else
		{
			if (not aToken.empty())
			{
				oList.push_back(aToken);
				aToken.clear();
			}
		}
	});

	if (not aToken.empty())
	{
		oList.push_back(aToken);
	}
}

uint16_t Application::popPort()
{
	if (not m_availablePorts.empty())
	{
		uint16_t aReturnPort = m_availablePorts.back();
		m_availablePorts.pop_back();
		m_takenPorts.push_back(aReturnPort);
		ORWELL_LOG_DEBUG("popPort() -> " << aReturnPort);
		return aReturnPort;
	}
	else
	{
		return 0;
	}
}


} // namespace orwell

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
	bool aSameItems = (iLeft.m_items.size() == iRight.m_items.size());
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
	auto aItemIterator = iLeft.m_items.begin();
	while ((aSameItems) and (iLeft.m_items.end() != aItemIterator))
	{
		std::string const & aKey = aItemIterator->first;
		auto const aFound = iRight.m_items.find(aKey);
		if ((aSameItems = (iRight.m_items.end() != aFound)))
		{
			aSameItems = (aItemIterator->second == aFound->second);
		}
		++aItemIterator;
	}
	return ((iLeft.m_commandLineParameters.m_pullerPort == iRight.m_commandLineParameters.m_pullerPort)
		and (iLeft.m_commandLineParameters.m_publisherPort == iRight.m_commandLineParameters.m_publisherPort)
		and (iLeft.m_commandLineParameters.m_agentPort == iRight.m_commandLineParameters.m_agentPort)
		and (iLeft.m_videoPorts == iRight.m_videoPorts)
		and (iLeft.m_commandLineParameters.m_tickInterval == iRight.m_commandLineParameters.m_tickInterval)
		and (iLeft.m_commandLineParameters.m_rcFilePath == iRight.m_commandLineParameters.m_rcFilePath)
		and (iLeft.m_commandLineParameters.m_gameFilePath == iRight.m_commandLineParameters.m_gameFilePath)
		and (iLeft.m_commandLineParameters.m_dryRun == iRight.m_commandLineParameters.m_dryRun)
		and (iLeft.m_commandLineParameters.m_broadcast == iRight.m_commandLineParameters.m_broadcast)
		and (aSameRobots)
		and (aSameItems)
		and (iLeft.m_teams == iRight.m_teams)
		and (iLeft.m_mapLimits == iRight.m_mapLimits)
		);
}

std::ostream & operator<<(
		std::ostream & ioOstream,
		orwell::Application::Parameters const & iParameters)
{
	ioOstream << "puller port [" << iParameters.m_commandLineParameters.m_pullerPort << "] ; ";
	ioOstream << "publisher port [" << iParameters.m_commandLineParameters.m_publisherPort << "] ; ";
	ioOstream << "agent port [" << iParameters.m_commandLineParameters.m_agentPort << "] ; ";
	ioOstream << "available video ports [";
	for (auto const aPort : iParameters.m_videoPorts)
	{
		ioOstream << aPort << ", ";
	}
	ioOstream << "] ; ";
	ioOstream << "tick interval [" << iParameters.m_commandLineParameters.m_tickInterval << "] ; ";
	ioOstream << "rc file path [" << iParameters.m_commandLineParameters.m_rcFilePath << "] ; ";
	ioOstream << "game config file path [" << iParameters.m_commandLineParameters.m_gameFilePath << "] ; ";
	ioOstream << "dry run [" << iParameters.m_commandLineParameters.m_dryRun << "] ; ";
	ioOstream << "broadcast [" << iParameters.m_commandLineParameters.m_broadcast << "] ";
	ioOstream << "robots [";
	for (auto const aPair : iParameters.m_robots)
	{
		std::string const & aKey = aPair.first;
		orwell::Application::Parameters::Robot const & aRobot = aPair.second;
		ioOstream << aKey << ":" << aRobot.m_name << "@" << aRobot.m_team << ", ";
	}
	bool aFirstRfid;
	for (auto const aPair : iParameters.m_items)
	{
		std::string const & aKey = aPair.first;
		orwell::Application::Parameters::Item const & aItem = aPair.second;
		ioOstream << aKey << ":" << aItem.m_name << "(" << aItem.m_type << "), rfid=";
		aFirstRfid = true;
		for (std::string const & aRfid : aItem.m_rfids)
		{
			if (aFirstRfid)
			{
				aFirstRfid = false;
			}
			else
			{
				ioOstream << " ";
			}
			ioOstream << aRfid;
		}
		ioOstream << ", colour=" << aItem.m_colour;
	}
	ioOstream << "] ; ";
	ioOstream << "teams [";
	std::ostream_iterator< std::string > aOut(ioOstream, ", ");
	std::copy(iParameters.m_teams.begin(), iParameters.m_teams.end(), aOut);
	ioOstream << "] ; ";
	ioOstream << "map limits [";
	for (auto const & aMapLimit : iParameters.m_mapLimits)
	{
		ioOstream << aMapLimit << ", ";
	}
	ioOstream << "] ; ";
	return ioOstream;
}

bool operator==(
		orwell::Application::Parameters::Robot const & iLeft,
		orwell::Application::Parameters::Robot const & iRight)
{
	return ((iLeft.m_name == iRight.m_name)
		and (iLeft.m_team == iRight.m_team));
}

bool operator==(
		orwell::Application::Parameters::Item const & iLeft,
		orwell::Application::Parameters::Item const & iRight)
{
	//return ((iLeft.m_name == iRight.m_name)
		//and (iLeft.m_type == iRight.m_type)
		//and (iLeft.m_rfids == iRight.m_rfids)
		//and (iLeft.m_colour == iRight.m_colour));
	if (not (iLeft.m_name == iRight.m_name))
	{
		ORWELL_LOG_DEBUG("Difference on name");
		ORWELL_LOG_DEBUG("left name = '" << iLeft.m_name << "'");
		ORWELL_LOG_DEBUG("right name = '" << iRight.m_name << "'");
		return false;
	}
	if (not (iLeft.m_type == iRight.m_type))
	{
		ORWELL_LOG_DEBUG("Difference on type");
		return false;
	}
	if (not (iLeft.m_rfids == iRight.m_rfids))
	{
		ORWELL_LOG_DEBUG("Difference on RFIDs");
		return false;
	}
	if (not (iLeft.m_colour == iRight.m_colour))
	{
		ORWELL_LOG_DEBUG("Difference on colour");
		return false;
	}
	return true;
}

