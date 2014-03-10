#include "orwell/Server.hpp"

#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/filter/levelrangefilter.h>
#include <log4cxx/ndc.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

#include "orwell/Application.hpp"

using namespace log4cxx;
using namespace std;

static void signal_handler(int /*signum*/)
{
	// Stop the application whan a signal is received
	Application::GetInstance().stop();
}

int main(int argc, char *argv[])
{
	// Register the signal handler
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	// Run the application
	Application::GetInstance().run(argc, argv);

	return 0;
}
