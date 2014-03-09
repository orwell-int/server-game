
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

using namespace orwell;
using namespace boost::program_options;
using namespace boost::property_tree;
using namespace log4cxx;

Application & Application::GetInstance()
{
	static Application m_application;
	return m_application;
}

void Application::run(int argc, char **argv)
{
	initLogger();

	initApplication(argc, argv);
	initConfiguration();
//	initServer();
	
//	m_server->runBroadcastReceiver();
//	m_server->loop();
}

bool Application::stop()
{
	m_server->stop();
	return true;
}

bool Application::initLogger()
{

	m_logger = log4cxx::Logger::getLogger(LOGGER);
	m_logger->setLevel(log4cxx::Level::getDebug());
	
	LOG4CXX_DEBUG(m_logger, "Inited logger");
	
	return true;
}

bool Application::initApplication(int argc, char **argv)
{
	// Parse the command line arguments
	options_description aDescription("Allowed options");
	aDescription.add_options()
	    ("help,h", "produce help message")
	    ("publisher-port,P", value<uint32_t>(), "Publisher port")
	    ("puller-port,p", value<uint32_t>(), "Puller port")
	    ("agent-port,A", value<uint32_t>(), "Agent Port")
	    ("version,v", "Print version number and exits")
    	("tic-interval,T", value<uint32_t>(), "Interval in tics between GameState messages")
    	("orwellrc,r", value<std::string>(), "Load configuration from rc file");
	
	variables_map aVariablesMap;
	store(parse_command_line(argc, argv, aDescription), aVariablesMap);
	notify(aVariablesMap);
	
	if (aVariablesMap.count("help"))
	{
		LOG4CXX_DEBUG(m_logger, "Print help and exit");
		std::cout << aDescription << std::endl;
		return false;
	}
	
	if (aVariablesMap.count("version"))
	{
		LOG4CXX_DEBUG(m_logger, "Print version and exit");
		std::cout << "Version fake" << std::endl;
		return false;
	}
	
	if (aVariablesMap.count("publisher-port"))
	{
		m_publisherPort = aVariablesMap["publisher-port"].as<uint32_t>();
		LOG4CXX_DEBUG(m_logger, "m_publisherPort " << m_publisherPort);
	}
	
	if (aVariablesMap.count("puller-port"))
	{
		m_pullerPort = aVariablesMap["puller-port"].as<uint32_t>();
		LOG4CXX_DEBUG(m_logger, "m_pullerPort " << m_pullerPort);
	}
	
	if (aVariablesMap.count("agent-port"))
	{
		m_agentPort = aVariablesMap["agent-port"].as<uint32_t>();
		LOG4CXX_DEBUG(m_logger, "m_agentPort " << m_agentPort);
	}
	
	if (aVariablesMap.count("tic-interval"))
	{
		m_ticInterval = aVariablesMap["tic-interval"].as<uint32_t>();
		LOG4CXX_DEBUG(m_logger, "m_ticInterval " << m_ticInterval);
	}
	
	if (aVariablesMap.count("orwellrc"))
	{
		m_rcFilePath = aVariablesMap["orwellrc"].as<std::string>();
		LOG4CXX_DEBUG(m_logger, "m_rcFilePath " << m_rcFilePath);
	}
	
	return true;
}

bool Application::initConfiguration()
{
	// Read and parse the ini file (if needed)
	if (not m_rcFilePath.empty())
	{
		
	}
	
	return true;
}
