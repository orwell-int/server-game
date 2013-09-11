#include <iostream>
#include <RawMessage.hpp>

#include <zmq.hpp>
#include <string>
#include <unistd.h>
#include <url.hpp>

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

const uint32_t _port(9000);

// takes as sole argument the expected value of the ID that is sent back from the server if the registration goes well.
void requestRegistration( int iExpectedId )
{
/*
    cout << "Robot starting" << endl;

//logging
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
//end logging

    LOG4CXX_DEBUG(logger, "Start logger DEBUG")

    LOG4CXX_INFO(logger, "Start logger INFO")

Sender aPublisher("tcp://*:9001", ZMQ_PUB);

*/
/*
    Input aInputMessage;

    aInputMessage.mutable_move()->set_left(10);
    aInputMessage.mutable_move()->set_right(11);

    aInputMessage.mutable_fire()->set_weapon1(true);
    aInputMessage.mutable_fire()->set_weapon2(false);
string astr = "Input";
RawMessage aMessage(astr, aInputMessage.SerializeAsString());

cout << "batman " <<  aInputMessage.SerializeAsString() << endl;
/*
while (true){
    aPublisher.send("",aMessage);
    sleep(2);
}
*/



 //   Sender aPusher("tcp://*:9000", ZMQ_PUSH);
//    Receiver aSubscriber("tcp://*:9001", ZMQ_SUB);
   /* while (true){
        RawMessage aRaw = aSubscriber.receive();
        cout << "received" << aRaw._type << endl;
    }*/

/*
    Sender aPublisher("tcp://*:9000", ZMQ_PUB);
    Input aInputMessage;

    aInputMessage.mutable_move()->set_left(10);
    aInputMessage.mutable_move()->set_right(11);

    aInputMessage.mutable_fire()->set_weapon1(true);
    aInputMessage.mutable_fire()->set_weapon2(false);
string astr = "Input";
RawMessage aMessage(astr, aInputMessage.SerializeAsString());
    aPublisher.send("", aMessage);
    */
/*
   // initilize the message
    Hello aHelloMessage;
    aHelloMessage.set_name("bidule");

    string aHello = "Hello";

    RawMessage aMessage(aHello, aHelloMessage.SerializeAsString());

    aPusher.send("", aMessage);

    aSubscriber.receive();
    aPusher.send("", aMessage);

    aSubscriber.receive();
    aPusher.send("", aMessage);

    aSubscriber.receive();

    cout << "Robot shutting down" << endl;
    return;
*/

}


int main()
{
	requestRegistration( 0 );

	cout << "done " << endl;

	return 0;
}

