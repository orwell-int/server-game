
#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <stdint.h>

#include <boost/optional.hpp>

#include "orwell/AgentProxy.hpp"

namespace orwell {
class Server;
class BroadcastServer;
class AgentProxy;

class Application
{
public :
	struct Parameters
	{
		struct Robot
		{
			std::string m_name;
			std::string m_team;
		};
		typedef std::string Team;
		boost::optional<uint16_t> m_pullerPort;
		boost::optional<uint16_t> m_publisherPort;
		boost::optional<uint16_t> m_agentPort;
		std::vector< uint16_t > m_videoPorts;
		boost::optional<int64_t> m_tickInterval;
		boost::optional< int32_t > m_gameDuration;
		/// path to technical configuration file
		boost::optional<std::string> m_rcFilePath;
		/// path to game configuration file
		boost::optional<std::string> m_gameFilePath;
		boost::optional<bool> m_dryRun;
		boost::optional<bool> m_broadcast;
		std::map<std::string, Robot> m_robots;
		std::set<Team> m_teams;
		boost::optional<std::string> m_gameType;
		boost::optional<std::string> m_gameName;
	};

	virtual ~Application();

	static Application & GetInstance();

	static bool ReadParameters(
			int argc,
			char * argv[],
			Application::Parameters & oParam);
	void run(Parameters const & iParam);
	bool stop();
	void clean();

protected :
	friend class AgentProxy;
	orwell::Server * accessServer(bool const iUnsafe = false);

private:
	Application();
	Application(Application const & iRight);
	Application & operator=(Application const & iRight);

	// Initialization functions

	void initServer(Parameters const & iParam);

	void initBroadcastServer(Parameters const & iParam);

	static void TokenizeRobots(
		std::string const & iRobotsString,
		std::vector<std::string> & oRobotList);

	//Parameter parsing and validation
	static bool ParseParametersFromCommandLine(
			int argc, char * argv[],
			Parameters & oParam);
	static bool ParseParametersFromConfigFile(
			Parameters & ioParam);
	static bool CheckParametersConsistency(Parameters const & iParam);
	static void ParseGameConfigFromFile(
			Parameters & ioParam);
	///takes the last port of m_availableVideoPorts, returns it and put it in m_takenVideoPorts
	uint16_t popPort();

	// Instance of the server running
	orwell::Server * m_server;
	// Broadcast server for UDP discovery
	orwell::BroadcastServer * m_broadcastServer;

	enum class State
	{
		CREATED,
		// it does not seem to be needed
		//INITIALISED,
		RUNNING,
		STOPPED,
	};
	State m_state;
	orwell::AgentProxy m_agentProxy;
	std::vector< uint16_t > m_availableVideoPorts;
	std::vector< uint16_t > m_takenVideoPorts;
};
}

bool operator==(
		orwell::Application::Parameters const & iLeft,
		orwell::Application::Parameters const & iRight);

bool operator!=(
		orwell::Application::Parameters const & iLeft,
		orwell::Application::Parameters const & iRight);

std::ostream & operator<<(
		std::ostream & ioOstream,
		orwell::Application::Parameters const & iParameters);

bool operator==(
		orwell::Application::Parameters::Robot const & iLeft,
		orwell::Application::Parameters::Robot const & iRight);

