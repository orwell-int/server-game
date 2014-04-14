#include <string>
#include <iosfwd>

#include <stdint.h>

#include <boost/optional.hpp>

#define ARG_HELP "-h"
#define ARG_PUBLISHER_PORT "-P"
#define ARG_PULLER_PORT "-p"
#define ARG_AGENT_PORT "-A"
#define ARG_ORWELLRC "-r"
#define ARG_TIC_INTERVAL "-T"
#define ARG_VERSION "-v"
#define ARG_DEBUG_LOG "-d"
#define ARG_NO_BROADCAST "--no-broadcast"
#define ARG_DRY_RUN "-n"

namespace orwell
{
namespace com
{
class Receiver;
class RawMessage;
}
}

struct Arguments
{
	char ** m_argv;
	uint8_t m_argc;

	Arguments();

	~Arguments();

	Arguments(Arguments && oOld);

	void addArgument(char * const argument);
};

class Common
{
public:
	static bool ExpectMessage(
			std::string const & iType,
			orwell::com::Receiver & iSubscriber,
			orwell::com::RawMessage & oReceived,
			unsigned int const iTimeout = 500);


	static Arguments GetArugments(
			bool const iHelp = false,
			boost::optional< int32_t > const iPublisherPort = boost::none,
			boost::optional< int32_t > const iPullerPort = boost::none,
			boost::optional< int32_t > const iAgentPort = boost::none,
			boost::optional< std::string > const iOrwellRc = boost::none,
			boost::optional< int64_t > const iTicInterval = boost::none,
			bool const iVersion = false,
			bool const iDebugLog = false,
			bool const iNoBroadcast = false,
			bool const iDryRun = false);

};

std::ostream & operator<<(
		std::ostream & ioOstream,
		Arguments const & iArguments);
