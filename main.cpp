#include <iostream>

#include "Sender.hpp"
#include "RawMessage.hpp"
#include "Receiver.hpp"

#include "ProcessDecider.hpp"
#include "GlobalContext.hpp"
#include "Server.hpp"

#include <zmq.hpp>

#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/filter/levelrangefilter.h>

#include <memory>

using namespace log4cxx;

using namespace std;
using namespace orwell::tasks;
using namespace orwell::com;

int main()
{

	PatternLayoutPtr aPatternLayout = new PatternLayout("%d %-5p (%F:%L) - %m%n");
	ConsoleAppenderPtr aConsoleAppender = new ConsoleAppender(aPatternLayout);
	filter::LevelRangeFilterPtr aLevelFilter = new filter::LevelRangeFilter();
	//  aLevelFilter->setLevelMin(Level::getInfo());
	aConsoleAppender->addFilter(aLevelFilter);
	FileAppenderPtr aFileApender = new FileAppender( aPatternLayout, "orwelllog.txt");
	BasicConfigurator::configure(aFileApender);
	BasicConfigurator::configure(aConsoleAppender);
	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("orwell.log"));
	logger->setLevel(log4cxx::Level::getDebug());

	orwell::tasks::Server aServer("tcp://*:9000", "tcp://*:9001", logger);
	aServer.accessContext().addRobot("Gipsy Danger");
	aServer.accessContext().addRobot("Goldorak");
	aServer.accessContext().addRobot("Securitron");

	while (true)
	{
		aServer.run();
	}

	return 0;
}

