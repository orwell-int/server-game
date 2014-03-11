
#include "orwell/Application.hpp"
#include "orwell/Server.hpp"

#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/filter/levelrangefilter.h>
#include <log4cxx/ndc.h>


#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

using namespace orwell;
using namespace boost::program_options;
using namespace boost::property_tree;
using namespace log4cxx;
using std::make_shared;

Application & Application::GetInstance()
{
	static Application m_application;
	return m_application;
}

void Application::run(int argc, char **argv)
{
	/***************************************
	*  Run the server only if all is set  *
	***************************************/
	if (initApplication(argc, argv) and initLogger() and initServer())
	{

		// Broadcast receiver and main loop are run in separated threads
		pid_t aChildProcess = fork();

		switch (aChildProcess)
		{
			case 0:
				LOG4CXX_INFO(m_logger, "Child started");
				m_server->runBroadcastReceiver();
				return;
			default:
				LOG4CXX_INFO(m_logger, "Father started, child's pid: " << aChildProcess);
				m_server->loop();
				break;
		}

		LOG4CXX_INFO(m_logger, "Father continued");

		// Here the father will be waiting for the child to be over
		int aStatus;
		while(waitpid(aChildProcess, &aStatus, WNOHANG) == 0) 
		{
			LOG4CXX_INFO(m_logger, "Waiting for process: " << aChildProcess << " to be over");
			sleep(1);
		}
	}
}

bool Application::stop()
{
	m_server->stop();
	return true;
}

void Application::clean()
{
	if (m_server != nullptr) {
		delete m_server;
	}
}

bool Application::initLogger()
{
	PatternLayoutPtr aPatternLayout = new PatternLayout("%d %-5p %x (%F:%L) - %m%n");

	//Console Log
	ConsoleAppenderPtr aConsoleAppender = new ConsoleAppender(aPatternLayout);
	if ( not m_consoleDebugLogs )
	{
		filter::LevelRangeFilterPtr aLevelFilter = new filter::LevelRangeFilter();
		aLevelFilter->setLevelMin(Level::getInfo());
		aConsoleAppender->addFilter(aLevelFilter);
	}
	BasicConfigurator::configure(aConsoleAppender);

	//File log
	FileAppenderPtr aFileApender = new FileAppender( aPatternLayout, "orwelllog.txt");
	BasicConfigurator::configure(aFileApender);

	//log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("orwell.log"));
	//logger->setLevel(log4cxx::Level::getDebug());

	m_logger = log4cxx::Logger::getLogger("orwell.log");
	m_logger->setLevel(log4cxx::Level::getDebug());
	
	LOG4CXX_DEBUG(m_logger, "Inited logger");
	
	return true;
}

bool Application::initServer()
{
	LOG4CXX_DEBUG(m_logger, "Initialize server : publisher tcp://*:" << m_publisherPort << " puller tcp://*:" << m_pullerPort);

	std::string aPublisherAddress = "tcp://*:" + boost::lexical_cast<std::string>(m_publisherPort);
	std::string aPullerAddress = "tcp://*:" + boost::lexical_cast<std::string>(m_pullerPort);

	m_server = new orwell::tasks::Server(aPullerAddress, aPublisherAddress, 500, m_logger);
	return true;
}

bool Application::initConfigurationFile()
{
	return true;
}

bool Application::initApplication(int argc, char **argv)
{
	// Parse the command line arguments
	options_description aDescription("Allowed options");

	aDescription.add_options()
	    ("help,h", "produce help message")
	    ("publisher-port,P", value<uint32_t>()->default_value(9000), "Publisher port")
	    ("puller-port,p", value<uint32_t>()->default_value(9001), "Puller port")
	    ("agent-port,A", value<uint32_t>(), "Agent Port")
	    ("version,v", "Print version number and exits")
    	("tic-interval,T", value<uint32_t>()->default_value(500), "Interval in tics between GameState messages")
    	("debug-log,d", "Print debug logs on the console")
    	("orwellrc,r", value<std::string>(), "Load configuration from rc file");

	variables_map aVariablesMap;
	store(parse_command_line(argc, argv, aDescription), aVariablesMap);
	notify(aVariablesMap);
	
	m_rcFilePath = "orwell-config.ini";
	if (aVariablesMap.count("orwellrc"))
	{
		m_rcFilePath = aVariablesMap["orwellrc"].as<std::string>();
	}

	if (aVariablesMap.count("help"))
	{
		std::cout << aDescription << std::endl;
		return false;
	}
	
	if (aVariablesMap.count("version"))
	{
		std::cout << "Version fake" << std::endl;
		return false;
	}
	
	if (aVariablesMap.count("publisher-port"))
	{
		m_publisherPort = aVariablesMap["publisher-port"].as<uint32_t>();
	}
	
	if (aVariablesMap.count("puller-port"))
	{
		m_pullerPort = aVariablesMap["puller-port"].as<uint32_t>();
	}
	
	if (aVariablesMap.count("agent-port"))
	{
		m_agentPort = aVariablesMap["agent-port"].as<uint32_t>();
	}
	
	if (aVariablesMap.count("tic-interval"))
	{
		m_ticInterval = aVariablesMap["tic-interval"].as<uint32_t>();
	}
	
	if (aVariablesMap.count("debug-log"))
	{
		m_consoleDebugLogs = true;
	}


	if (m_publisherPort == 0 or m_pullerPort == 0)
	{
		std::cout << "Missing ports informations" << std::endl;
		std::cout << aDescription << std::endl;
		return false;
	}
	
	return true;
}

