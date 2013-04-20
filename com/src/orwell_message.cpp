/* Definition for a orwell message, nothing more than a wrapped Protobuf message */

#include <orwell_message.hpp>
#include <google/protobuf/message.h>
#include <zmq.hpp>
#include <version1.pb.h>

orwell::com::orwell_message::orwell_message(std::string const & _message_type) : _internal_message(0), _internally_modified(false)
{
	_proto_message = new orwell::com::base_message();
	_proto_message->set_message_type(_message_type);
}

orwell::com::orwell_message::orwell_message(std::string const & _message_type, 
		google::protobuf::Message const & _message) : _internal_message(0), _internally_modified(false)
{
	std::string serialized = _message.SerializeAsString();

	if (!serialized.empty()) {
		_proto_message = new orwell::com::base_message();
		_proto_message->set_message_type(_message_type);
		_proto_message->set_serialized_message(serialized);
	}

	else
		_proto_message = 0;
}

orwell::com::orwell_message::orwell_message(zmq::message_t & _zmq_message) : _internal_message(0), _internally_modified(false)
{
	construct_from_msg(_zmq_message);
}

orwell::com::orwell_message::orwell_message(zmq::socket_t & zmq_socket) : _internal_message(0), _internally_modified(false)
{
	zmq::message_t msg;
	if(zmq_socket.recv(&msg))
		construct_from_msg(msg);

	else 
		_proto_message = 0;
}

orwell::com::orwell_message::~orwell_message()
{
	delete _proto_message;
}

void orwell::com::orwell_message::construct_from_msg(zmq::message_t &_zmq_message)
{
	_proto_message = new orwell::com::base_message();
	_proto_message->ParseFromArray(
			reinterpret_cast<const char *>(_zmq_message.data()), 
			_zmq_message.size()); 
}

void orwell::com::orwell_message::commit()
{
	_proto_message->set_serialized_message(_internal_message->SerializeAsString());
}

void orwell::com::orwell_message::send_in_socket(zmq::socket_t & socket)
{
	// Update every modification
	if (_internally_modified)
		_proto_message->set_serialized_message(_internal_message->SerializeAsString());

	_internally_modified = false;
	std::size_t message_size(_proto_message->SerializeAsString().size());

	zmq::message_t msg(message_size);
	_proto_message->SerializeToArray(msg.data(), message_size);

	socket.send(msg);
}

std::string const & orwell::com::orwell_message::serialize_message()
{
	_serialized_in_string.clear();
	if (_proto_message != 0)
		_serialized_in_string = _proto_message->SerializePartialAsString();

	return _serialized_in_string;
}
