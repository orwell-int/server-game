#include <Receiver.hpp>

//std
#include <iostream>
#include <unistd.h>

#include <zmq.hpp>

#include <sstream>

using namespace log4cxx;
using namespace log4cxx::helpers;
using std::string;
using std::cout;
using std::endl;

namespace orwell{
namespace com{

Receiver::Receiver(string const & iUrl, unsigned int const iSocketType ) :
   	_zmqContext(new zmq::context_t(1)),
   	_zmqSocket(new zmq::socket_t(*_zmqContext, iSocketType)),
   	_logger(log4cxx::Logger::getLogger("orwell.log"))
{
    int aLinger = 1000; // linger 1 second max after being closed
    _zmqSocket->setsockopt(ZMQ_LINGER, &aLinger, sizeof(aLinger));

    if ( iSocketType == ZMQ_SUB)
    {
        string atag = "";
        _zmqSocket->setsockopt(ZMQ_SUBSCRIBE, atag.c_str(), atag.size() );
       	_zmqSocket->connect(iUrl.c_str());
       	sleep( 1 );
        LOG4CXX_INFO(_logger, "Subscriber connects on " << iUrl.c_str() << " - it subscribes to everything" );
    }
    else
    {
        _zmqSocket->bind(iUrl.c_str());
        sleep( 1 );
        LOG4CXX_INFO(_logger, "Puller binds on " << iUrl.c_str() );
    }

}

Receiver::~Receiver()
{
    _zmqSocket->close();
	delete _zmqSocket;
	delete _zmqContext;
}

RawMessage Receiver::receive()
{
    log4cxx::LoggerPtr  aLogger(log4cxx::Logger::getLogger("orwell.log"));

    zmq::message_t aZmqMessage;
    string aType;
    string aPayload;
    string aDest;

    LOG4CXX_DEBUG(aLogger, "now receiving" );

	if ( _zmqSocket->recv(&aZmqMessage) )
	{
	    string aMessageData = string(static_cast<char*>(aZmqMessage.data()), aZmqMessage.size());
	//    string aMessageData = reinterpret_cast< char *>( aZmqMessage.data() );
	    size_t aEndDestFlag = aMessageData.find( " ", 0 );
	    if ( aEndDestFlag != string::npos )
	    {
            aDest = aMessageData.substr(0, aEndDestFlag );

            size_t aEndTypeFlag = aMessageData.find( " ", aEndDestFlag + 1 );
            if ( aEndTypeFlag != string::npos )
            {
                aType = aMessageData.substr( aEndDestFlag + 1, aEndTypeFlag - aEndDestFlag - 1 );
                aPayload = aMessageData.substr( aEndTypeFlag + 1 );
            }
	    }
	}

    LOG4CXX_DEBUG(aLogger, "Received "<< aZmqMessage.size() << " bytes : type=" << aType << "- dest=" << aDest << "-");
//    LOG4CXX_DEBUG(aLogger, "Payload : " << aPayload << "-");

    RawMessage aEnvelopeMessage( aDest, aType, aPayload );
    return aEnvelopeMessage;
}

}}
