
#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "controller.pb.h"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"
#include "Common.hpp"

#include "MissingFromTheStandard.hpp"

#include <cassert>

#include <log4cxx/ndc.h>


#include <unistd.h>
#include <mutex>
#include <thread>

using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using std::string;

int g_status = 0;

static void const client()
{
	log4cxx::NDC ndc("client");
	zmq::context_t aContext(1);
	usleep(6 * 1000);
	Sender aPusher("tcp://127.0.0.1:9000", ZMQ_PUSH, orwell::com::ConnectionMode::CONNECT, aContext);
	Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, orwell::com::ConnectionMode::CONNECT, aContext);
	usleep(6 * 1000);

	Input aInputMessage;

	aInputMessage.mutable_move()->set_left(1);
	aInputMessage.mutable_move()->set_right(1);
	aInputMessage.mutable_fire()->set_weapon1(false);
	aInputMessage.mutable_fire()->set_weapon2(false);

	ORWELL_LOG_INFO("message built (size=" << aInputMessage.ByteSize() << ")");
	ORWELL_LOG_INFO("message built : left" << aInputMessage.move().left() << "-right" << aInputMessage.move().right());
	ORWELL_LOG_INFO("message built : w1:" << aInputMessage.fire().weapon1() << "-w2:" << aInputMessage.fire().weapon2());

	string aType = "Input";
	RawMessage aMessage("TANK_0", "Input", aInputMessage.SerializeAsString());
	aPusher.send(aMessage);

	if ( not Common::ExpectMessage(aType, aSubscriber, aMessage) )
	{
		g_status = -1;
	}
}


static void const server(std::shared_ptr< orwell::Server > ioServer)
{
	log4cxx::NDC ndc("server");
	ioServer->loopUntilOneMessageIsProcessed();
}

int main()
{
	orwell::support::GlobalLogger("input", "test_input.log");
	log4cxx::NDC ndc("input");
	std::shared_ptr< orwell::Server > aServer =
		std::make_shared< orwell::Server >("tcp://*:9000", "tcp://*:9001", 500);
	ORWELL_LOG_INFO("server created");
	aServer->accessContext().addRobot("Gipsy Danger");
	aServer->accessContext().addRobot("Goldorak");
	aServer->accessContext().addRobot("Securitron");
	std::thread aServerThread(server, aServer);
	std::thread aClientThread(client);
	aClientThread.join();
	aServerThread.join();
	orwell::support::GlobalLogger::Clear();
	return g_status;
}

