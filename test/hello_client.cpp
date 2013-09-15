#include <RawMessage.hpp>

#include <zmq.hpp>
#include <string>

#include <controller.pb.h>
#include <server-game.pb.h>

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
//    aLevelFilter->setLevelMin(Level::getInfo());
    aConsoleAppender->addFilter(aLevelFilter);
    FileAppenderPtr aFileApender = new FileAppender( aPatternLayout, "orwelllog.txt");
    BasicConfigurator::configure(aFileApender);
    BasicConfigurator::configure(aConsoleAppender);
    log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("orwell.log"));
    logger->setLevel(log4cxx::Level::getDebug());

    Sender aPusher("tcp://*:9000", ZMQ_PUSH);
    Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB);

    Hello aHelloMessage;
    aHelloMessage.set_name("jambon");

    LOG4CXX_INFO(logger, "message built Hello (size=" << aHelloMessage.ByteSize() << ")" );

    RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
    aPusher.send( aMessage);

    RawMessage aResponse = aSubscriber.receive();

    Welcome aWelcome;
    aWelcome.ParsePartialFromString(aResponse._payload);


    LOG4CXX_INFO(logger, "message received is (size=" << aWelcome.ByteSize() << ")");
    LOG4CXX_INFO(logger, "message received : robot:" << aWelcome.robot() << "-team:" << aWelcome.team());



    aHelloMessage.set_name("fromage");

    LOG4CXX_INFO(logger, "message built Hello (size=" << aHelloMessage.ByteSize() << ")" );

    RawMessage aMessage2("randomid", "Hello", aHelloMessage.SerializeAsString());
    aPusher.send( aMessage2);

    aResponse = aSubscriber.receive();

    aWelcome.ParsePartialFromString(aResponse._payload);

    LOG4CXX_INFO(logger, "message received is (size=" << aWelcome.ByteSize() << ")");
    LOG4CXX_INFO(logger, "message received : robot:" << aWelcome.robot() << "-team:" << aWelcome.team());




        aHelloMessage.set_name("poulet");

    LOG4CXX_INFO(logger, "message built Hello (size=" << aHelloMessage.ByteSize() << ")" );

    RawMessage aMessage3("randomid", "Hello", aHelloMessage.SerializeAsString());
    aPusher.send( aMessage3);

    aResponse = aSubscriber.receive();

    aWelcome.ParsePartialFromString(aResponse._payload);

    LOG4CXX_INFO(logger, "message received is (size=" << aWelcome.ByteSize() << ")");
    LOG4CXX_INFO(logger, "message received : robot:" << aWelcome.robot() << "-team:" << aWelcome.team());







        aHelloMessage.set_name("rutabagas");

    LOG4CXX_INFO(logger, "message built Hello (size=" << aHelloMessage.ByteSize() << ")" );

    RawMessage aMessage4("randomid", "Hello", aHelloMessage.SerializeAsString());
    aPusher.send( aMessage4);

    aResponse = aSubscriber.receive();

    aWelcome.ParsePartialFromString(aResponse._payload);

    LOG4CXX_INFO(logger, "message received is (size=" << aWelcome.ByteSize() << ")");
    LOG4CXX_INFO(logger, "message received : robot:" << aWelcome.robot() << "-team:" << aWelcome.team());


	return 0;
}

