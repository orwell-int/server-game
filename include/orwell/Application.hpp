
#pragma once

#include <string>
#include <vector>
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
		boost::optional<uint16_t> m_pullerPort;
		boost::optional<uint16_t> m_publisherPort;
		boost::optional<uint16_t> m_agentPort;
		boost::optional<int64_t> m_tickInterval;
		boost::optional<std::string> m_rcFilePath;
		boost::optional<bool> m_dryRun;
		boost::optional<bool> m_broadcast;
		std::vector<std::string> m_robotsList;
	};

	virtual ~Application() {};
	static Application & GetInstance();

	static bool ReadParameters(int argc, char * argv[], Application::Parameters & oParam);
	void run(Parameters const & iParam);
	bool stop();
	void clean();

private:
	Application();
	Application(Application const & iRight);
	Application & operator=(Application const & iRight);

	// Initialization functions
	bool initServer(Parameters const & iParam);
	static std::vector<std::string> & tokenizeRobots(std::string const & iRobotsString, Parameters & ioParameters);

	//Parameter parsing and validation
	static bool ParseParametersFromCommandLine(
			int argc, char * argv[],
			Parameters & oParam,
			std::string & oConfigFilePath);
	static bool ParseParametersFromConfigFile(
			Parameters & oParam,
			std::string const & oConfigFilePath);
	static bool CheckParametersConsistency(Parameters const & iParam);

	// Instance of the server running
	orwell::Server * m_server;
	// Broadcast server for UDP discovery
	orwell::BroadcastServer * m_broadcastServer;
};
}

