#include <stdlib.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "orwell/game/Game.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"

static void Application(uint16_t const iAgentPort)
{
	system(std::string(
				"../server_main -A " + std::to_string(iAgentPort)
				+ " --tic 10").c_str());
}

static void Stopper(uint16_t const iAgentPort)
{
	TestAgent aTestAgent(iAgentPort);
	aTestAgent.sendCommand("ping", std::string("pong"));
	aTestAgent.sendCommand("stop application");
}

int main()
{
	orwell::support::GlobalLogger::Create("test_main", "test_main.log", true);
	log4cxx::NDC ndc("test_main");
	ORWELL_LOG_INFO("Test starts\n");

	uint16_t const aAgentPort{9003};
	std::thread aApplicationThread(Application, aAgentPort);
	std::thread aAgentThread(Stopper, aAgentPort);
	aApplicationThread.join();
	aAgentThread.join();
	orwell::support::GlobalLogger::Clear();
	return 0;
}
