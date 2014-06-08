#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

#include "orwell/Application.hpp"
#include "orwell/support/GlobalLogger.hpp"

static void signal_handler(int iSignum)
{
	std::cerr << "Signal received: " << iSignum << std::endl;
	// Stop the application whan a signal is received
	orwell::Application::GetInstance().stop();
}

int main(int argc, char *argv[])
{
	// Register the signal handler
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	// Run the application
	orwell::Application::Parameters aParameters;
	//int aErrorCode; todo
	if (orwell::Application::ReadParameters(argc, argv, aParameters))
	{
		//aErrorCode = orwell::Application::GetInstance().run(aParameters); todo
		orwell::Application::GetInstance().run(aParameters);
	}

	// Clean the application before leaving
	orwell::Application::GetInstance().clean();

	orwell::support::GlobalLogger::Clear();
	return 0;
	//return aErrorCode; todo
}

