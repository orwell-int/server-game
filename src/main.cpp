#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

#include "orwell/Application.hpp"
#include "orwell/support/GlobalLogger.hpp"

static void signal_handler(int /*signum*/)
{
	// Stop the application whan a signal is received
	orwell::Application::GetInstance().stop();
}

int main(int argc, char *argv[])
{
	orwell::support::GlobalLogger::Create("server_game", "orwell.log", true);
	// Register the signal handler
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	// Run the application
	orwell::Application::GetInstance().run(argc, argv);

	// Clean the application before leaving
	orwell::Application::GetInstance().clean();

	orwell::support::GlobalLogger::Clear();
	return 0;
}

