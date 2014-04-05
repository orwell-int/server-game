#include <zmq.hpp>

#include <sys/types.h>
#include <cassert>

#include <log4cxx/ndc.h>

#include "orwell/Application.hpp"
#include "orwell/AgentProxy.hpp"

#include "orwell/support/GlobalLogger.hpp"

static void test_1(orwell::Application & ioApplication)
{
	orwell::AgentProxy agentProxy(ioApplication);
	assert(agentProxy.step("stop application"));
	assert(agentProxy.step("add player Player1"));
	assert(agentProxy.step("add robot Robot1"));
	assert(agentProxy.step("start game"));
	assert(agentProxy.step("stop game"));
	assert(agentProxy.step("remove robot Robot1"));
	assert(agentProxy.step("remove player Player1"));
}


int main()
{
	orwell::support::GlobalLogger::Create("application_errors", "orwell.log", true);
	log4cxx::NDC ndc("application_errors");
	orwell::Application & application = orwell::Application::GetInstance();

	test_1(application);

	orwell::support::GlobalLogger::Clear();
	return 0;
}

