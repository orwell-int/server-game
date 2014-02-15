#include <Sender.hpp>

//std
#include <iostream>
#include <unistd.h>

//com
#include <RawMessage.hpp>

#include <zmq.hpp>

using namespace log4cxx;
using std::string;
using std::cout;
using std::endl;

namespace orwell{
namespace com{

Sender::Sender(
		string const & iUrl,
		unsigned int const iSocketType,
		ConnectionMode::ConnectionMode const iConnectionMode,
		unsigned int const iSleep) :
	_zmqContext(new zmq::context_t(1)),
	_zmqSocket(new zmq::socket_t(*_zmqContext, iSocketType)),
	_logger(log4cxx::Logger::getLogger("orwell.log")),
	_url(iUrl)
{
	int aLinger = 10; // linger 0.01 second max after being closed
	_zmqSocket->setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));

	if (ConnectionMode::BIND == iConnectionMode)
	{
		_zmqSocket->bind(iUrl.c_str());
		LOG4CXX_INFO(_logger, "Publisher binds on " << iUrl.c_str());
	}

	else
	{
		assert(ConnectionMode::CONNECT == iConnectionMode);
		_zmqSocket->connect(iUrl.c_str());
		LOG4CXX_INFO(_logger, "Pusher connects to " << iUrl.c_str());
	}

	if (iSleep > 0)
	{
		sleep(iSleep);
	}
}

Sender::~Sender()
{
	_zmqSocket->close();
	delete _zmqSocket;
	delete _zmqContext;
}

void Sender::send( RawMessage const & iMessage )
{
	log4cxx::LoggerPtr aLogger(log4cxx::Logger::getLogger("orwell.log"));

	string aMessage;
	aMessage += iMessage._routingId;
	aMessage += " ";
	aMessage += iMessage._type;
	aMessage += " ";
	aMessage += iMessage._payload;

	zmq::message_t aZmqMessage( aMessage.size() );
	memcpy((void *) aZmqMessage.data(), aMessage.c_str(), aMessage.size());

	_zmqSocket->send( aZmqMessage );
	LOG4CXX_DEBUG(aLogger, "Sent " << aMessage.size() << " bytes : " << iMessage._type << "-" );

}
	
std::string const & Sender::getUrl() const
{
	return _url;
}

}} // end namespace

