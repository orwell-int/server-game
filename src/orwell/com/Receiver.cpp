#include "orwell/com/Receiver.hpp"

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

Receiver::Receiver(
		std::string const & iUrl,
		unsigned int const iSocketType,
		ConnectionMode const iConnectionMode,
		zmq::context_t & ioZmqContext,
		unsigned int const iSleep)
	: m_zmqSocket(new zmq::socket_t(ioZmqContext, iSocketType))
	, m_url(iUrl)
{
	int aLinger = 10; // linger 0.01 second max after being closed
	m_zmqSocket->setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));

	if (ZMQ_SUB == iSocketType)
	{
		string atag;
		m_zmqSocket->setsockopt(ZMQ_SUBSCRIBE, atag.c_str(), atag.size());
	}
	if (ConnectionMode::BIND == iConnectionMode)
	{
		m_zmqSocket->bind(iUrl.c_str());
		ORWELL_LOG_INFO("Puller binds on " << iUrl.c_str());
	}
	else
	{
		assert(ConnectionMode::CONNECT == iConnectionMode);
		m_zmqSocket->connect(iUrl.c_str());
		ORWELL_LOG_INFO("Subscriber connects to " << iUrl.c_str() << " - it subscribes to everything");
	}
	if (iSleep > 0)
	{
		sleep(iSleep);
	}
}

Receiver::Receiver(Receiver const & iOther)
{
}

Receiver::~Receiver()
{
	delete(m_zmqSocket);
}

bool Receiver::receiveString(
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

bool Receiver::receive(RawMessage & oMessage)
{
	string aType;
	string aPayload;
	string aDest;

	std::string aMessageData;
	bool const aReceived = receiveString(aMessageData);
	if (aReceived)
	{
		size_t aEndDestFlag = aMessageData.find( " ", 0 );
		if (string::npos != aEndDestFlag)
		{
			aDest = aMessageData.substr(0, aEndDestFlag);

			size_t aEndTypeFlag = aMessageData.find(" ", aEndDestFlag + 1);
			if ( aEndTypeFlag != string::npos )
			{
				aType = aMessageData.substr( aEndDestFlag + 1, aEndTypeFlag - aEndDestFlag - 1 );
				aPayload = aMessageData.substr( aEndTypeFlag + 1 );
			}
		}

		oMessage._type = aType;
		oMessage._routingId = aDest;
		oMessage._payload = aPayload;
		ORWELL_LOG_DEBUG("Received message : type=" << aType << "- dest=" << aDest << "-");
	}
	return aReceived;
}

std::string const & Receiver::getUrl() const
{
	return m_url;
}

}
}

