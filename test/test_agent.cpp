#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "orwell/game/Game.hpp"
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

	orwell::Application::CommandLineParameters aCommandLineArguments;
	aCommandLineArguments.m_agentPort = 9004;
	aCommandLineArguments.m_tickInterval = 1;
	aCommandLineArguments.m_gameDuration = 100;
	aCommandLineArguments.m_dryRun = false;
	aCommandLineArguments.m_broadcast = false;

	orwell::Application::Parameters aParameters;
	Arguments aArguments = Common::GetArguments(
			aCommandLineArguments,
			true);
	orwell::Application::ReadParameters(
			aArguments.m_argc,
			aArguments.m_argv,
			aParameters);
	TestAgent aTestAgent(aParameters.m_commandLineParameters.m_agentPort.get());
	std::thread aApplicationThread(Application, aParameters);
	aTestAgent.sendCommand("stop application");
	aApplicationThread.join();
	orwell::support::GlobalLogger::Clear();
	return 0;
}

