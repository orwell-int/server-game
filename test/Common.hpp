#pragma once

#include <string>
#include <iosfwd>

#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/none.hpp>

#include <zmq.hpp>

#include "orwell/proxy/IAgentProxy.hpp"
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

struct Application_CommandLineParameters
{
	boost::optional<int32_t> m_pullerPort;
	boost::optional<int32_t> m_publisherPort;
	boost::optional<int32_t> m_agentPort;
	boost::optional<int32_t> m_replierPort;
	/// path to technical configuration file
	boost::optional<std::string> m_rcFilePath;
	/// path to game configuration file
	boost::optional<std::string> m_gameFilePath;
	boost::optional<int64_t> m_tickInterval;
	boost::optional< int32_t > m_gameDuration;
	boost::optional<bool> m_dryRun;
	boost::optional<bool> m_broadcast;
	boost::optional< int32_t > m_broadcastPort;

	Application_CommandLineParameters() = default;

	Application_CommandLineParameters(
			orwell::Application_CommandLineParameters const & iParameters);
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
			Application_CommandLineParameters const & iCommandLineParams,
			bool const iDebugLog = false,
			bool const iHelp = false,
			bool const iShowVersion = false
	);
	static bool ExpectMessage(
			std::string const & iType,
			orwell::com::Receiver & iSubscriber,
			orwell::com::RawMessage & oReceived,
			unsigned int const iTimeout = 500);

	static void Synchronize(
			int32_t const iServerReplierPort,
			zmq::context_t & ioContext);

	struct Replacement
	{
		std::string const m_search;
		std::string const m_replace;
	};

	static std::string Replace(
			std::string iText,
			std::vector< Replacement> const & iReplacements);
};

class FakeAgentProxy : public orwell::proxy::IAgentProxy
{
public:
	MOCK_METHOD2(step, bool(
				std::string const & iCommand,
				std::string & ioReply));
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

	MOCK_METHOD1(feedAgentProxy, void(bool const iBlocking));

	MOCK_METHOD0(feedGreeter, void());
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

class MinimalistPrinter : public ::testing::EmptyTestEventListener
{
	// Called before a test starts.
	void OnTestStart(::testing::TestInfo const & iTestInfo) override
	{
		ORWELL_LOG_INFO("Test starts (" << iTestInfo.test_case_name() << "."
				<< iTestInfo.name()<< ")\n");
	}

	// Called after a failed assertion or a SUCCESS().
	void OnTestPartResult(::testing::TestPartResult const & iTestPartResult) override
	{
		if (iTestPartResult.failed())
		{
			ORWELL_LOG_ERROR("Failure in " << iTestPartResult.file_name()
					<< ":" << iTestPartResult.line_number() << "\n"
					<< iTestPartResult.summary());
		}
	}

	// Called after a test ends.
	void OnTestEnd(::testing::TestInfo const & iTestInfo) override
	{
		ORWELL_LOG_INFO("Test ends (" << iTestInfo.test_case_name() << "."
				<< iTestInfo.name()<< ")\n");
	}
};

int RunTest(int argc, char ** argv, std::string const& iTestName);

std::ostream & operator<<(
		std::ostream & ioOstream,
		Arguments const & iArguments);
