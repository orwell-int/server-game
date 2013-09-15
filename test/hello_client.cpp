#include "RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "controller.pb.h"
#include "server-game.pb.h"

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
using namespace std;


static int const client(log4cxx::LoggerPtr iLogger)
{
	sleep(1);
	LOG4CXX_INFO(iLogger, "create pusher");
	Sender aPusher("tcp://127.0.0.1:9000", ZMQ_PUSH, false);
	LOG4CXX_INFO(iLogger, "create subscriber");
	Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, false);

	Hello aHelloMessage;
	aHelloMessage.set_name("jambon");

	LOG4CXX_INFO(iLogger, "message built Hello (size=" << aHelloMessage.ByteSize() << ")");

	RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
	aPusher.send(aMessage);

	RawMessage aResponse = aSubscriber.receive();

	Welcome aWelcome;
	aWelcome.ParsePartialFromString(aResponse._payload);

	LOG4CXX_INFO(iLogger, "message received is (size=" << aWelcome.ByteSize() << ")");
	LOG4CXX_INFO(iLogger, "message received : robot:" << aWelcome.robot() << "-team:" << aWelcome.team());

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

