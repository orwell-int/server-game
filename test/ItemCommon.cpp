#include "ItemCommon.hpp"

#include <mutex>

#include <boost/lexical_cast.hpp>

#include <zmq.hpp>

#include <log4cxx/ndc.h>

#include "robot.pb.h"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/com/Url.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Robot.hpp"

static void const ProxySendsRobotState(
	int32_t iPullerPort,
	int32_t iPublisherPort,
	int32_t iReplierPort,
	std::string const & iRobotId,
	std::vector< FlagAndTime > const & iFlagsAndTimes)
{
	log4cxx::NDC ndc("client");
	zmq::context_t aContext(1);

	orwell::com::Sender aPusher(
			orwell::com::Url("tcp", "localhost", iPullerPort).toString(),
			ZMQ_PUSH,
			orwell::com::ConnectionMode::CONNECT,
			aContext);
	orwell::com::Receiver aSubscriber(
			orwell::com::Url("tcp", "localhost", iPublisherPort).toString(),
			ZMQ_SUB,
			orwell::com::ConnectionMode::CONNECT,
			aContext);

	Common::Synchronize(iReplierPort, aContext);

	orwell::messages::ServerRobotState aRobotState;
	orwell::messages::Rfid * aRfidMessage = aRobotState.add_rfid();

	aRfidMessage->set_status(orwell::messages::ON);
	for (FlagAndTime const & aFlagAndTime : iFlagsAndTimes)
	{
		aRfidMessage->set_rfid(aFlagAndTime.m_flag);
		aRfidMessage->set_timestamp(aFlagAndTime.m_timeStamp);

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
				orwell::messages::GameState aGameState;
				aGameState.ParsePartialFromString(aResponse._payload);

				if (aFlagAndTime.m_hasWinner)
				{
					if ((not aGameState.playing()) and (aGameState.winner() == "TEAM"))
					{
						ORWELL_LOG_DEBUG("Game stopped as expected and team TEAM won.");
						aGotWhatExpected = true;
					}
				}
				else
				{
					assert(not ((not aGameState.playing()) and (aGameState.winner() == "TEAM")));
					// more could be checked here
					aGotWhatExpected = true;
				}
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

ItemTester::ItemTester(
		orwell::Application::Parameters const & iParameters,
		std::vector< FlagAndTime > const & iFlagsAndTimes,
		uint64_t const iScore)
	: m_parameters(iParameters)
	, m_testAgent(iParameters.m_commandLineParameters.m_agentPort.get())
{
	m_score = boost::lexical_cast< std::string >(iScore);
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
			*m_parameters.m_commandLineParameters.m_replierPort,
			aRobotId,
			iFlagsAndTimes);
}

void ItemTester::run()
{
	m_testAgent.sendCommand("start game");
	m_proxySendsRobotStateThread.join();
	usleep(3 * *m_parameters.m_commandLineParameters.m_tickInterval * 1000);
	m_testAgent.sendCommand("get team TEAM score", m_score);
	m_testAgent.sendCommand("stop application");
	m_applicationThread.join();
}
