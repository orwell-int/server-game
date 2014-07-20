#include "Common.hpp"

#include <iostream>

#include "orwell/com/Url.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "MissingFromTheStandard.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <zmq.hpp>

#include <unistd.h>

#define ARG_HELP "-h"
#define ARG_PUBLISHER_PORT "-P"
#define ARG_PULLER_PORT "-p"
#define ARG_AGENT_PORT "-A"
#define ARG_ORWELLRC "-r"
#define ARG_GAMECONFIG "-g"
#define ARG_TIC_INTERVAL "-T"
#define ARG_GAME_DURATION "-D"
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

Arguments Common::GetArguments(
		bool const iHelp,
		boost::optional< int32_t > const iPublisherPort,
		boost::optional< int32_t > const iPullerPort,
		boost::optional< int32_t > const iAgentPort,
		boost::optional< std::string > const iOrwellRc,
		boost::optional< std::string > const iGameConfigPath,
		boost::optional< int64_t > const iTicInterval,
		boost::optional< int32_t > const iGameDuration,
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
	if (iGameDuration)
	{
		BuildIntArgument(ARG_GAME_DURATION, *iGameDuration, arguments);
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
			ORWELL_LOG_DEBUG("Expected message not received ; timeout ("
					<< aTrueTimeout << ") exceeded: " << aDuration);
		}
	}
	return aReceivedExpectedMessage;
}

uint16_t Common::GetWaitLoops()
{
	ORWELL_LOG_DEBUG("GetWaitLoops");
	zmq::context_t aZmqContext(1);
	orwell::com::Receiver aReceiver(
			"tcp://*:9999",
			ZMQ_PULL,
			orwell::com::ConnectionMode::BIND,
			aZmqContext,
			0);
	orwell::com::Sender aSender(
			"tcp://127.0.0.1:9999",
			ZMQ_PUSH,
			orwell::com::ConnectionMode::CONNECT,
			aZmqContext,
			0);
	boost::posix_time::ptime aBeginTime(boost::posix_time::microsec_clock::local_time());
	aSender.sendString("test");
	std::string aMessage;
	ORWELL_LOG_DEBUG("GetWaitLoops ; start");
	uint16_t aLoops = 0;
	while (not aReceiver.receiveString(aMessage))
	{
		ORWELL_LOG_DEBUG("GetWaitLoops ; sleep");
		usleep(10);
		++aLoops;
	}
	boost::posix_time::ptime aEndTime(boost::posix_time::microsec_clock::local_time());
	boost::posix_time::time_duration const aDuration = aEndTime - aBeginTime;
	ORWELL_LOG_DEBUG("GetWaitLoops ; aLoops = " << aLoops);
	ORWELL_LOG_DEBUG("GetWaitLoops ; aDuration = " << aDuration.total_milliseconds() << "ms");
	bool const aIsSlow(aDuration > boost::posix_time::milliseconds(100));
	ORWELL_LOG_DEBUG("GetWaitLoops ; aIsSlow = " << aIsSlow);
	return aLoops;
}

void Common::PingAndStop(uint16_t const iAgentPort)
{
	usleep(2000 * Common::GetWaitLoops());
	int const aLinger = 10;
	zmq::context_t aZmqContext(1);
	zmq::socket_t aReplySocket(aZmqContext, ZMQ_PULL);
	aReplySocket.setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));
	orwell::com::Url aUrl("tcp", "*", 9004);
	usleep(20 * 1000);
	ORWELL_LOG_DEBUG("Read reply on " << aUrl.toString());
	aReplySocket.bind(aUrl.toString().c_str());
	std::string aMessage("list player 127.0.0.1 9004");
	zmq::message_t aZmqReply;
	bool aReceived = false;
	aUrl.setHost("localhost");
	aUrl.setPort(iAgentPort);
	ORWELL_LOG_DEBUG("send agent command to " << aUrl.toString());
	zmq::socket_t aAgentSocket(aZmqContext, ZMQ_PUB);
	aAgentSocket.setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));
	aAgentSocket.connect(aUrl.toString().c_str());
	usleep(20 * 1000);
	while (not aReceived)
	{
		ORWELL_LOG_DEBUG("send command: " << aMessage);
		zmq::message_t aZmqMessage(aMessage.size());
		memcpy((void *) aZmqMessage.data(), aMessage.c_str(), aMessage.size());
		aAgentSocket.send(aZmqMessage);
		aReceived = aReplySocket.recv(&aZmqReply, ZMQ_NOBLOCK);
		if (not aReceived)
		{
			usleep(20 * 1000);
			aReceived = aReplySocket.recv(&aZmqReply, ZMQ_NOBLOCK);
		}
	}
	ORWELL_LOG_DEBUG("ping / pong");
	aMessage = "stop application";
	zmq::message_t aZmqMessage(aMessage.size());
	memcpy((void *) aZmqMessage.data(), aMessage.c_str(), aMessage.size());
	ORWELL_LOG_DEBUG("send command: " << aMessage);
	aAgentSocket.send(aZmqMessage);
}

void Common::SendStopFromFakeAgent(
		uint16_t const iAgentPort,
		uint64_t const iExtraSleep)
{
	usleep(iExtraSleep);
	zmq::context_t aZmqContext(1);
	zmq::socket_t aAgentSocket(aZmqContext, ZMQ_PUB);
	int const aLinger = 10;
	aAgentSocket.setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));
	orwell::com::Url aUrl("tcp", "localhost", iAgentPort);
	ORWELL_LOG_DEBUG("send agent command to " << aUrl.toString());
	aAgentSocket.connect(aUrl.toString().c_str());
	usleep(20 * 1000); // sleep for 0.020 s
	std::string aMessage("stop application");
	zmq::message_t aZmqMessage(aMessage.size());
	memcpy((void *) aZmqMessage.data(), aMessage.c_str(), aMessage.size());
	// for some reason messages are lost without the sleep
	usleep(2000 * Common::GetWaitLoops());
	ORWELL_LOG_DEBUG("send command: " << aMessage);
	aAgentSocket.send(aZmqMessage);
}

void Common::SendAgentCommand(
		std::string const & iCommand,
		uint16_t const iAgentPort,
		uint64_t const iExtraSleep)
{
	usleep(iExtraSleep);
	zmq::context_t aZmqContext(1);
	zmq::socket_t aAgentSocket(aZmqContext, ZMQ_PUB);
	int const aLinger = 10;
	aAgentSocket.setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));
	orwell::com::Url aUrl("tcp", "localhost", iAgentPort);
	ORWELL_LOG_DEBUG("send agent command \"" << iCommand << "\" to " << aUrl.toString());
	aAgentSocket.connect(aUrl.toString().c_str());
	usleep(20 * 1000); // sleep for 0.020 s
	zmq::message_t aZmqMessage(iCommand.size());
	memcpy((void *) aZmqMessage.data(), iCommand.c_str(), iCommand.size());
	// for some reason messages are lost without the sleep
	usleep(2000 * Common::GetWaitLoops());
	ORWELL_LOG_DEBUG("send command: " << iCommand);
	aAgentSocket.send(aZmqMessage);
}

