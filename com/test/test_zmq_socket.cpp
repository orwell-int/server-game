// Test serialization and de-serialization of messages and sending sockets

#include <iostream>
#include <zmq.hpp>
#include <string>
#include <unistd.h>

const std::string _client_id("Client ID");
const std::string _wished_robo_type("TANK");

void client()
{
    /*
std::cout << "Client starting" << std::endl;
zmq::context_t zmq_context(1);
zmq::socket_t zmq_socket(zmq_context, ZMQ_REQ);

std::cout << "Client connecting to server" << std::endl;
zmq_socket.connect("tcp://localhost:9000");

orwell::com::orwell_message rmsg("LOGIN");
orwell::com::login_message & rlogin = rmsg.access_internal_message<orwell::com::login_message>();
rlogin.set_client_id(_client_id);
rlogin.set_wished_robo_type(_wished_robo_type);
rmsg.commit(); // This shouldn't be necessary

std::cout << "Client sending message" << std::endl;
rmsg.send_in_socket(zmq_socket);

zmq_socket.close();
*/
}

bool server()
{
    /*
std::cout << "Server starting" << std::endl;
zmq::context_t zmq_context(1);
zmq::socket_t zmq_socket(zmq_context, ZMQ_REP);

std::cout << "Server binding to tcp://localhost:9000" << std::endl;
zmq_socket.bind("tcp://*:9000");

std::cout << "Server receiving message" << std::endl;
orwell::com::orwell_message rmsg(zmq_socket);
zmq_socket.close();

orwell::com::login_message const & rlogin = rmsg.get_internal_message<orwell::com::login_message>();
std::cout << "Received: " << rlogin.client_id() << std::endl;
std::cout << "Received: " << rlogin.wished_robo_type() << std::endl;

return ((rlogin.client_id() == _client_id) &&
(rlogin.wished_robo_type() == _wished_robo_type));
*/
return true;
}

int main()
{
	switch (fork())
	{
		case 0: // child
			client();
			return 0; // child can stop here
		default: // father
			if (server())
			{
				return 0;
			}
			else
			{
				return -1;
			}
	}

	return -1; // Should never arrive here..
}

