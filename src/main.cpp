#include "orwell/Server.hpp"

#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/filter/levelrangefilter.h>
#include <log4cxx/ndc.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

#include "orwell/Application.hpp"

using namespace log4cxx;
using namespace std;

static orwell::tasks::Server * ServerPtr;
static void signal_handler(int signum)
{
	ServerPtr->stop();
}

int main(int argc, char *argv[])
{
	PatternLayoutPtr aPatternLayout = new PatternLayout("%d %-5p %x (%F:%L) - %m%n");
	ConsoleAppenderPtr aConsoleAppender = new ConsoleAppender(aPatternLayout);
	filter::LevelRangeFilterPtr aLevelFilter = new filter::LevelRangeFilter();
	aLevelFilter->setLevelMin(Level::getDebug());
	aConsoleAppender->addFilter(aLevelFilter);
	FileAppenderPtr aFileAppender = new FileAppender( aPatternLayout, LOGFILE);
	
	BasicConfigurator::configure(aFileAppender);
	BasicConfigurator::configure(aConsoleAppender);
	
	Application & anOrwellApplication = Application::GetInstance();
	anOrwellApplication.run(argc, argv);
//	anOrwellApplication.stop();
	
//	std::string aString;
//
//	orwell::tasks::Server aServer("tcp://*:9000", "tcp://*:9001", 500, logger);
//	aServer.accessContext().addRobot("Gipsy Danger");
//	aServer.accessContext().addRobot("Goldorak");
//	aServer.accessContext().addRobot("Securitron");
//	
//	// This is needed to handle the signal
//	ServerPtr = &aServer;
//
//	// Register the signal handler
//	signal(SIGINT, signal_handler);
//	signal(SIGTERM, signal_handler);
//	
//	pid_t aChildProcess = fork();
//	switch (aChildProcess)
//	{
//		case 0:
//			LOG4CXX_INFO(logger, "Child started, pid: " << aChildProcess);
//			aServer.runBroadcastReceiver();
//			
//			// The child can stop here
//			return 0;
//			break;
//		default:
//			LOG4CXX_INFO(logger, "Father continued, pid: " << aChildProcess);
//			aServer.loop();
//			break;
//	}
//	
//	// Let's wait for everything to be over.
//	int status;
//	while (waitpid(aChildProcess, &status, WNOHANG) == 0)
//	{
//		LOG4CXX_INFO(logger, "Waiting for child " << aChildProcess << " to complete..");
//		sleep(1);
//	}

	return 0;
}
