
#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

#include <boost/optional.hpp>

namespace orwell {
class Server;
class BroadcastServer;

class Application
{
public:
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
		boost::optional<int64_t> m_tickInterval;
		boost::optional<std::string> m_rcFilePath;
		boost::optional<bool> m_dryRun;
		boost::optional<bool> m_broadcast;
		std::map<std::string, Robot> m_robots;
		std::set<Team> m_teams;
		boost::optional<std::string> m_gameType;
		boost::optional<std::string> m_gameName;
	};

	virtual ~Application() {};
	static Application & GetInstance();

	static bool ReadParameters(
			int argc,
			char * argv[],
			Application::Parameters & oParam);
	void run(Parameters const & iParam);
	bool stop();
	void clean();

private:
	Application();
	Application(Application const & iRight);
	Application & operator=(Application const & iRight);

	// Initialization functions
	bool initServer(Parameters const & iParam);
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

	// Instance of the server running
	orwell::Server * m_server;
	// Broadcast server for UDP discovery
	orwell::BroadcastServer * m_broadcastServer;
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

