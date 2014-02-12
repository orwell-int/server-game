#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "controller.pb.h"
#include "server-game.pb.h"

#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"

#include "Common.hpp"

#include <log4cxx/logger.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/ndc.h>

#include <unistd.h>
#include <mutex>
#include <thread>

using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using namespace std;

int g_status = 0;

static void ExpectWelcome( 
		string const & iPlayerName, 
		string const & iExpectedRobotName, 
		Sender & ioPusher,
		Receiver & ioSubscriber)
{
	Hello aHelloMessage;
	aHelloMessage.set_name( iPlayerName );
	RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
	ioPusher.send(aMessage);

	RawMessage aResponse ;
	if ( not Common::ExpectMessage("Welcome", ioSubscriber, aResponse) )
	{
		g_status = -1;
	}

	Welcome aWelcome;
	aWelcome.ParsePartialFromString(aResponse._payload);

	if ( aWelcome.robot() != iExpectedRobotName )
	{
		g_status = -2;
	}
}

static void client(log4cxx::LoggerPtr iLogger)
{
	log4cxx::NDC ndc("client");
	usleep(6 * 1000);
	LOG4CXX_INFO(iLogger, "create pusher");
	Sender aPusher("tcp://127.0.0.1:9000", ZMQ_PUSH, orwell::com::ConnectionMode::CONNECT);
	LOG4CXX_INFO(iLogger, "create subscriber");
	Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, orwell::com::ConnectionMode::CONNECT);
	usleep(6 * 1000);

	ExpectWelcome( "jambon", "Gipsy Danger", aPusher, aSubscriber);
	ExpectWelcome( "fromage", "Goldorak", aPusher, aSubscriber);
	ExpectWelcome( "poulet", "Securitron", aPusher, aSubscriber);

	Hello aHelloMessage;
	aHelloMessage.set_name("rutabagas");

    RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
    aPusher.send(aMessage);

	RawMessage aResponse ;
	if ( not Common::ExpectMessage("Goodbye", aSubscriber, aResponse) )
	{
		g_status = -1;
	}
}


static void const server(log4cxx::LoggerPtr iLogger, std::shared_ptr< orwell::tasks::Server > ioServer)
{
	log4cxx::NDC ndc("server");
	for (int i = 0 ; i < 4 ; ++i )
	{
        ioServer->loopUntilOneMessageIsProcessed();
	}
}

int main()
{
	auto logger = Common::SetupLogger("hello");
	log4cxx::NDC ndc("hello");
	std::shared_ptr< orwell::tasks::Server > aServer = std::make_shared< orwell::tasks::Server >("tcp://*:9000", "tcp://*:9001", 500, logger);
	LOG4CXX_INFO(logger, "server created");
	aServer->accessContext().addRobot("Gipsy Danger");
	aServer->accessContext().addRobot("Goldorak");
	aServer->accessContext().addRobot("Securitron");
	std::thread aServerThread(server, logger, aServer);
	std::thread aClientThread(client, logger);
	aClientThread.join();
	aServerThread.join();
	return g_status;
}

