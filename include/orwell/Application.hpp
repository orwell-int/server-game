
#pragma once

#include <string>
#include <memory>
#include <log4cxx/logger.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/consoleappender.h>

#define LOGFILE "orwelllog.txt"
#define LOGGER  "orwell.log"

namespace orwell { namespace tasks {
class Server;
}}

class Application
{
public:
	virtual ~Application() {};
	static Application & GetInstance();

	void run(int argc, char *argv[]);
	bool stop();

	void clean();
private:
	Application() {};
	Application(Application const & iRight);
	Application & operator=(Application const & iRight);
	
	// Initialization functions
	bool initApplication(int argc, char *argv[]);
	bool initConfigurationFile();
	bool initLogger();
	bool initServer();
	
	// Common logger
	log4cxx::LoggerPtr m_logger;
	
	// Instance of the server running
	orwell::tasks::Server * m_server;
	
	// Configurations retrieved either from rc file or from command line
	// Command line has the priority over the rc file
	uint32_t m_pullerPort;
	uint32_t m_publisherPort;
	uint32_t m_agentPort;
	uint32_t m_ticInterval;
	std::string m_rcFilePath;
	bool m_consoleDebugLogs;

};
