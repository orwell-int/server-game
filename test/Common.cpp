#include "Common.hpp"

#include "orwell/com/Receiver.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "MissingFromTheStandard.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <unistd.h>

#define ARG_HELP "-h"
#define ARG_PUBLISHER_PORT "-P"
#define ARG_PULLER_PORT "-p"
#define ARG_AGENT_PORT "-A"
#define ARG_ORWELLRC "-r"
#define ARG_GAMECONFIG "-g"
#define ARG_TIC_INTERVAL "-T"
#define ARG_VERSION "-v"
#define ARG_DEBUG_LOG "-d"
#define ARG_NO_BROADCAST "--no-broadcast"
#define ARG_DRY_RUN "-n"


using namespace log4cxx;

Arguments::Arguments()
	: m_argv(nullptr)
	, m_argc(0)
{
}

Arguments::~Arguments()
{
	for (size_t i = 0 ; i < m_argc ; ++i)
	{
		delete [] m_argv[i];
	}
	if (nullptr != m_argv)
	{
		free(m_argv);
		m_argv = nullptr;
	}
	m_argc = 0;
}

Arguments::Arguments(Arguments && oOld)
	: m_argv(oOld.m_argv)
	, m_argc(oOld.m_argc)
{
	oOld.m_argv = nullptr;
	oOld.m_argc = 0;
}

void Arguments::addArgument(char const * const iArgument)
{
	m_argc += 1;
	char ** tmp_argv = (char **)realloc(m_argv, m_argc * sizeof(char *));
	if (nullptr == tmp_argv)
	{
		std::cerr << "Out of memory !" << std::endl;
		throw 1;
	}
	m_argv = tmp_argv;
	size_t aSize = strlen(iArgument) + 1;
	char * aCopy = new char[aSize];
	aCopy[aSize - 1] = '\0';
	strncpy(aCopy, iArgument, aSize - 1);
	m_argv[m_argc - 1] = aCopy;
}

std::ostream & operator<<(
		std::ostream & ioOstream,
		Arguments const & iArguments)
{
	for (size_t i = 0 ; i < iArguments.m_argc ; ++i)
	{
		ioOstream << " " << iArguments.m_argv[i];
	}
	return ioOstream;
}

static void BuildArgument(
		char const * const iValue,
		Arguments & ioArguments)
{
	ioArguments.addArgument(iValue);
}


template< typename INT >
static void BuildIntArgument(
		char const * const iName,
		INT const iValue,
		Arguments & ioArguments)
{
	BuildArgument(iName, ioArguments);
	std::string aString = std::to_string(iValue);
	BuildArgument(aString.c_str(), ioArguments);
}


static void BuildStrArgument(
		char const * const iName,
		char const * const iValue,
		Arguments & ioArguments)
{
	BuildArgument(iName, ioArguments);
	BuildArgument(iValue, ioArguments);
}

Arguments Common::GetArugments(
		bool const iHelp,
		boost::optional< int32_t > const iPublisherPort,
		boost::optional< int32_t > const iPullerPort,
		boost::optional< int32_t > const iAgentPort,
		boost::optional< std::string > const iOrwellRc,
		boost::optional< std::string > const iGameConfigPath,
		boost::optional< int64_t > const iTicInterval,
		bool const iVersion,
		bool const iDebugLog,
		bool const iNoBroadcast,
		bool const iDryRun)
{
	Arguments arguments;
	BuildArgument("FAKE", arguments);
	if (iHelp)
	{
		BuildArgument(ARG_HELP, arguments);
	}
	if (iPublisherPort)
	{
		BuildIntArgument(ARG_PUBLISHER_PORT, *iPublisherPort, arguments);
	}
	if (iPullerPort)
	{
		BuildIntArgument(ARG_PULLER_PORT, *iPullerPort, arguments);
	}
	if (iAgentPort)
	{
		BuildIntArgument(ARG_AGENT_PORT, *iAgentPort, arguments);
	}
	if (iOrwellRc)
	{
		BuildStrArgument(ARG_ORWELLRC, (*iOrwellRc).c_str(), arguments);
	}
	if (iGameConfigPath)
	{
		BuildStrArgument(ARG_GAMECONFIG, (*iGameConfigPath).c_str(), arguments);
	}
	if (iTicInterval)
	{
		BuildIntArgument(ARG_TIC_INTERVAL, *iTicInterval, arguments);
	}
	if (iVersion)
	{
		BuildArgument(ARG_VERSION, arguments);
	}
	if (iDebugLog)
	{
		BuildArgument(ARG_DEBUG_LOG, arguments);
	}
	if (iNoBroadcast)
	{
		BuildArgument(ARG_NO_BROADCAST, arguments);
	}
	if (iDryRun)
	{
		BuildArgument(ARG_DRY_RUN, arguments);
	}
	return arguments;
}


bool Common::ExpectMessage(
		std::string const & iType,
		orwell::com::Receiver & iSubscriber,
		orwell::com::RawMessage & oReceived,
		unsigned int const iTimeout)
{
	ORWELL_LOG_DEBUG("Wait for message of type " << iType << " for " << iTimeout);
	bool aReceivedExpectedMessage(false);
	boost::posix_time::time_duration aTrueTimeout = boost::posix_time::milliseconds(iTimeout);
	boost::posix_time::time_duration aDuration;
	boost::posix_time::ptime aCurrentTime;
	boost::posix_time::ptime aStartTime = boost::posix_time::microsec_clock::local_time();
	while (not aReceivedExpectedMessage and (aDuration < aTrueTimeout))
	{
		aCurrentTime = boost::posix_time::microsec_clock::local_time();
		aDuration = aCurrentTime - aStartTime;
		bool aReceivedAnyMessage = iSubscriber.receive(oReceived);
		if (not aReceivedAnyMessage or oReceived._type != iType)
		{
			if (aReceivedAnyMessage)
			{
				ORWELL_LOG_DEBUG("Discarded message of type " << iType);
			}
			usleep( 10 );
		}
		else
		{
			ORWELL_LOG_DEBUG("Accepted message of type " << iType);
			aReceivedExpectedMessage = true;
		}
	}
	if (not aReceivedExpectedMessage)
	{
		if (aDuration >= aTrueTimeout)
		{
			ORWELL_LOG_DEBUG("Excpected message not received ; timeout ("
					<< aTrueTimeout << ") exceeded: " << aDuration);
		}
	}
	return aReceivedExpectedMessage;
}

