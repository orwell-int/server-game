#include <iostream>

#include <Sender.hpp>
#include <RawMessage.hpp>
#include <Receiver.hpp>

#include <ProcessDecider.hpp>
#include <GlobalContext.hpp>

#include <zmq.hpp>

#include "log4cxx/logger.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/consoleappender.h"
#include "log4cxx/fileappender.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "log4cxx/filter/levelrangefilter.h"

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

    LOG4CXX_INFO(logger, "Starting server")

	Sender aPublisher("tcp://*:9001", ZMQ_PUB);
    Receiver aPuller("tcp://*:9000", ZMQ_PULL);

    GlobalContext aContext( aPublisher );
    aContext.addRobot("Gipsy Danger");
    aContext.addRobot("Goldorak");
    aContext.addRobot("Securitron");

	while (true)
	{
	    RawMessage aMessage = aPuller.receive();

	    processDecider::Process( aMessage, aContext );
    }

    return 0;
}
