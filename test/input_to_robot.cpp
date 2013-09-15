
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

    aInputMessage.mutable_move()->set_left(10.33);
    aInputMessage.mutable_move()->set_right(11.33);
    aInputMessage.mutable_fire()->set_weapon1(true);
    aInputMessage.mutable_fire()->set_weapon2(false);

    LOG4CXX_INFO(logger, "message built (size=" << aInputMessage.ByteSize() << ")" );
    LOG4CXX_INFO(logger, "message built : left" << aInputMessage.move().left() << "-right" << aInputMessage.move().right() );
    LOG4CXX_INFO(logger, "message built : w1:" << aInputMessage.fire().weapon1() << "-w2:" << aInputMessage.fire().weapon2() );

    string aType = "Input";
    RawMessage aMessage("TANK_0", "Input", aInputMessage.SerializeAsString());
    aPusher.send( aMessage);

    aMessage = aSubscriber.receive();
    Input aInput ;
    aInput.ParsePartialFromString( aMessage._payload );
    LOG4CXX_INFO(logger, "message received is (size=" << aInput.ByteSize() << ")");
    LOG4CXX_INFO(logger, "message received : left" << aInput.move().left() << "-right" << aInput.move().right() );
    LOG4CXX_INFO(logger, "message received : w1:" << aInput.fire().weapon1() << "-w2:" << aInput.fire().weapon2() );

    LOG4CXX_INFO(logger, "done")


	return 0;
}

