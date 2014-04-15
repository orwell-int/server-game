#include "orwell/support/GlobalLogger.hpp"

#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/filter/levelrangefilter.h>

#include <iostream>

using namespace log4cxx;
using namespace log4cxx::helpers;

namespace orwell
{
namespace support
{

struct GlobalLogger::Pimpl
{
	Pimpl(
		std::string const & iOutput,
		bool const iDebug);

	~Pimpl();

	PatternLayoutPtr m_patternLayout;
	ConsoleAppenderPtr m_consoleAppender;
	filter::LevelRangeFilterPtr m_levelFilter;
	FileAppenderPtr m_fileApender;
};

GlobalLogger::Pimpl::Pimpl(
		std::string const & iOutput,
		bool const iDebug)
{
	m_patternLayout = new PatternLayout("%d %-5p %x (%F:%L) - %m%n");
	m_consoleAppender = new ConsoleAppender(m_patternLayout);
	m_levelFilter = new filter::LevelRangeFilter();
	if (not iDebug)
	{
		m_levelFilter->setLevelMin(Level::getInfo());

	}
	m_consoleAppender->addFilter(m_levelFilter);
	FileAppenderPtr m_fileApender = new FileAppender(m_patternLayout, iOutput);
	BasicConfigurator::configure(m_fileApender);
	BasicConfigurator::configure(m_consoleAppender);
}

GlobalLogger::Pimpl::~Pimpl()
{
	m_consoleAppender->clearFilters();
	m_fileApender = nullptr;
	m_levelFilter = nullptr;
	m_consoleAppender = nullptr;
	m_patternLayout = nullptr;
}

log4cxx::LoggerPtr GlobalLogger::m_ActiveLogger(nullptr);
GlobalLogger::Pimpl * GlobalLogger::m_Garbage(nullptr);


void GlobalLogger::Create(
		std::string const & iName,
		std::string const & iOutput,
		bool const iDebug)
{
	if (nullptr == GlobalLogger::m_Garbage)
	{
		GlobalLogger::m_Garbage = new GlobalLogger::Pimpl(iOutput, iDebug);
	}
	if (nullptr == GlobalLogger::m_ActiveLogger)
	{
		GlobalLogger::m_ActiveLogger = log4cxx::Logger::getLogger(iName);
		GlobalLogger::m_ActiveLogger->setLevel(log4cxx::Level::getDebug());
	}
}

log4cxx::LoggerPtr GlobalLogger::GetActiveLogger()
{
	return GlobalLogger::m_ActiveLogger;
}

void GlobalLogger::Clear()
{
	delete GlobalLogger::m_Garbage;
	GlobalLogger::m_Garbage = nullptr;
	GlobalLogger::m_ActiveLogger = nullptr;
}

}
}

