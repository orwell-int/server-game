#include <stdlib.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "orwell/game/Game.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"

static void Application()
{
	system("../server_main -A 9003 --tic 10");
}

static void Stopper()
{
	TestAgent aTestAgent(9003);
	aTestAgent.sendCommand("ping", std::string("pong"));
	aTestAgent.sendCommand("stop application");
}

int main()
{
	orwell::support::GlobalLogger::Create("test_main", "test_main.log", true);
	log4cxx::NDC ndc("test_main");
	ORWELL_LOG_INFO("Test starts\n");

	std::thread aApplicationThread(Application);
	std::thread aAgentThread(Stopper);
	aApplicationThread.join();
	aAgentThread.join();
	orwell::support::GlobalLogger::Clear();
	return 0;
}

