#pragma once

#include <string>
#include <iosfwd>

#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/none.hpp>

#include <zmq.hpp>

#include "orwell/IAgentProxy.hpp"
#include "orwell/com/Socket.hpp"

#include "gmock/gmock.h"

#if defined(WIN32) || defined(_WIN32)
#  define PATH_SEPARATOR "\\"
#else
#  define PATH_SEPARATOR "/"
#endif

namespace orwell
{
class Application_CommandLineParameters;

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
public :
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

std::ostream & operator<<(
		std::ostream & ioOstream,
		Arguments const & iArguments);

