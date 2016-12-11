#pragma once

#include <string>
#include <iosfwd>

#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/none.hpp>

#include <zmq.hpp>

#include "orwell/IAgentProxy.hpp"
#include "orwell/support/ISystemProxy.hpp"
#include "orwell/com/Socket.hpp"
#include "orwell/IServer.hpp"
#include "orwell/game/IContactHandler.hpp"

#include "gmock/gmock.h"

#if defined(WIN32) || defined(_WIN32)
#  define PATH_SEPARATOR "\\"
#else
#  define PATH_SEPARATOR "/"
#endif

#include "orwell/support/GlobalLogger.hpp"

#define ORWELL_ASSERT(Expected, Received, Message) \
{\
	if (Expected != Received)\
	{\
		ORWELL_LOG_ERROR("expected: " << Expected);\
		ORWELL_LOG_ERROR(Message);\
		return;\
	}\
}\

#define ORWELL_ASSERT_TRUE(Condition, Message) \
{\
	if (!Condition)\
	{\
		ORWELL_LOG_ERROR(Message);\
		return;\
	}\
}\

namespace orwell
{
struct Application_CommandLineParameters;

namespace com
{
class Receiver;
class RawMessage;
}
namespace game
{
class Item;
} // namespace game
} // namespace orwell

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
			orwell::Application_CommandLineParameters const & iCommandLineParams,
			bool const iDebugLog = false,
			bool const iHelp = false,
			bool const iShowVersion = false
	);
	static bool ExpectMessage(
			std::string const & iType,
			orwell::com::Receiver & iSubscriber,
			orwell::com::RawMessage & oReceived,
			unsigned int const iTimeout = 500);
};

class FakeAgentProxy : public orwell::IAgentProxy
{
public:
	MOCK_METHOD2(step, bool(
				std::string const & iCommand,
				std::string & ioReply));

	MOCK_METHOD0(stopApplication, void());
	
	MOCK_METHOD1(listTeam, void(std::string & ioReply));

	MOCK_METHOD1(addTeam, void(std::string const & iTeamName));

	MOCK_METHOD1(removeTeam, void(std::string const & iTeamName));

	MOCK_METHOD3(getTeam, void(
			std::string const & iTeamName,
			std::string const & iProperty,
			std::string & oValue));

	MOCK_METHOD1(listRobot, void(std::string & ioReply));

	MOCK_METHOD2(addRobot, void(
				std::string const & iRobotName,
				std::string const & iTeamName));

	MOCK_METHOD1(removeRobot, void(std::string const & iRobotName));

	MOCK_METHOD1(registerRobot, void(std::string const & iRobotName));
	
	MOCK_METHOD1(unregisterRobot, void(std::string const & iRobotName));
	
	MOCK_METHOD3(setRobot, void(
			std::string const & iRobotName,
			std::string const & iProperty,
			std::string const & iValue));

	MOCK_METHOD3(getRobot, void(
			std::string const & iRobotName,
			std::string const & iProperty,
			std::string & oValue));

	MOCK_METHOD1(listPlayer, void(std::string & ioReply));

	MOCK_METHOD1(addPlayer, void(std::string const & iPlayerName));

	MOCK_METHOD1(removePlayer, void(std::string const & iPlayerName));

	MOCK_METHOD0(startGame, void());

	MOCK_METHOD0(stopGame, void());

	MOCK_METHOD2(getGame, void(
			std::string const & iProperty,
			std::string & oValue));

};

struct TempFile
{
	std::string m_fileName;

	TempFile(std::string const & iContent);

	~TempFile();
};

class TestAgent
{
public:
	TestAgent(uint16_t const & iPort);
	~TestAgent();

	std::string sendCommand(
			std::string const & iCmd,
			boost::optional< std::string > const & iExpectedReply = std::string("OK"));

	void reset();
private:
	zmq::context_t m_zmqContext;
	orwell::com::Socket m_agentSocket;
};


class FakeSystemProxy : public orwell::support::ISystemProxy
{
public:
	MOCK_CONST_METHOD1(mkstemp, int(char * ioTemplate));

	MOCK_CONST_METHOD1(close, int(int const iFileDescriptor));

	MOCK_CONST_METHOD1(system, int(char const * iCommand));
};

class FakeServer: public orwell::IServer
{
public:
	MOCK_METHOD0(processMessageIfAvailable, bool());

	MOCK_METHOD0(loopUntilOneMessageIsProcessed, void());

	/// Loop eternaly to process all incoming messages.
	MOCK_METHOD0(loop, void());

	/// Correctly stop the server
	MOCK_METHOD0(stop, void());

	MOCK_METHOD0(accessContext, orwell::game::Game & ());

	MOCK_METHOD0(feedAgentProxy, void());
};

class FakeContactHandler : public orwell::game::IContactHandler
{
public:
	MOCK_METHOD2(robotIsInContactWith, void(
		std::string const & iRobotId,
		std::shared_ptr< orwell::game::Item > const iItem));

	MOCK_METHOD2(robotDropsContactWith, void(
		std::string const & iRobotId,
		std::shared_ptr< orwell::game::Item > const iItem));
};

std::ostream & operator<<(
		std::ostream & ioOstream,
		Arguments const & iArguments);
