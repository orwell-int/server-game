#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "orwell/Application.hpp"
#include "orwell/com/Url.hpp"
#include "orwell/support/GlobalLogger.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/call_traits.hpp>

#include "Common.hpp"

static void Application(orwell::Application::Parameters const & aParameters)
{
	orwell::Application & aApplication = orwell::Application::GetInstance();
	aApplication.run(aParameters);
}

static void Agent(uint16_t const iAgentPort)
{
	zmq::context_t aZmqContext(1);
	zmq::socket_t aAgentSocket(aZmqContext, ZMQ_PUB);
	int const aLinger = 10;
	aAgentSocket.setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));
	orwell::com::Url aUrl;
	aUrl.setProtocol("tcp");
	aUrl.setHost("localhost");
	aUrl.setPort(iAgentPort);
	usleep(20 * 1000); // sleep for 0.020 s
	ORWELL_LOG_DEBUG("send agent command to " << aUrl.toString());
	aAgentSocket.connect(aUrl.toString().c_str());
	std::string aMessage("stop application");
	zmq::message_t aZmqMessage(aMessage.size());
	memcpy((void *) aZmqMessage.data(), aMessage.c_str(), aMessage.size());
	// for some reason messages are lost without the sleep
	usleep(13 * 1000); // magic number: 12 ; add one for 'security'
	ORWELL_LOG_DEBUG("send command: " << aMessage);
	aAgentSocket.send(aZmqMessage);
}

int main()
{
	orwell::support::GlobalLogger::Create("agent", "test_agent.log", true);
	log4cxx::NDC ndc("agent");

	orwell::Application::Parameters aParameters;
	Arguments aArguments = Common::GetArguments(
			false,
			boost::none,
			boost::none,
			9004,
			boost::none,
			boost::none,
			1,
			false,
			true,
			true,
			false);
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			aParameters);
	std::thread aApplicationThread(Application, aParameters);
	std::thread aAgentThread(Agent, *aParameters.m_agentPort);
	aApplicationThread.join();
	aAgentThread.join();
	orwell::support::GlobalLogger::Clear();
	return 0;
}

