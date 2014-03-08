#include "orwell/com/Sender.hpp"

//std
#include <iostream>
#include <unistd.h>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>

#include <memory>

using namespace log4cxx;
using std::string;
using std::cout;
using std::endl;

namespace orwell {
namespace com {

Sender::Sender(
		string const & iUrl,
		unsigned int const iSocketType,
		ConnectionMode::ConnectionMode const iConnectionMode,
		zmq::context_t & ioZmqContext,
		unsigned int const iSleep)
	: _zmqSocket(new zmq::socket_t(ioZmqContext, iSocketType))
	, _url(iUrl)
{
	//int aLinger = 10; // linger 0.01 second max after being closed
	int const aLinger = 10;
	_zmqSocket->setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));

	if (ConnectionMode::BIND == iConnectionMode)
	{
		_zmqSocket->bind(iUrl.c_str());
		ORWELL_LOG_INFO("Publisher binds on " << iUrl.c_str());
	}

	else
	{
		assert(ConnectionMode::CONNECT == iConnectionMode);
		_zmqSocket->connect(iUrl.c_str());
		ORWELL_LOG_INFO("Pusher connects to " << iUrl.c_str());
	}

	if (iSleep > 0)
	{
		sleep(iSleep);
	}
}

Sender::~Sender()
{
	delete(_zmqSocket);
}

void Sender::send( RawMessage const & iMessage )
{
	string aMessage;
	aMessage += iMessage._routingId;
	aMessage += " ";
	aMessage += iMessage._type;
	aMessage += " ";
	aMessage += iMessage._payload;

	zmq::message_t aZmqMessage( aMessage.size() );
	memcpy((void *) aZmqMessage.data(), aMessage.c_str(), aMessage.size());

	_zmqSocket->send( aZmqMessage );
	ORWELL_LOG_DEBUG("Sent " << aMessage.size() << " bytes : " << iMessage._type << "-" );

}

std::string const & Sender::getUrl() const
{
	return _url;
}

}} // end namespace

