#include "orwell/com/Socket.hpp"

//std
#include <iostream>
#include <unistd.h>

#include <zmq.hpp>

#include <sstream>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"

using std::string;

namespace orwell
{
namespace com
{

Socket::Socket(
		std::string const & iUrl,
		unsigned int const iSocketType,
		ConnectionMode const iConnectionMode,
		zmq::context_t & ioZmqContext,
		unsigned int const iSleep)
	: m_zmqContext(ioZmqContext)
	, m_zmqSocket(nullptr)
	, m_url(iUrl)
	, m_socketType(iSocketType)
	, m_connectionMode(iConnectionMode)
{
	innerReset();
	if (iSleep > 0)
	{
		sleep(iSleep);
	}
}

Socket::~Socket()
{
	delete(m_zmqSocket);
}

void Socket::reset()
{
	ORWELL_LOG_INFO("reset the socket.");
}

bool Socket::receiveString(
		std::string & oMessage,
		bool const iBlocking)
{
	zmq::message_t aZmqMessage;

	int aFlags = 0;
	if (not iBlocking)
	{
		aFlags = ZMQ_NOBLOCK;
	}
	bool const aReceived = m_zmqSocket->recv(&aZmqMessage, aFlags);
	if (aReceived)
	{
		ORWELL_LOG_TRACE("message received");
		oMessage = string(static_cast<char*>(aZmqMessage.data()), aZmqMessage.size());
	}
	return aReceived;
}

bool Socket::receive(RawMessage & oMessage)
{
	string aType;
	string aPayload;
	string aDest;

	std::string aMessageData;
	bool const aReceived = receiveString(aMessageData);
	if (aReceived)
	{
		size_t aEndDestFlag = aMessageData.find(" ", 0);
		if (string::npos != aEndDestFlag)
		{
			aDest = aMessageData.substr(0, aEndDestFlag);

			size_t aEndTypeFlag = aMessageData.find(" ", aEndDestFlag + 1);
			if (aEndTypeFlag != string::npos)
			{
				aType = aMessageData.substr(aEndDestFlag + 1, aEndTypeFlag - aEndDestFlag - 1);
				aPayload = aMessageData.substr(aEndTypeFlag + 1);
			}
		}

		oMessage._type = aType;
		oMessage._routingId = aDest;
		oMessage._payload = aPayload;
		ORWELL_LOG_DEBUG("Received message : type=" << aType << "- dest=" << aDest << "-");
	}
	return aReceived;
}

void Socket::sendString(std::string const & iMessage) const
{
	zmq::message_t aZmqMessage(iMessage.size());
	memcpy((void *) aZmqMessage.data(), iMessage.c_str(), iMessage.size());

	try
	{
		m_zmqSocket->send(aZmqMessage);
		ORWELL_LOG_TRACE("Sent " << iMessage);
	}
	catch (...)
	{
		ORWELL_LOG_TRACE("Failed to send zmq message.");
	}
}

void Socket::send(RawMessage const & iMessage) const
{
	string aMessage;
	aMessage += iMessage._routingId;
	aMessage += " ";
	aMessage += iMessage._type;
	aMessage += " ";
	aMessage += iMessage._payload;

	sendString(aMessage);
	ORWELL_LOG_DEBUG("Sent message of type " << iMessage._type << " to " << iMessage._routingId << " with size " << aMessage.size());
}

std::string const & Socket::getUrl() const
{
	return m_url;
}

void Socket::innerReset()
{
	if (0 != m_zmqSocket)
	{
		delete(m_zmqSocket);
	}
	m_zmqSocket = new zmq::socket_t(m_zmqContext, m_socketType);
	int aLinger = 10; // linger 0.01 second max after being closed
	m_zmqSocket->setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));

	if (ZMQ_SUB == m_socketType)
	{
		string atag;
		m_zmqSocket->setsockopt(ZMQ_SUBSCRIBE, atag.c_str(), atag.size());
	}
	if (ConnectionMode::BIND == m_connectionMode)
	{
		m_zmqSocket->bind(m_url.c_str());
		ORWELL_LOG_INFO("Socket " << m_socketType << " binds on " << m_url.c_str());
	}
	else
	{
		assert(ConnectionMode::CONNECT == m_connectionMode);
		m_zmqSocket->connect(m_url.c_str());
		ORWELL_LOG_INFO("Socket " << m_socketType << " connects to '" << m_url.c_str() << "'");
	}
}

}
}

