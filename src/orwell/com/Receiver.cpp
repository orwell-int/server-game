#include "orwell/com/Receiver.hpp"

//std
#include <iostream>
#include <unistd.h>

#include <zmq.hpp>

#include <sstream>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/RawMessage.hpp"

using std::string;
using std::cout;
using std::endl;

namespace orwell {
namespace com {

Receiver::Receiver(
		std::string const & iUrl,
		unsigned int const iSocketType,
		ConnectionMode::ConnectionMode const iConnectionMode,
		zmq::context_t & ioZmqContext,
		unsigned int const iSleep)
	: _zmqSocket(new zmq::socket_t(ioZmqContext, iSocketType))
	, _url(iUrl)
{
	int aLinger = 10; // linger 0.01 second max after being closed
	_zmqSocket->setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));

	if (ZMQ_SUB == iSocketType)
	{
		string atag;
		_zmqSocket->setsockopt(ZMQ_SUBSCRIBE, atag.c_str(), atag.size());
	}
	if (ConnectionMode::BIND == iConnectionMode)
	{
		_zmqSocket->bind(iUrl.c_str());
		ORWELL_LOG_INFO("Puller binds on " << iUrl.c_str());
	}
	else
	{
		assert(ConnectionMode::CONNECT == iConnectionMode);
		_zmqSocket->connect(iUrl.c_str());
		ORWELL_LOG_INFO("Subscriber connects to " << iUrl.c_str() << " - it subscribes to everything");
	}
	if (iSleep > 0)
	{
		sleep(iSleep);
	}
}

Receiver::~Receiver()
{
	delete(_zmqSocket);
}

bool Receiver::receive(RawMessage & oMessage)
{
	zmq::message_t aZmqMessage;
	string aType;
	string aPayload;
	string aDest;

	bool aReceived = _zmqSocket->recv(&aZmqMessage, ZMQ_NOBLOCK);
	if ( aReceived )
	{
		string aMessageData = string(static_cast<char*>(aZmqMessage.data()), aZmqMessage.size());
		size_t aEndDestFlag = aMessageData.find( " ", 0 );
		if (string::npos != aEndDestFlag)
		{
			aDest = aMessageData.substr(0, aEndDestFlag);

			size_t aEndTypeFlag = aMessageData.find(" ", aEndDestFlag + 1 );
			if ( aEndTypeFlag != string::npos )
			{
				aType = aMessageData.substr( aEndDestFlag + 1, aEndTypeFlag - aEndDestFlag - 1 );
				aPayload = aMessageData.substr( aEndTypeFlag + 1 );
			}
		}

		oMessage._type = aType;
		oMessage._routingId = aDest;
		oMessage._payload = aPayload;
		ORWELL_LOG_DEBUG("Received "<< aZmqMessage.size() << " bytes : type=" << aType << "- dest=" << aDest << "-");
	}
	return aReceived;
}

std::string const & Receiver::getUrl() const
{
	return _url;
}

}}

