#include "Receiver.hpp"

//std
#include <iostream>
#include <unistd.h>

#include <zmq.hpp>

#include <sstream>

#include "RawMessage.hpp"

using namespace log4cxx;
using namespace log4cxx::helpers;
using std::string;
using std::cout;
using std::endl;

namespace orwell{
namespace com{

Receiver::Receiver(
		std::string const & iUrl,
		unsigned int const iSocketType,
		ConnectionMode::ConnectionMode const iConnectionMode,
		unsigned int const iSleep) :
	_zmqContext(new zmq::context_t(1)),
	_zmqSocket(new zmq::socket_t(*_zmqContext, iSocketType)),
	_logger(log4cxx::Logger::getLogger("orwell.log"))
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
		LOG4CXX_INFO(_logger, "Puller binds on " << iUrl.c_str());
	}
	else
	{
		assert(ConnectionMode::CONNECT == iConnectionMode);
		_zmqSocket->connect(iUrl.c_str());
		LOG4CXX_INFO(_logger, "Subscriber connects to " << iUrl.c_str() << " - it subscribes to everything");
	}
	if (iSleep > 0)
	{
		sleep(iSleep);
	}
}

Receiver::~Receiver()
{
	_zmqSocket->close();
	delete _zmqSocket;
	delete _zmqContext;
}

bool Receiver::receive(RawMessage & oMessage)
{
	log4cxx::LoggerPtr aLogger(log4cxx::Logger::getLogger("orwell.log"));

	zmq::message_t aZmqMessage;
	string aType;
	string aPayload;
	string aDest;

    bool aReceived = _zmqSocket->recv(&aZmqMessage, ZMQ_NOBLOCK);
	if ( aReceived )
	{
		string aMessageData = string(static_cast<char*>(aZmqMessage.data()), aZmqMessage.size());
		//    string aMessageData = reinterpret_cast< char *>( aZmqMessage.data() );
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
        LOG4CXX_DEBUG(aLogger, "Received "<< aZmqMessage.size() << " bytes : type=" << aType << "- dest=" << aDest << "-");
        LOG4CXX_DEBUG(aLogger, "batman Received "<< aMessageData);
        LOG4CXX_DEBUG(aLogger, "batman Received payload "<< aPayload);
	}
    return aReceived;
}

}}

