#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

#include <log4cxx/ndc.h>

#include "orwell/Application.hpp"
#include "orwell/support/GlobalLogger.hpp"

static void signal_handler(int iSignum)
{
	std::cerr << "In process " << getpid() << " ; Signal received: " << iSignum << std::endl;
	// Stop the application when a signal is received
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
		log4cxx::NDC ndc("main");
		orwell::Application::GetInstance().run(aParameters);
	}

	// Clean the application before leaving
	orwell::Application::GetInstance().clean();

	orwell::support::GlobalLogger::Clear();
	std::cerr << "In process " << getpid() << " ; exit 0" << std::endl;
	return 0;
	//return aErrorCode; todo
}

