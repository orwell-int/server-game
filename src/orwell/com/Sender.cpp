#include "orwell/com/Sender.hpp"

//std
#include <iostream>
#include <unistd.h>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>

#include <memory>

using std::string;

namespace orwell {
namespace com {

Sender::Sender(
		string const & iUrl,
		unsigned int const iSocketType,
		ConnectionMode const iConnectionMode,
		zmq::context_t & ioZmqContext,
		unsigned int const iSleep)
	: m_zmqSocket(new zmq::socket_t(ioZmqContext, iSocketType))
	, m_url(iUrl)
{
	//int aLinger = 10; // linger 0.01 second max after being closed
	int const aLinger = 10;
	m_zmqSocket->setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));

	if (ConnectionMode::BIND == iConnectionMode)
	{
		m_zmqSocket->bind(iUrl.c_str());
		ORWELL_LOG_INFO("Publisher binds on " << iUrl.c_str());
	}

	else
	{
		assert(ConnectionMode::CONNECT == iConnectionMode);
		m_zmqSocket->connect(iUrl.c_str());
		ORWELL_LOG_INFO("Pusher connects to " << iUrl.c_str());
	}

	if (iSleep > 0)
	{
		sleep(iSleep);
	}
}

Sender::Sender(Sender const & iOther)
{
}

Sender::~Sender()
{
	delete(m_zmqSocket);
}

void Sender::sendString(std::string const & iMessage)
{
	zmq::message_t aZmqMessage(iMessage.size());
	memcpy((void *) aZmqMessage.data(), iMessage.c_str(), iMessage.size());

	m_zmqSocket->send(aZmqMessage);
	ORWELL_LOG_DEBUG("Sent " << iMessage);
}

void Sender::send( RawMessage const & iMessage )
{
	string aMessage;
	aMessage += iMessage._routingId;
	aMessage += " ";
	aMessage += iMessage._type;
	aMessage += " ";
	aMessage += iMessage._payload;

	sendString(aMessage);
	ORWELL_LOG_DEBUG("Sent " << aMessage.size() << " bytes : " << iMessage._type << "-");
}

std::string const & Sender::getUrl() const
{
	return m_url;
}

}
} // end namespace

