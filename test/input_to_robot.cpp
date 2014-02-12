
#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "controller.pb.h"

#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"
#include "Common.hpp"

#include "MissingFromTheStandard.hpp"

#include <cassert>

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
#include <thread>

using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using std::string;

int g_status = 0;

static void const client(log4cxx::LoggerPtr iLogger)
{
	log4cxx::NDC ndc("client");
	usleep(6 * 1000);
	Sender aPusher("tcp://127.0.0.1:9000", ZMQ_PUSH, orwell::com::ConnectionMode::CONNECT);
	Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, orwell::com::ConnectionMode::CONNECT);
	usleep(6 * 1000);

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

	if ( not Common::ExpectMessage(aType, aSubscriber, aMessage) )
	{
		g_status = -1;
	}
}


static void const server(log4cxx::LoggerPtr iLogger, std::shared_ptr< orwell::tasks::Server > ioServer)
{
	log4cxx::NDC ndc("server");
    ioServer->loopUntilOneMessageIsProcessed();
}

int main()
{
	log4cxx::LoggerPtr logger = Common::SetupLogger("Input");
	log4cxx::NDC ndc("input");
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

