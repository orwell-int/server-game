
#include "RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "controller.pb.h"

#include "Sender.hpp"
#include "Receiver.hpp"
#include "Server.hpp"

#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/filter/levelrangefilter.h>


#include <unistd.h>

using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using std::string;

static int const client(log4cxx::LoggerPtr iLogger)
{
	sleep(1);
	Sender aPusher("tcp://127.0.0.1:9000", ZMQ_PUSH, false);
	Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, false);

	Input aInputMessage;

	aInputMessage.mutable_move()->set_left(10.33);
	aInputMessage.mutable_move()->set_right(11.33);
	aInputMessage.mutable_fire()->set_weapon1(true);
	aInputMessage.mutable_fire()->set_weapon2(false);

	LOG4CXX_INFO(iLogger, "message built (size=" << aInputMessage.ByteSize() << ")");
	LOG4CXX_INFO(iLogger, "message built : left" << aInputMessage.move().left() << "-right" << aInputMessage.move().right());
	LOG4CXX_INFO(iLogger, "message built : w1:" << aInputMessage.fire().weapon1() << "-w2:" << aInputMessage.fire().weapon2());

	string aType = "Input";
	RawMessage aMessage("TANK_0", "Input", aInputMessage.SerializeAsString());
	aPusher.send(aMessage);

	aMessage = aSubscriber.receive();
	Input aInput ;
	aInput.ParsePartialFromString( aMessage._payload );
	LOG4CXX_INFO(iLogger, "message received is (size=" << aInput.ByteSize() << ")");
	LOG4CXX_INFO(iLogger, "message received : left" << aInput.move().left() << "-right" << aInput.move().right());
	LOG4CXX_INFO(iLogger, "message received : w1:" << aInput.fire().weapon1() << "-w2:" << aInput.fire().weapon2());

	LOG4CXX_INFO(iLogger, "done")

	return 0;
}


static int const server(log4cxx::LoggerPtr iLogger)
{
	orwell::tasks::Server aServer("tcp://*:9000", "tcp://*:9001", iLogger);
	LOG4CXX_INFO(iLogger, "server created");
	return aServer.run();
}

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

	int status(-1);
	switch (fork())
	{
		case 0: // child
		{
			status = client(logger);
			break;
		}
		default: // father
		{
			status = server(logger);
			break;
		}
	}
	return status;
}

