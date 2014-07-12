#include <string>
#include <iosfwd>

#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/none.hpp>

#include "orwell/IAgentProxy.hpp"

#include "gmock/gmock.h"

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

	static uint16_t GetWaitLoops();
	
	static void PingAndStop(uint16_t const iAgentPort);

	static void SendStopFromFakeAgent(
			uint16_t const iAgentPort,
			uint64_t const iExtraSleep = 0);

	static void SendAgentCommand(
			std::string const & iCommand,
			uint16_t const iAgentPort,
			uint64_t const iExtraSleep = 0);
};

class FakeAgentProxy : public orwell::IAgentProxy
{
public :
	MOCK_METHOD1(step, bool(std::string const & iCommand));

	MOCK_METHOD0(stopApplication, void());
	
	MOCK_METHOD2(listRobot, void(
				std::string const & iReplyAddress,
				uint16_t const iReplyPort));

	MOCK_METHOD1(addRobot, void(std::string const & iRobotName));

	MOCK_METHOD1(removeRobot, void(std::string const & iRobotName));

	MOCK_METHOD1(registerRobot, void(std::string const & iRobotName));
	
	MOCK_METHOD1(unregisterRobot, void(std::string const & iRobotName));
	
	MOCK_METHOD3(setRobot, void(
			std::string const & iRobotName,
			std::string const & iProperty,
			std::string const & iValue));

	MOCK_METHOD2(listPlayer, void(
				std::string const & iReplyAddress,
				uint16_t const iReplyPort));

	MOCK_METHOD1(addPlayer, void(std::string const & iPlayerName));

	MOCK_METHOD1(removePlayer, void(std::string const & iPlayerName));

	MOCK_METHOD0(startGame, void());

	MOCK_METHOD0(stopGame, void());
};

std::ostream & operator<<(
		std::ostream & ioOstream,
		Arguments const & iArguments);
