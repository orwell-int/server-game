
#include <RawMessage.hpp>

#include <zmq.hpp>
#include <string>

#include <controller.pb.h>

#include <Sender.hpp>
#include <Receiver.hpp>

#include "log4cxx/logger.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/consoleappender.h"
#include "log4cxx/fileappender.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "log4cxx/filter/levelrangefilter.h"


using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using namespace std;

int main()
{

    PatternLayoutPtr aPatternLayout = new PatternLayout("%d %-5p (%F:%L) - %m%n");
    ConsoleAppenderPtr aConsoleAppender = new ConsoleAppender(aPatternLayout);
    filter::LevelRangeFilterPtr aLevelFilter = new filter::LevelRangeFilter();
    aLevelFilter->setLevelMin(Level::getInfo());
    aConsoleAppender->addFilter(aLevelFilter);
    FileAppenderPtr aFileApender = new FileAppender( aPatternLayout, "orwelllog.txt");
    BasicConfigurator::configure(aFileApender);
    BasicConfigurator::configure(aConsoleAppender);
    log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("orwell.log"));
    logger->setLevel(log4cxx::Level::getDebug());


    Sender aPusher("tcp://*:9000", ZMQ_PUSH);
    Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB);

    Input aInputMessage;

    aInputMessage.mutable_move()->set_left(10);
    aInputMessage.mutable_move()->set_right(11);

    aInputMessage.mutable_fire()->set_weapon1(true);
    aInputMessage.mutable_fire()->set_weapon2(false);

    string aType = "Input";
    RawMessage aMessage(aType, aInputMessage.SerializeAsString());
    aPusher.send("", aMessage);

    aSubscriber.receive();

    LOG4CXX_INFO(logger, "done")


	return 0;
}

