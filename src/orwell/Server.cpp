#include "orwell/Server.hpp"

#include <string>
#include <cstring>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <unistd.h>

#include <boost/lexical_cast.hpp>

#include <zmq.hpp>

#include "MissingFromTheStandard.hpp"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/ConnectionMode.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/callbacks/ProcessDecider.hpp"
#include "orwell/callbacks/ProcessTimer.hpp"
#include "orwell/AgentProxy.hpp"

#define UDP_MESSAGE_LIMIT 512

using orwell::com::Receiver;
using orwell::com::Sender;
using orwell::callbacks::ProcessDecider;
using orwell::callbacks::ProcessTimer;

namespace orwell
{

Server::Server(
		support::ISystemProxy const & iSystemProxy,
		orwell::IAgentProxy & ioAgentProxy,
		game::Ruleset const & iRuleset,
		std::string const & iAgentUrl,
		std::string const & iPullUrl,
		std::string const & iPublishUrl,
		std::string const & iReplierUrl,
		long const iTicDuration,
		uint32_t const iGameDuration)
	: m_zmqContext(1)
	, m_agentProxy(ioAgentProxy)
	, m_agentSocket(std::make_shared< orwell::com::Socket >(
				iAgentUrl,
				ZMQ_REP,
				orwell::com::ConnectionMode::BIND,
				m_zmqContext,
				0))
	, m_puller(std::make_shared< Receiver >(
				iPullUrl,
				ZMQ_PULL,
				orwell::com::ConnectionMode::BIND,
				m_zmqContext,
				0))
	, m_publisher(std::make_shared< Sender >(
				iPublishUrl,
				ZMQ_PUB,
				orwell::com::ConnectionMode::BIND,
				m_zmqContext,
				0))
	, m_replier(std::make_shared< orwell::com::Socket >(
				iReplierUrl,
				ZMQ_REP,
				orwell::com::ConnectionMode::BIND,
				m_zmqContext,
				0))
	, m_game(iSystemProxy, boost::posix_time::seconds(iGameDuration), iRuleset, *this)
	, m_decider(m_game, m_publisher, m_replier)
	, m_ticDuration(boost::posix_time::milliseconds(iTicDuration))
	, m_previousTic(boost::posix_time::microsec_clock::local_time())
	, m_mainLoopRunning(false)
	, m_forcedStop(false)
	, m_checkPullerFirst(true)
{
}

Server::~Server()
{
}

bool Server::processMessageIfAvailable()
{
	bool aProcessedMessage = false;
	orwell::com::RawMessage aMessage;
	if (m_checkPullerFirst)
	{
		if (m_puller->receive(aMessage))
		{
			ORWELL_LOG_DEBUG("Message received in processMessageIfAvailable");
			m_decider.process(aMessage, com::Channel::PUBLISH);
			aProcessedMessage = true;
		}
		else if (m_replier->receive(aMessage))
		{
			ORWELL_LOG_DEBUG("Message received in processMessageIfAvailable");
			m_decider.process(aMessage, com::Channel::REPLY);
			aProcessedMessage = true;
		}
	}
	else
	{
		if (m_replier->receive(aMessage))
		{
			ORWELL_LOG_DEBUG("Message received in processMessageIfAvailable");
			m_decider.process(aMessage, com::Channel::REPLY);
			aProcessedMessage = true;
		}
		else if (m_puller->receive(aMessage))
		{
			ORWELL_LOG_DEBUG("Message received in processMessageIfAvailable");
			m_decider.process(aMessage, com::Channel::PUBLISH);
			aProcessedMessage = true;
		}
	}
	m_checkPullerFirst = !m_checkPullerFirst;
	return aProcessedMessage;
}

void Server::loopUntilOneMessageIsProcessed()
{
	bool aMessageHasBeenProcessed = false;
	boost::posix_time::time_duration aDuration;
	boost::posix_time::ptime aCurrentTic;

	feedAgentProxy();
	while (not aMessageHasBeenProcessed)
	{
		aCurrentTic = boost::posix_time::microsec_clock::local_time();
		m_game.setTime(aCurrentTic);
		aDuration = aCurrentTic - m_previousTic;
		if (aDuration < m_ticDuration)
		{
			if (processMessageIfAvailable())
			{
				aMessageHasBeenProcessed = true;
			}
			else
			{
				// sleep a fraction of ticduration
				usleep(m_ticDuration.total_milliseconds() / 10);
			}
		}
		else if (m_forcedStop)
		{
			break;
		}
		else
		{
			feedAgentProxy();
			if (m_game.getIsRunning())
			{
				m_game.step();
				ProcessTimer aProcessTimer(m_game, m_publisher, m_replier);
				aProcessTimer.execute();
			}
			m_previousTic = aCurrentTic;
		}
	}
}

void Server::loop()
{
	m_mainLoopRunning = true;

	while (m_mainLoopRunning)
	{
		loopUntilOneMessageIsProcessed();
	}
}

void Server::stop()
{
	ORWELL_LOG_INFO("Terminating server main loop");
	m_forcedStop = true;
	m_mainLoopRunning = false;
	m_game.stop();
}

orwell::game::Game & Server::accessContext()
{
	return m_game;
}

void Server::feedAgentProxy()
{
	std::string aMessage;
	ORWELL_LOG_TRACE("Try to read agent command ...");
	std::string aReply("KO");
	if (m_agentSocket->receiveString(aMessage))
	{
		ORWELL_LOG_DEBUG("command received: '" << aMessage << "'");
		m_agentProxy.step(aMessage, aReply);
		ORWELL_LOG_TRACE("Reply to the client ...");
		m_agentSocket->sendString(aReply);
	}
}

void Server::feedGreeter()
{
	orwell::com::RawMessage aMessage;
	ORWELL_LOG_TRACE("Try to read greeting message ...");
	std::string aReply("KO");
	if (m_agentSocket->receive(aMessage))
	{
		ORWELL_LOG_DEBUG("Message received is of type '" << aMessage._type << "'");
		m_decider.process(aMessage, com::Channel::REPLY);
		ORWELL_LOG_TRACE("Reply to the client ...");
		m_agentSocket->sendString(aReply);
	}
}
}
