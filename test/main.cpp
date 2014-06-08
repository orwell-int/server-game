#include <stdlib.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"

static void Application()
{
	system("../server_main -A 9003 --tic 10");
}

static void Stopper()
{
	Common::PingAndStop(9003);
}

int main()
{
	orwell::support::GlobalLogger::Create("main", "test_main.log", true);
	log4cxx::NDC ndc("main");
	ORWELL_LOG_INFO("Test starts\n");

	std::thread aApplicationThread(Application);
	std::thread aAgentThread(Stopper);
	aApplicationThread.join();
	aAgentThread.join();
	orwell::support::GlobalLogger::Clear();
	return 0;
}

