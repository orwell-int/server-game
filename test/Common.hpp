#include <string>
#include <iosfwd>

#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/none.hpp>

#include "orwell/IAgentProxy.hpp"

namespace orwell
{
namespace com
{
class Receiver;
class RawMessage;
}
}

enum class Status
{
	FAIL,
	PASS,
};

struct Arguments
{
	char ** m_argv;
	uint8_t m_argc;

	Arguments();

	~Arguments();

	Arguments(Arguments && oOld);

	void addArgument(char const * const argument);
};

class Common
{
public:
	static Arguments GetArguments(
			bool const iHelp = false,
			boost::optional< int32_t > const iPublisherPort = boost::none,
			boost::optional< int32_t > const iPullerPort = boost::none,
			boost::optional< int32_t > const iAgentPort = boost::none,
			boost::optional< std::string > const iOrwellRc = boost::none,
			boost::optional< std::string > const iGameConfig = boost::none,
			boost::optional< int64_t > const iTicInterval = boost::none,
			bool const iVersion = false,
			bool const iDebugLog = false,
			bool const iNoBroadcast = false,
			bool const iDryRun = false);

	static bool ExpectMessage(
			std::string const & iType,
			orwell::com::Receiver & iSubscriber,
			orwell::com::RawMessage & oReceived,
			unsigned int const iTimeout = 500);
};

class FakeAgentProxy : public orwell::IAgentProxy
{
public :
	/// \return
	///  True if and only if the command was successfully parsed.
	bool step(std::string const & iCommand);

	/// stop application
	void stopApplication();

	/// add robot <name>
	void addRobot(std::string const & iRobotName);

	/// remove robot <name>
	void removeRobot(std::string const & iRobotName);

	/// add player <name>
	void addPlayer(std::string const & iPlayerName);

	/// remove player <name>
	void removePlayer(std::string const & iPlayerName);

	/// start game
	void startGame();

	/// stop game
	void stopGame();
};

std::ostream & operator<<(
		std::ostream & ioOstream,
		Arguments const & iArguments);
