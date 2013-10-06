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
#include <log4cxx/ndc.h>


#include <unistd.h>
#include <mutex>

std::mutex g_pages_mutex;

using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using namespace std;


static int const client(log4cxx::LoggerPtr iLogger)
{
	log4cxx::NDC ndc("client");
	g_pages_mutex.unlock();
	g_pages_mutex.lock();
	usleep(6 * 1000);
	LOG4CXX_INFO(iLogger, "create pusher");
	Sender aPusher("tcp://127.0.0.1:9000", ZMQ_PUSH, orwell::com::ConnectionMode::CONNECT);
	LOG4CXX_INFO(iLogger, "create subscriber");
	Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, orwell::com::ConnectionMode::CONNECT);
	usleep(6 * 1000);

	Hello aHelloMessage;
	aHelloMessage.set_name("jambon");

	LOG4CXX_INFO(iLogger, "message built Hello (size=" << aHelloMessage.ByteSize() << ")");

	RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
	aPusher.send(aMessage);

	RawMessage aResponse ;
	while ( not aSubscriber.receive(aResponse) )
	{
        usleep( 10 );
	}

	Welcome aWelcome;
	aWelcome.ParsePartialFromString(aResponse._payload);

	LOG4CXX_INFO(iLogger, "1 message received is (size=" << aWelcome.ByteSize() << ")");
	LOG4CXX_INFO(iLogger, "1 message received : robot:" << aWelcome.robot() << "-team:" << aWelcome.team());

    aHelloMessage.set_name("fromage");

    LOG4CXX_INFO(iLogger, "1 message built Hello (size=" << aHelloMessage.ByteSize() << ")");

	RawMessage aMessage2("randomid", "Hello", aHelloMessage.SerializeAsString());
    aPusher.send(aMessage2);

    while ( not aSubscriber.receive(aResponse) )
	{
        usleep( 10 );
	}

    aWelcome.ParsePartialFromString(aResponse._payload);

    LOG4CXX_INFO(iLogger, "2 message received is (size=" << aWelcome.ByteSize() << ")");
    LOG4CXX_INFO(iLogger, "2 message received : robot:" << aWelcome.robot() << "-team:" << aWelcome.team());

	aHelloMessage.set_name("poulet");

	LOG4CXX_INFO(iLogger, "2 message built Hello (size=" << aHelloMessage.ByteSize() << ")");

	RawMessage aMessage3("randomid", "Hello", aHelloMessage.SerializeAsString());
	aPusher.send(aMessage3);

	while ( not aSubscriber.receive(aResponse) )
	{
        usleep( 10 );
	}

	aWelcome.ParsePartialFromString(aResponse._payload);

	LOG4CXX_INFO(iLogger, "3 message received is (size=" << aWelcome.ByteSize() << ")");
	LOG4CXX_INFO(iLogger, "3 message received : robot:" << aWelcome.robot() << "-team:" << aWelcome.team());

	aHelloMessage.set_name("rutabagas");

    LOG4CXX_INFO(iLogger, "3 message built Hello (size=" << aHelloMessage.ByteSize() << ")");

    RawMessage aMessage4("randomid", "Hello", aHelloMessage.SerializeAsString());
    aPusher.send(aMessage4);

	bool aReceivedGoodbye(false);
	while (not aReceivedGoodbye)
	{
		while ( not aSubscriber.receive(aResponse) )
		{
			usleep( 10 );
		}

		aReceivedGoodbye = ("Goodbye" == aResponse._type);
	}

	Goodbye aGoodbye;
	aGoodbye.ParsePartialFromString(aResponse._payload);
	LOG4CXX_INFO(iLogger, "4 message received is (size=" << aGoodbye.ByteSize() << ")");
    LOG4CXX_INFO(iLogger, "goodbye message received");

	g_pages_mutex.unlock();
	return 0;
}


static int const server(log4cxx::LoggerPtr iLogger)
{
	log4cxx::NDC ndc("server");
	orwell::tasks::Server aServer("tcp://*:9000", "tcp://*:9001", 500, iLogger);
	LOG4CXX_INFO(iLogger, "server created");
	aServer.accessContext().addRobot("Gipsy Danger");
	aServer.accessContext().addRobot("Goldorak");
	aServer.accessContext().addRobot("Securitron");
	g_pages_mutex.unlock();

	for (int i = 0 ; i < 4 ; ++i )
	{
        aServer.loopUntilOneMessageIsProcessed();
	}
	return 0;
}

int main()
{
	log4cxx::NDC ndc("hello");
	PatternLayoutPtr aPatternLayout = new PatternLayout("%d %-5p %x (%F:%L) - %m%n");
	ConsoleAppenderPtr aConsoleAppender = new ConsoleAppender(aPatternLayout);
	filter::LevelRangeFilterPtr aLevelFilter = new filter::LevelRangeFilter();
	//    aLevelFilter->setLevelMin(Level::getInfo());
	aConsoleAppender->addFilter(aLevelFilter);
	FileAppenderPtr aFileApender = new FileAppender(aPatternLayout, "orwelllog.txt");
	BasicConfigurator::configure(aFileApender);
	BasicConfigurator::configure(aConsoleAppender);
	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("orwell.log"));
	logger->setLevel(log4cxx::Level::getDebug());

	g_pages_mutex.lock();
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

