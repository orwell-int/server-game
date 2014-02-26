#include "orwell/Server.hpp"

#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/filter/levelrangefilter.h>
#include <log4cxx/ndc.h>

#include <stdio.h>
#include <signal.h>

using namespace log4cxx;
using namespace std;

static orwell::tasks::Server * ServerPtr;

void signal_handler(int signum)
{
	ServerPtr->stop();
}

int main()
{
	std::string aString;

	PatternLayoutPtr aPatternLayout = new PatternLayout("%d %-5p %x (%F:%L) - %m%n");
	ConsoleAppenderPtr aConsoleAppender = new ConsoleAppender(aPatternLayout);
	filter::LevelRangeFilterPtr aLevelFilter = new filter::LevelRangeFilter();
	aLevelFilter->setLevelMin(Level::getInfo());
	aConsoleAppender->addFilter(aLevelFilter);
	FileAppenderPtr aFileApender = new FileAppender( aPatternLayout, "orwelllog.txt");
	BasicConfigurator::configure(aFileApender);
	BasicConfigurator::configure(aConsoleAppender);
	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("orwell.log"));
	logger->setLevel(log4cxx::Level::getDebug());

	orwell::tasks::Server aServer("tcp://*:9000", "tcp://*:9001", 500, logger);
	ServerPtr = &aServer;
	aServer.accessContext().addRobot("Gipsy Danger");
	aServer.accessContext().addRobot("Goldorak");
	aServer.accessContext().addRobot("Securitron");
	
	// Register the signal handler
	signal(SIGINT, signal_handler);
	signal(SIGKILL, signal_handler);
	
	pid_t aChildProcess = fork();
	switch (aChildProcess)
	{
		case 0:
			LOG4CXX_INFO(logger, "Child started, pid: " << aChildProcess);
			aServer.runBroadcastReceiver();
			break;
		default:
			LOG4CXX_INFO(logger, "Father continued, pid: " << aChildProcess);
			aServer.loop();
			
			// Propagate the signal to the child
			kill(aChildProcess, SIGTERM);
			break;
	}

	return 0;
}
