#include "ItemCommon.hpp"

#include <mutex>

#include <boost/lexical_cast.hpp>

#include <zmq.hpp>

#include <log4cxx/ndc.h>

#include "robot.pb.h"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Robot.hpp"

using namespace orwell::messages;

static void const ProxySendsRobotState(
	int32_t iServerPullerPort,
	int32_t iServerPublisherPort,
	std::string const & iRobotId)
{
	log4cxx::NDC ndc("client");
	zmq::context_t aContext(1);

	std::string aPusherUrl = "tcp://127.0.0.1:" + boost::lexical_cast<std::string>(iServerPullerPort);
	std::string aSubscriberUrl = "tcp://127.0.0.1:" + boost::lexical_cast<std::string>(iServerPublisherPort);

	usleep(6 * 1000);

	orwell::com::Sender aPusher(
			aPusherUrl,
			ZMQ_PUSH,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	orwell::com::Receiver aSubscriber(
			aSubscriberUrl,
			ZMQ_SUB,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	usleep(6 * 1000);

	ServerRobotState aRobotState;
	Rfid * aRfidMessage = aRobotState.add_rfid();

	aRfidMessage->set_rfid("myrfidredflag");
	aRfidMessage->set_status(ON);
	aRfidMessage->set_timestamp(0);

	orwell::com::RawMessage aMessage2("Idonotexist", "ServerRobotState", aRobotState.SerializeAsString());
	aPusher.send(aMessage2);
	ORWELL_LOG_INFO("message sent with wrong id = " + aRobotState.SerializeAsString());

	orwell::com::RawMessage aMessage(iRobotId, "ServerRobotState", aRobotState.SerializeAsString());
	aPusher.send(aMessage);
	ORWELL_LOG_INFO("batman message sent = " + aRobotState.SerializeAsString());
	orwell::com::RawMessage aResponse;
	bool aGotWhatExpected(false);
	while (not aGotWhatExpected)
	{
		if (not Common::ExpectMessage("GameState", aSubscriber, aResponse))
		{
			ORWELL_LOG_DEBUG("Expected gamestate but we got something else : " << aResponse._type);
		}
		else
		{
			GameState aGameState;
			aGameState.ParsePartialFromString(aResponse._payload);

			if ((not aGameState.playing()) and (aGameState.winner() == "TEAM"))
			{
				ORWELL_LOG_DEBUG("Game stopped as expected and team TEAM won.");
				aGotWhatExpected = true;
			}
		}
	}
}

static void Application(orwell::Application::Parameters const & aParameters)
{
	orwell::Application & aApplication = orwell::Application::GetInstance();
	ORWELL_LOG_INFO("application gonna start\n");
	aApplication.run(aParameters);
}

TestParameters::TestParameters(
		orwell::Application::CommandLineParameters const & iCommandLineArguments)
{
	Arguments aArguments = Common::GetArguments(
			iCommandLineArguments, true);
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			m_parameters);
}

orwell::Application::Parameters const & TestParameters::getParameters() const
{
	return m_parameters;
}

ItemTester::ItemTester(orwell::Application::Parameters const & iParameters)
	: m_parameters(iParameters)
	, m_testAgent(iParameters.m_commandLineParameters.m_agentPort.get())
{
	std::string aReply;
	m_applicationThread = std::thread(Application, m_parameters);
	m_testAgent.sendCommand("ping", std::string("pong"));
	std::string aRobotId = m_testAgent.sendCommand("get robot toto id", boost::none);
	assert("KO" != aRobotId);
	ORWELL_LOG_INFO("batman robotId = " + aRobotId);
	m_testAgent.sendCommand("get team TEAM score", std::string("0"));
	m_proxySendsRobotStateThread = std::thread(
			ProxySendsRobotState,
			*m_parameters.m_commandLineParameters.m_pullerPort,
			*m_parameters.m_commandLineParameters.m_publisherPort,
			aRobotId);
}

void ItemTester::run()
{
	m_testAgent.sendCommand("start game");
	m_proxySendsRobotStateThread.join();
	usleep(3 * *m_parameters.m_commandLineParameters.m_tickInterval * 1000);
	m_testAgent.sendCommand("get team TEAM score", std::string("1"));
	m_testAgent.sendCommand("stop application");
	m_applicationThread.join();
}

