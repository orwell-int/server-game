#include <zmq.hpp>

#include <sys/types.h>
#include <cassert>

#include <log4cxx/ndc.h>

#include "orwell/Application.hpp"
#include "orwell/AgentProxy.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"

static void test_1(orwell::Application & ioApplication)
{
	orwell::AgentProxy aAgentProxy(ioApplication);
	assert(aAgentProxy.step("add player Player1"));
	assert(aAgentProxy.step("add robot Robot1"));
	assert(aAgentProxy.step("start game"));
	assert(aAgentProxy.step("stop game"));
	assert(aAgentProxy.step("remove robot Robot1"));
	assert(aAgentProxy.step("remove player Player1"));
	assert(aAgentProxy.step("stop application"));
}


int main()
{
	orwell::support::GlobalLogger::Create("aApplication_errors", "orwell.log", true);
	log4cxx::NDC ndc("aApplication_errors");
	{
		orwell::Application & aApplication = orwell::Application::GetInstance();

		Arguments aArguments = Common::GetArugments(
				false, 9001, 9000, 9003,
				boost::none, 500,
				false, true, true, true);
		orwell::Application::Parameters aParameters;
		orwell::Application::ReadParameters(
				aArguments.m_argc,
				aArguments.m_argv,
				aParameters);
		aApplication.run(aParameters);
		test_1(aApplication);
	}

	orwell::support::GlobalLogger::Clear();
	return 0;
}

