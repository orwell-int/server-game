#include "orwell/support/GlobalLogger.hpp"

//#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/filter/levelrangefilter.h>
#include <log4cxx/ndc.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

namespace orwell
{
namespace support
{

log4cxx::LoggerPtr GlobalLogger::_ActiveLogger(nullptr);
std::map< std::string, log4cxx::LoggerPtr > GlobalLogger::m_Loggers;

GlobalLogger::GlobalLogger(
		std::string const & iName,
		std::string const & iOutput,
		bool const iDebug)
{
	//log4cxx::NDC ndc(iName);
	PatternLayoutPtr aPatternLayout = new PatternLayout("%d %-5p %x (%F:%L) - %m%n");
	ConsoleAppenderPtr aConsoleAppender = new ConsoleAppender(aPatternLayout);
	filter::LevelRangeFilterPtr aLevelFilter = new filter::LevelRangeFilter();
	if (not iDebug)
	{
		aLevelFilter->setLevelMin(Level::getInfo());

	}
	aConsoleAppender->addFilter(aLevelFilter);
	FileAppenderPtr aFileApender = new FileAppender(aPatternLayout, iOutput);
	BasicConfigurator::configure(aFileApender);
	BasicConfigurator::configure(aConsoleAppender);
	log4cxx::LoggerPtr aLogger(log4cxx::Logger::getLogger(iName));
	aLogger->setLevel(log4cxx::Level::getDebug());

	GlobalLogger::m_Loggers[iName] = aLogger;
	if (nullptr == GlobalLogger::_ActiveLogger)
	{
		GlobalLogger::_ActiveLogger = aLogger;
	}
}

void GlobalLogger::SwitchToLogger(std::string const & iName)
{
	GlobalLogger::_ActiveLogger = GlobalLogger::m_Loggers[iName];
}

log4cxx::LoggerPtr GlobalLogger::GetActiveLogger()
{
	return GlobalLogger::_ActiveLogger;
}

void GlobalLogger::Clear()
{
	GlobalLogger::m_Loggers.clear();
}

}
}

