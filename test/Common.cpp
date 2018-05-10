#include "Common.hpp"

#include <iostream>

#include "orwell/com/Url.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/Application.hpp"
#include "controller.pb.h"
#include "MissingFromTheStandard.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <unistd.h>

#define ARG_HELP "-h"
#define ARG_PUBLISHER_PORT "-P"
#define ARG_PULLER_PORT "-p"
#define ARG_AGENT_PORT "-A"
#define ARG_REPLIER_PORT "-R"
#define ARG_ORWELLRC "-r"
#define ARG_GAMECONFIG "-g"
#define ARG_TIC_INTERVAL "-T"
#define ARG_GAME_DURATION "-D"
#define ARG_VERSION "-v"
#define ARG_DEBUG_LOG "-d"
#define ARG_NO_BROADCAST "--no-broadcast"
#define ARG_DRY_RUN "-n"
#define ARG_BROADCAST_PORT "-B"


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

Arguments Common::GetArguments(
		orwell::Application::CommandLineParameters const & iCommandLineParams,
		bool const iDebug,
		bool const iHelp,
		bool const iShowVersion
		)
{
	Arguments arguments;
	BuildArgument("FAKE", arguments);
	if (iHelp)
	{
		BuildArgument(ARG_HELP, arguments);
	}
	if (iCommandLineParams.m_publisherPort)
	{
		BuildIntArgument(ARG_PUBLISHER_PORT, *iCommandLineParams.m_publisherPort, arguments);
	}
	if (iCommandLineParams.m_pullerPort)
	{
		BuildIntArgument(ARG_PULLER_PORT, *iCommandLineParams.m_pullerPort, arguments);
	}
	if (iCommandLineParams.m_agentPort)
	{
		BuildIntArgument(ARG_AGENT_PORT, *iCommandLineParams.m_agentPort, arguments);
	}
	if (iCommandLineParams.m_replierPort)
	{
		BuildIntArgument(ARG_REPLIER_PORT, *iCommandLineParams.m_replierPort, arguments);
	}
	if (iCommandLineParams.m_rcFilePath)
	{
		BuildStrArgument(ARG_ORWELLRC, (*iCommandLineParams.m_rcFilePath).c_str(), arguments);
	}
	if (iCommandLineParams.m_gameFilePath)
	{
		BuildStrArgument(ARG_GAMECONFIG, (*iCommandLineParams.m_gameFilePath).c_str(), arguments);
	}
	if (iCommandLineParams.m_tickInterval)
	{
		BuildIntArgument(ARG_TIC_INTERVAL, *iCommandLineParams.m_tickInterval, arguments);
	}
	if (iCommandLineParams.m_gameDuration)
	{
		BuildIntArgument(ARG_GAME_DURATION, *iCommandLineParams.m_gameDuration, arguments);
	}
	if (iShowVersion)
	{
		BuildArgument(ARG_VERSION, arguments);
	}
	if (iDebug)
	{
		BuildArgument(ARG_DEBUG_LOG, arguments);
	}
	if (iCommandLineParams.m_broadcast
			and not *iCommandLineParams.m_broadcast)
	{
		BuildArgument(ARG_NO_BROADCAST, arguments);
	}
	if (iCommandLineParams.m_dryRun
			and *iCommandLineParams.m_dryRun)
	{
		BuildArgument(ARG_DRY_RUN, arguments);
	}
	if (iCommandLineParams.m_broadcastPort)
	{
		BuildIntArgument(ARG_BROADCAST_PORT, *iCommandLineParams.m_broadcastPort, arguments);
	}
	return arguments;
}


bool Common::ExpectMessage(
		std::string const & iType,
		orwell::com::Receiver & iSubscriber,
		orwell::com::RawMessage & oReceived,
		unsigned int const iTimeout)
{
	ORWELL_LOG_DEBUG("GetWaitLoops for message of type " << iType << " for " << iTimeout);
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
				ORWELL_LOG_DEBUG("Discarded message of type " << oReceived._type);
			}
			usleep(10);
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
			ORWELL_LOG_ERROR("Expected message not received (" << iType
					<< ") ; timeout (" << aTrueTimeout
					<< ") exceeded: " << aDuration);
		}
	}
	return aReceivedExpectedMessage;
}

void Common::Synchronize(
		int32_t iServerReplierPort,
		zmq::context_t & ioContext)
{
	std::string aRequesterUrl = "tcp://127.0.0.1:" +
		boost::lexical_cast<std::string>(iServerReplierPort);
	ORWELL_LOG_INFO("create requester");
	orwell::com::Socket aRequester(
			aRequesterUrl,
			ZMQ_REQ,
			orwell::com::ConnectionMode::CONNECT,
			ioContext);

	orwell::messages::Hello aHello;
	aHello.set_name("jambon");
	orwell::com::RawMessage aHelloMessage("randomid", "Hello", aHello.SerializeAsString());
	aRequester.send(aHelloMessage);
	orwell::com::RawMessage aHelloReply;
	aRequester.receive(aHelloReply, true);
}

TestAgent::TestAgent(uint16_t const & iPort) :
		m_zmqContext(1),
		m_agentSocket(
				orwell::com::Url("tcp", "localhost", iPort).toString().c_str(),
				ZMQ_REQ,
				orwell::com::ConnectionMode::CONNECT,
				m_zmqContext,
				0)
{
}

TestAgent::~TestAgent()
{

}

std::string TestAgent::sendCommand(
		std::string const & iCmd,
		boost::optional< std::string > const & iExpectedReply)
{
	ORWELL_LOG_DEBUG("send command: " << iCmd);
	std::string aReply;
	while (true)
	{
		m_agentSocket.sendString(iCmd);
		m_agentSocket.receiveString(aReply, true);
		ORWELL_LOG_INFO("reply = '" << aReply << "'");
		if ((not iExpectedReply) or (*iExpectedReply == aReply))
		{
			break;
		}
		ORWELL_LOG_INFO("try again to send command");
		reset();
		usleep(50);
	}
	return aReply;
}

void TestAgent::reset()
{
	m_agentSocket.reset();
}

TempFile::TempFile(std::string const & iContent)
{
	char aTempName [] = "/tmp/test-file.XXXXXX";
	int aFileDescriptor = mkstemp(aTempName);
	if (-1 == aFileDescriptor)
	{
		std::cerr << "Temporary file not created properly." << std::endl;
		abort();
	}
	write(aFileDescriptor, iContent.c_str(), iContent.size());
	close(aFileDescriptor);
	m_fileName = std::string(aTempName);
}

TempFile::~TempFile()
{
	if (not m_fileName.empty())
	{
		remove(m_fileName.c_str());
		m_fileName.erase();
	}
}
