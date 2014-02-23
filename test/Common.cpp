#include "Common.hpp"

#include "orwell/com/Receiver.hpp"
#include "orwell/com/RawMessage.hpp"
#include "MissingFromTheStandard.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/filter/levelrangefilter.h>
#include <log4cxx/ndc.h>

#include <unistd.h>

using namespace log4cxx;

bool Common::ExpectMessage(
		std::string const & iType,
		orwell::com::Receiver & iSubscriber,
		orwell::com::RawMessage & oReceived,
		unsigned int const iTimeout)
{
	bool aReceived(false);
	boost::posix_time::time_duration aTrueTimeout = boost::posix_time::milliseconds(iTimeout);
	boost::posix_time::time_duration aDuration;
	boost::posix_time::ptime aCurrentTime;
	boost::posix_time::ptime aStartTime = boost::posix_time::second_clock::local_time();
	while (not aReceived and (aDuration < aTrueTimeout))
	{
		aCurrentTime = boost::posix_time::second_clock::local_time();
		aDuration = aCurrentTime - aStartTime;
		if (not iSubscriber.receive(oReceived) or oReceived._type != iType)
		{
			usleep( 10 );
		}
		else
		{
			aReceived = true;
		}
	}

	return aReceived;
}

log4cxx::LoggerPtr Common::SetupLogger(
		std::string const & iName,
		bool const iDebug)
{
	log4cxx::NDC ndc(iName);
	PatternLayoutPtr aPatternLayout = new PatternLayout("%d %-5p %x (%F:%L) - %m%n");
	ConsoleAppenderPtr aConsoleAppender = new ConsoleAppender(aPatternLayout);
	filter::LevelRangeFilterPtr aLevelFilter = new filter::LevelRangeFilter();
	if (not iDebug)
	{
		aLevelFilter->setLevelMin(Level::getInfo());

	}
	aConsoleAppender->addFilter(aLevelFilter);
	FileAppenderPtr aFileApender = new FileAppender(aPatternLayout, "orwelllog.txt");
	BasicConfigurator::configure(aFileApender);
	BasicConfigurator::configure(aConsoleAppender);
	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("orwell.log"));
	logger->setLevel(log4cxx::Level::getDebug());

	return logger;
}
