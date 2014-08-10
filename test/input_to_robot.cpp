#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "controller.pb.h"

#include "orwell/Application.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/RawMessage.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/call_traits.hpp>

#include "Common.hpp"

bool gOK;

static void const ClientSendsInput(
	int32_t iServerPullerPort,
	int32_t iServerPublisherPort,
	std::string const & iRobotId)
{
	using namespace orwell::com;
	using namespace orwell::messages;
	log4cxx::NDC ndc("client");
	zmq::context_t aContext(1);

	std::string aPusherUrl = "tcp://127.0.0.1:" + boost::lexical_cast<std::string>(iServerPullerPort);
	std::string aSubscriberUrl = "tcp://127.0.0.1:" + boost::lexical_cast<std::string>(iServerPublisherPort);

	usleep(6 * 1000);
	Sender aPusher(aPusherUrl, ZMQ_PUSH, orwell::com::ConnectionMode::CONNECT, aContext);
	Receiver aSubscriber(aSubscriberUrl, ZMQ_SUB, orwell::com::ConnectionMode::CONNECT, aContext);
	usleep(6 * 1000);

	Input aInputMessage;
	aInputMessage.mutable_move()->set_left(1);
	aInputMessage.mutable_move()->set_right(1);
	aInputMessage.mutable_fire()->set_weapon1(true);
	aInputMessage.mutable_fire()->set_weapon2(false);

	ORWELL_LOG_INFO("message built (size=" << aInputMessage.ByteSize() << ")");
	ORWELL_LOG_INFO("message built : left" << aInputMessage.move().left() << "-right" << aInputMessage.move().right());
	ORWELL_LOG_INFO("message built : w1:" << aInputMessage.fire().weapon1() << "-w2:" << aInputMessage.fire().weapon2());
	std::string aType = "Input";
	RawMessage aMessage(iRobotId, "Input", aInputMessage.SerializeAsString());
	aPusher.send(aMessage);
	if ( not Common::ExpectMessage(aType, aSubscriber, aMessage, 100) )
	{
		gOK = false;
	}
	else
	{
		gOK = true;
	}
}

static void Application(orwell::Application::Parameters const & aParameters)
{
	orwell::Application & aApplication = orwell::Application::GetInstance();
	ORWELL_LOG_INFO("application gonna start\n");
	aApplication.run(aParameters);
}

int main()
{
	orwell::support::GlobalLogger::Create("test_input", "test_input.log", true);
	log4cxx::NDC ndc("test_input");
	ORWELL_LOG_INFO("Test starts\n");
	orwell::Application::Parameters aParameters;
	Arguments aArguments = Common::GetArguments(
			false,
			9000,
			9001,
			9003,
			boost::none,
			boost::none,
			10,
			200,
			false,
			true,
			true,
			false);
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			aParameters);
	TestAgent aTestAgent(aParameters.m_agentPort.get());
	usleep(100);

	std::string aReply;
	std::thread aApplicationThread(Application, aParameters);
	aReply = aTestAgent.sendCommand("ping", std::string("pong"));

	aReply = aTestAgent.sendCommand("add robot toto");
	std::string aRobotId = aTestAgent.sendCommand("get robot toto id", boost::none);
	assert("KO" != aRobotId);
	std::thread aClientSendsInputThread(
			ClientSendsInput,
			*aParameters.m_pullerPort,
			*aParameters.m_publisherPort,
			aRobotId);
	aClientSendsInputThread.join();
	assert(not gOK); // because the game is not started yet, the Input message must be dropped by the server
	char aPath[FILENAME_MAX];
	getcwd(aPath, sizeof(aPath));
	std::string aFullPath = std::string(aPath) + PATH_SEPARATOR + "master.part";
	ORWELL_LOG_INFO("Path to stream file: " + aFullPath);
	aReply = aTestAgent.sendCommand("set robot toto video_url " + aFullPath);
//	aReply = aTestAgent.sendCommand("add player titi");
	aReply = aTestAgent.sendCommand("start game");
	// this cannot work as the call to read in the video server is only made when sending the video
	//{
		//aReply = aTestAgent.sendCommand("get robot toto video_url", boost::none);
		//size_t aIndex = aReply.rfind(":");
		//uint16_t aPort = boost::lexical_cast< uint16_t >(aReply.substr(aIndex + 1));

		//usleep(2000000);
		//TestAgent aTestClient(aPort);
		//aTestClient.sendCommand("ping", std::string("pong"));
	//}
	//if (false)
	{
		aReply = aTestAgent.sendCommand("get robot toto video_port", boost::none);
		std::string aCommand("cd server-web && make client ARGS='-u http://127.0.0.1:" + aReply + " -l 9020'");
		int aCode = system(aCommand.c_str());
		ORWELL_LOG_INFO("fake client started (return code is " << aCode << ").");
		//usleep(200000);
	}
	TestAgent aFakeClientConnector(9020);
	aFakeClientConnector.sendCommand("ping", std::string("pong"));
	{
		aReply = aTestAgent.sendCommand("get robot toto video_command_port", boost::none);
		ORWELL_LOG_INFO("DEBUG/=");
		size_t aIndex = aReply.rfind(":");
		uint16_t aPort = boost::lexical_cast< uint16_t >(aReply.substr(aIndex + 1));

		//usleep(200000);
		TestAgent aTestClient(aPort);
		ORWELL_LOG_INFO("DEBUG/ping video server");
		aTestClient.sendCommand("ping", std::string("pong"));
		ClientSendsInput(
				*aParameters.m_pullerPort,
				*aParameters.m_publisherPort,
				aRobotId);
		aTestClient.sendCommand("status", std::string("captured = 1"));
	}

	assert(gOK);
	aReply = aTestAgent.sendCommand("stop game");
	aFakeClientConnector.sendCommand("stop", std::string("stopping"));
	ClientSendsInput(
			*aParameters.m_pullerPort,
			*aParameters.m_publisherPort,
			aRobotId);
	assert(not gOK);
	aReply = aTestAgent.sendCommand("stop application");
	aApplicationThread.join();
	ORWELL_LOG_INFO("Test ends\n");
	//orwell::support::GlobalLogger::Clear();
	return 0;
}
