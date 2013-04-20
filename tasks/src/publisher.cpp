#include <publisher.hpp>
#include <zmq.hpp>
#include <orwell_message.hpp>
#include <string>

using namespace orwell::com;
using namespace orwell::tasks;
using std::string;

publisher::publisher() : 
	_zmq_context(new zmq::context_t(1)), 
	_zmq_socket(new zmq::socket_t(*_zmq_context, ZMQ_PUB))
{

}

publisher::publisher(string const & itag) : publisher()
{
	_tag = itag;
}

publisher::publisher(string const & itag, url const & iurl) : publisher(itag)
{
	_url = iurl;
}

publisher::publisher(std::string const & itag, std::string const & ihost, uint32_t iport) : publisher(itag)
{
	_url.set_host(ihost);
	_url.set_port(iport);
}

publisher::~publisher()
{
	_zmq_socket->close();
	delete _zmq_socket;
	delete _zmq_context;
}

void publisher::bind()
{
	_zmq_socket->bind(_url.get_url().c_str());
}

void publisher::publish(orwell_message & iorwell_message)
{
	string msg = _tag;
	msg += " ";
	msg += iorwell_message.serialize_message();

	zmq::message_t zmq_msg(msg.size());
	memcpy((void *) zmq_msg.data(), msg.c_str(), msg.size());

	_zmq_socket->send(zmq_msg);
}

void publisher::set_tag(string const & itag)
{
	_tag = itag;
}

string const & publisher::get_tag() const
{
	return _tag;
}

string const & publisher::get_url() const
{
	return _url.get_url();
}

