#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "controller.pb.h"
#include "robot.pb.h"

#include "orwell/Application.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/RawMessage.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/call_traits.hpp>

#include "Common.hpp"

using namespace orwell::com;
using namespace orwell::messages;

bool gPongReceived;
bool gRegisteredReceived;
int g_status = 0;

static void const ClientSendsPing(
	int32_t iServerPullerPort,
	int32_t iServerPublisherPort,
	std::string const & iRobotId)
{
	using namespace orwell::com;
	using namespace orwell::messages;
	log4cxx::NDC ndc("client");
	zmq::context_t aContext(1);

	std::string aPusherUrl = "tcp://127.0.0.1:" +
		boost::lexical_cast<std::string>(iServerPullerPort);
	std::string aSubscriberUrl = "tcp://127.0.0.1:" +
		boost::lexical_cast<std::string>(iServerPublisherPort);

	usleep(6 * 1000);
	Sender aPusher(
			aPusherUrl,
			ZMQ_PUSH,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	Receiver aSubscriber(
			aSubscriberUrl,
			ZMQ_SUB,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	usleep(6 * 1000);

	Ping aPingMessage;
	orwell::messages::Timing * aTiming = aPingMessage.add_timing();
	aTiming->set_logger("client");
	aTiming->set_timestamp(42);

	ORWELL_LOG_INFO("message built (size=" << aPingMessage.ByteSize() << ")");
	std::string aType = "Ping";
	RawMessage aMessage(iRobotId, aType, aPingMessage.SerializeAsString());
	aPusher.send(aMessage);
	RawMessage aResponse;
	if (not Common::ExpectMessage("Pong", aSubscriber, aResponse))
	{
		ORWELL_LOG_ERROR("Expected Pong but received " << aResponse._type);
		g_status = -1;
		gPongReceived = false;
	}
	else
	{
		gPongReceived = true;
		Pong aPongMessage;
		aPongMessage.ParseFromString(aResponse._payload);
		ORWELL_LOG_INFO("aPongMessage.timing_size() = " << aPongMessage.timing_size());
		for (int64_t i = 0 ; i < aPongMessage.timing_size() ; ++i)
		{
			orwell::messages::Timing * aTiming = aPongMessage.mutable_timing(i);
			std::string const aLogger = aTiming->logger();
			ORWELL_LOG_INFO("aLogger = " << aLogger);
			if (not aTiming->has_timestamp())
			{
				ORWELL_LOG_ERROR("Missing timestamp");
				g_status = -8;
				break;
			}
			uint64_t const aTimestamp = aTiming->timestamp();
			ORWELL_LOG_INFO("aTimestamp = " << aTimestamp);
			uint64_t aElapsed = 0;
			if (aTiming->has_elapsed())
			{
				aElapsed = aTiming->elapsed();
			}
			ORWELL_LOG_INFO(
					"pong: '" << aLogger << "' @" << aTimestamp <<
					" elapsed = " << aElapsed);
		}
	}
}

static void ExpectPing(
		Sender & ioPusher,
		Receiver & ioSubscriber)
{
	RawMessage aResponse;
	if (not Common::ExpectMessage("Ping", ioSubscriber, aResponse))
	{
		ORWELL_LOG_ERROR("Expected Ping but received '" << aResponse._type << "'");
		g_status = -7;
	}
	else
	{
		Ping aPing;
		aPing.ParseFromString(aResponse._payload);
		ORWELL_LOG_INFO("Time to reply to ping with pong");
		Pong aPong;

		for (int64_t i = 0 ; i < aPing.timing_size() ; ++i)
		{
			orwell::messages::Timing const & aTiming = aPing.timing(i);
			std::string const aLogger = aTiming.logger();
			uint64_t const aTimestamp = aTiming.timestamp();
			ORWELL_LOG_INFO("pong: '" << aLogger << "' @" << aTimestamp);
			orwell::messages::Timing * aNewTiming = aPong.add_timing();
			aNewTiming->set_logger(aLogger);
			aNewTiming->set_timestamp(aTimestamp);
			if (aTiming.has_elapsed())
			{
				aNewTiming->set_elapsed(aTiming.elapsed());
			}
		}
		orwell::messages::Timing * aNewTiming = aPong.add_timing();
		aNewTiming->set_logger("proxy");
		aNewTiming->set_timestamp(12345);
		aNewTiming->set_elapsed(67);
		ORWELL_LOG_INFO("pong: 'proxy' @12345 elapsed = 67");
		RawMessage aMessage(
				aResponse._routingId,
				"Pong",
				aPong.SerializeAsString());
		ioPusher.send(aMessage);
	}

}

static void ExpectRegistered(
		std::string const & iTemporaryRobotId,
		Sender & ioPusher,
		Receiver & ioSubscriber)
{
	Register aRegisterMessage;
	aRegisterMessage.set_temporary_robot_id(iTemporaryRobotId);
	//aRegisterMessage.set_video_url("http://localhost:80");
	aRegisterMessage.set_image("this is a photo of the robot.jpg");
	RawMessage aMessage(
			iTemporaryRobotId,
			"Register",
			aRegisterMessage.SerializeAsString());
	ioPusher.send(aMessage);

	RawMessage aResponse;
	if (not Common::ExpectMessage("Registered", ioSubscriber, aResponse))
	{
		ORWELL_LOG_ERROR("Expected Registered but received '" << aResponse._type << "'");
		g_status = -2;
	}
	else
	{
		Registered aRegistered;
		aRegistered.ParseFromString(aResponse._payload);
		ORWELL_LOG_INFO("Robot registered");
		gRegisteredReceived = true;
	}
}

static void proxy(
	int32_t iServerPullerPort,
	int32_t iServerPublisherPort)
{
	log4cxx::NDC ndc("proxy");
	ORWELL_LOG_INFO("proxy ...");
	zmq::context_t aContext(1);
	std::string aPusherUrl = "tcp://127.0.0.1:" +
		boost::lexical_cast<std::string>(iServerPullerPort);
	std::string aSubscriberUrl = "tcp://127.0.0.1:" +
		boost::lexical_cast<std::string>(iServerPublisherPort);
	usleep(6 * 1000);
	Sender aPusher(
			aPusherUrl,
			ZMQ_PUSH,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	Receiver aSubscriber(
			aSubscriberUrl,
			ZMQ_SUB,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	usleep(6 * 1000);
	ExpectRegistered("jambon", aPusher, aSubscriber);
	ORWELL_LOG_INFO("after Registered try to get Ping");
	ExpectPing(aPusher, aSubscriber);
	ORWELL_LOG_INFO("quit proxy");
}


static void Application(orwell::Application::Parameters const & aParameters)
{
	orwell::Application & aApplication = orwell::Application::GetInstance();
	ORWELL_LOG_INFO("application gonna start\n");
	aApplication.run(aParameters);
}

int main()
{
	gPongReceived = false;
	gRegisteredReceived = false;
	orwell::support::GlobalLogger::Create(
			"test_ping_pong", "test_ping_pong.log", true);
	log4cxx::NDC ndc("test_ping_pong");
	ORWELL_LOG_INFO("Test starts\n");
	orwell::Application::Parameters aParameters;

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_publisherPort = 9000;
	aCommandLineArguments.m_pullerPort = 9001;
	aCommandLineArguments.m_agentPort = 9003;
	aCommandLineArguments.m_tickInterval = 10;
	aCommandLineArguments.m_gameDuration = 200;
	aCommandLineArguments.m_dryRun = false;
	aCommandLineArguments.m_broadcast = false;
	aCommandLineArguments.m_gameFilePath = "orwell-game.ini";

	Arguments aArguments = Common::GetArguments(
			aCommandLineArguments, true);
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			aParameters);
	TestAgent aTestAgent(aParameters.m_commandLineParameters.m_agentPort.get());
	usleep(100);

	std::string aReply;
	std::thread aApplicationThread(Application, aParameters);
	aTestAgent.sendCommand("ping", std::string("pong"));

	aTestAgent.sendCommand("add team TEAM");
	aTestAgent.sendCommand("add robot toto TEAM");
	std::string const aRobotId =
		aTestAgent.sendCommand("get robot toto id", boost::none);
	assert("KO" != aRobotId);
	std::thread aProxyThread(
			proxy,
			*aParameters.m_commandLineParameters.m_pullerPort,
			*aParameters.m_commandLineParameters.m_publisherPort);
	for (uint32_t i = 0 ; i < 500 ; ++i)
	{
		usleep(100);
		if (gRegisteredReceived)
		{
			break;
		}
	}
	assert(gRegisteredReceived);
	ClientSendsPing(
			*aParameters.m_commandLineParameters.m_pullerPort,
			*aParameters.m_commandLineParameters.m_publisherPort,
			aRobotId);

	for (uint32_t i = 0 ; i < 500 ; ++i)
	{
		usleep(100);
		if (gPongReceived)
		{
			break;
		}
	}
	assert(gPongReceived);
	aProxyThread.join();
	aTestAgent.sendCommand("stop application");
	aApplicationThread.join();
	ORWELL_LOG_INFO("Test ends\n");
	orwell::support::GlobalLogger::Clear();
	return g_status;
}
