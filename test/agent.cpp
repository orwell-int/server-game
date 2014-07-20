#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "orwell/Application.hpp"
#include "orwell/support/GlobalLogger.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/call_traits.hpp>

#include "Common.hpp"

static void Application(orwell::Application::Parameters const & aParameters)
{
	orwell::Application & aApplication = orwell::Application::GetInstance();
	aApplication.run(aParameters);
}

int main()
{
	orwell::support::GlobalLogger::Create("test_agent", "test_agent.log", true);
	log4cxx::NDC ndc("test_agent");
	ORWELL_LOG_INFO("Test starts\n");

	orwell::Application::Parameters aParameters;
	Arguments aArguments = Common::GetArguments(
			false,
			boost::none,
			boost::none,
			9004,
			boost::none,
			boost::none,
			1,
			100,
			false,
			true,
			true,
			false);
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			aParameters);
	std::thread aApplicationThread(Application, aParameters);
	std::thread aAgentThread(
			Common::SendAgentCommand, "stop application",  *aParameters.m_agentPort, 0);
	aApplicationThread.join();
	aAgentThread.join();
	orwell::support::GlobalLogger::Clear();
	return 0;
}

