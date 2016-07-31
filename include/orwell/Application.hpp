
#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <stdint.h>

#include <boost/optional.hpp>

#include "orwell/game/Ruleset.hpp"
#include "orwell/game/Landmark.hpp"
#include "orwell/AgentProxy.hpp"

namespace orwell
{

namespace support
{
class ISystemProxy;
} // namespace support

class Server;
class BroadcastServer;
class AgentProxy;


struct Application_CommandLineParameters
{
	boost::optional<uint16_t> m_pullerPort;
	boost::optional<uint16_t> m_publisherPort;
	boost::optional<uint16_t> m_agentPort;
	/// path to technical configuration file
	boost::optional<std::string> m_rcFilePath;
	/// path to game configuration file
	boost::optional<std::string> m_gameFilePath;
	boost::optional<int64_t> m_tickInterval;
	boost::optional< int32_t > m_gameDuration;
	boost::optional<bool> m_dryRun;
	boost::optional<bool> m_broadcast;
};


class Application
{
public :
	// The arguments which can be found in command line.

	typedef Application_CommandLineParameters CommandLineParameters;

	struct Parameters
	{
		struct Robot
		{
			std::string m_name;
			std::string m_team;
		};
		struct Item
		{
			std::string m_name;
			std::string m_type;
			std::set< std::string > m_rfids;
			int32_t m_colour;
		};

		CommandLineParameters m_commandLineParameters;
		typedef std::string Team;
		std::vector< uint16_t > m_videoPorts;
		std::map< std::string, Robot > m_robots;
		std::map< std::string, Item > m_items;
		std::set< Team > m_teams;
		game::Ruleset m_ruleset;
		std::vector< game::Landmark > m_mapLimits;
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
	Application(support::ISystemProxy const & iSystemProxy);
	Application(Application const & iRight);
	Application & operator=(Application const & iRight);

	// Initialization functions

	void initServer(Parameters const & iParam);

	void initBroadcastServer(Parameters const & iParam);

	static void Tokenize(
		std::string const & iString,
		std::vector<std::string> & oList);

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

	support::ISystemProxy const & m_systemProxy;
	/// Instance of the server running
	orwell::Server * m_server;
	/// Broadcast server for UDP discovery
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
	std::vector< uint16_t > m_availablePorts;
	std::vector< uint16_t > m_takenPorts;
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

bool operator==(
		orwell::Application::Parameters::Item const & iLeft,
		orwell::Application::Parameters::Item const & iRight);

