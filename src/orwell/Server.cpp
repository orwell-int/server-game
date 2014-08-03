#include "orwell/Server.hpp"

#include <string>
#include <cstring>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <unistd.h>

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
		orwell::IAgentProxy & ioAgentProxy,
		std::string const & iAgentUrl,
		std::string const & iPullUrl,
		std::string const & iPublishUrl,
		long const iTicDuration,
		uint32_t const iGameDuration)
	: _zmqContext(1)
	, m_agentProxy(ioAgentProxy)
	, m_agentSocket(std::make_shared< orwell::com::Socket >(
				iAgentUrl,
				ZMQ_REP,
				orwell::com::ConnectionMode::BIND,
				_zmqContext,
				0))
	, _puller(std::make_shared< Receiver >(
				iPullUrl,
				ZMQ_PULL,
				orwell::com::ConnectionMode::BIND,
				_zmqContext,
				0))
	, _publisher(std::make_shared< Sender >(
				iPublishUrl,
				ZMQ_PUB,
				orwell::com::ConnectionMode::BIND,
				_zmqContext,
				0))
	, _game(boost::posix_time::milliseconds(iGameDuration))
	, _decider(_game, _publisher)
	, _ticDuration( boost::posix_time::milliseconds(iTicDuration) )
	, _previousTic(boost::posix_time::microsec_clock::local_time())
	, _mainLoopRunning(false)
	, _forcedStop(false)
{
}

Server::~Server()
{
}

bool Server::processMessageIfAvailable()
{
	bool aProcessedMessage = false;
	orwell::com::RawMessage aMessage;
	if (_puller->receive(aMessage))
	{
		ORWELL_LOG_DEBUG("Message received in processMessageIfAvailable");
		_decider.process(aMessage);
		aProcessedMessage = true;
	}
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
		_game.setTime(aCurrentTic);
		aDuration = aCurrentTic - _previousTic;
		if ( aDuration < _ticDuration )
		{
			if ( not processMessageIfAvailable() )
			{
				// sleep a fraction of ticduration
				usleep( _ticDuration.total_milliseconds() / 10 );
			}
			else
			{
				aMessageHasBeenProcessed = true;
			}
		}
		else if (_forcedStop)
		{
			break;
		}
		else
		{
			feedAgentProxy();
			ProcessTimer aProcessTimer(_publisher, _game);
			aProcessTimer.execute();
			_previousTic = aCurrentTic;
		}
	}
}

void Server::loop()
{
	_mainLoopRunning = true;

	while (_mainLoopRunning)
	{
		loopUntilOneMessageIsProcessed();
	}
}

void Server::stop()
{
	ORWELL_LOG_INFO("Terminating server main loop");
	_forcedStop = true;
	_mainLoopRunning = false;
	_game.stop();
}

orwell::game::Game & Server::accessContext()
{
	return _game;
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

void Server::push(
		std::string const & iUrl,
		std::string const & iMessage)
{
	orwell::com::Sender aPusher(
			iUrl,
			ZMQ_PUSH,
			orwell::com::ConnectionMode::CONNECT,
			_zmqContext);
	aPusher.sendString(iMessage);
}

}

