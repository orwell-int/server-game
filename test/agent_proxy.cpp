#include <zmq.hpp>

#include <sys/types.h>
#include <cassert>

#include <boost/lexical_cast.hpp>

#include <log4cxx/ndc.h>

#include "orwell/Application.hpp"
#include "orwell/AgentProxy.hpp"
#include "orwell/Server.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/Url.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"

static void test_1(orwell::Application & ioApplication)
{
	ORWELL_LOG_DEBUG("test_1");
	orwell::com::Url aUrl;
	aUrl.setHost("0.0.0.0");
	aUrl.setPort(9005);
	zmq::context_t aZmqContext(2);
	orwell::com::Receiver aPuller(
			aUrl.toString(),
			ZMQ_PULL,
			orwell::com::ConnectionMode::BIND,
			aZmqContext,
			0);
	ORWELL_LOG_DEBUG("puller created");
	orwell::AgentProxy aAgentProxy(ioApplication);
	assert(aAgentProxy.step("add player Player1"));
	assert(aAgentProxy.step("add robot Robot1"));
	// list player {
	assert(aAgentProxy.step(
				"list player " + aUrl.getHost() + " "
				+ boost::lexical_cast< std::string >(aUrl.getPort())));
	std::string aPlayerList;
	usleep(2 * 1000); // give enough time to zmq to forward the message
	aPuller.receiveString(aPlayerList);
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	std::string aExpectedPlayerList(R"(Players:
	Player1 -> name = Player1 ; robot = 
)");
	assert(aExpectedPlayerList == aPlayerList);
	// } list player
	// list robot {
	assert(aAgentProxy.step(
				"list robot " + aUrl.getHost() + " "
				+ boost::lexical_cast< std::string >(aUrl.getPort())));
	std::string aRobotList;
	usleep(2 * 1000); // give enough time to zmq to forward the message
	aPuller.receiveString(aRobotList);
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotList(R"(Robots:
	Robot1 -> name = Robot1 ; not registered ; player = 
)");
	assert(aExpectedRobotList == aRobotList);
	// } list robot
	// register robot {
	assert(aAgentProxy.step("register robot Robot1"));
	// make sure that Robot1 is now registered
	assert(aAgentProxy.step(
				"list robot " + aUrl.getHost() + " "
				+ boost::lexical_cast< std::string >(aUrl.getPort())));
	usleep(2 * 1000); // give enough time to zmq to forward the message
	aPuller.receiveString(aRobotList);
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotList2(R"(Robots:
	Robot1 -> name = Robot1 ; registered ; player = 
)");
	assert(aExpectedRobotList2 == aRobotList);
	// } register robot
	// unregister robot {
	assert(aAgentProxy.step("unregister robot Robot1"));
	// make sure that Robot1 is now unregistered
	assert(aAgentProxy.step(
				"list robot " + aUrl.getHost() + " "
				+ boost::lexical_cast< std::string >(aUrl.getPort())));
	usleep(2 * 1000); // give enough time to zmq to forward the message
	aPuller.receiveString(aRobotList);
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	assert(aExpectedRobotList == aRobotList);
	// } unregister robot
	assert(aAgentProxy.step("start game"));
	assert(aAgentProxy.step("stop game"));
	assert(aAgentProxy.step("remove robot Robot1"));
	assert(aAgentProxy.step("remove player Player1"));
	assert(aAgentProxy.step(
				"list player " + aUrl.getHost() + " "
				+ boost::lexical_cast< std::string >(aUrl.getPort())));
	usleep(2 * 1000); // give enough time to zmq to forward the message
	aPuller.receiveString(aPlayerList);
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	aExpectedPlayerList = (R"(Players:
)");
	assert(aExpectedPlayerList == aPlayerList);
	assert(aAgentProxy.step(
				"list robot " + aUrl.getHost() + " "
				+ boost::lexical_cast< std::string >(aUrl.getPort())));
	usleep(2 * 1000); // give enough time to zmq to forward the message
	aPuller.receiveString(aRobotList);
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
)");
	assert(aExpectedRobotList == aRobotList);
	assert(aAgentProxy.step("stop application"));
}


int main()
{
	orwell::support::GlobalLogger::Create("aApplication_errors", "test_agent_proxy.log", true);
	log4cxx::NDC ndc("aApplication_errors");
	{
		orwell::Application & aApplication = orwell::Application::GetInstance();

		Arguments aArguments = Common::GetArguments(
				false, 9001, 9000, 9003,
				boost::none, boost::none, 500,
				false, true, true, true);
		orwell::Application::Parameters aParameters;
		orwell::Application::ReadParameters(
				aArguments.m_argc,
				aArguments.m_argv,
				aParameters);
		aApplication.run(aParameters);
		//usleep(40 * 1000); // sleep for 0.040 s
		test_1(aApplication);
	}

	orwell::support::GlobalLogger::Clear();
	return 0;
}

