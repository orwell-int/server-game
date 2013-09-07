


#include <iostream>
#include <zmq.hpp>
#include <string>
#include <unistd.h>

#include <sstream>



#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

int main()
{
/*
zmq::context_t context (1);

// Socket to talk to server
std::cout << "Collecting updates from weather server…\n" << std::endl;
zmq::socket_t subscriber (context, ZMQ_SUB);
subscriber.connect("tcp://localhost:5556");

// Subscribe to zipcode, default is NYC, 10001
const char *filter = "10001 ";
subscriber.setsockopt(ZMQ_SUBSCRIBE, filter, strlen (filter));

// Process 100 updates
int update_nbr;
long total_temp = 0;
for (update_nbr = 0; update_nbr < 5; update_nbr++) {

zmq::message_t update;
int zipcode, temperature, relhumidity;


subscriber.recv(&update);

std::istringstream iss(static_cast<char*>(update.data()));
iss >> zipcode >> temperature >> relhumidity ;
std::cout << "receiving '"<< temperature << "  " << relhumidity << endl;

total_temp += temperature;
}
std::cout << "Average temperature for zipcode '"<< filter
<<"' was "<<(int) (total_temp / update_nbr) <<"F"
<< std::endl;
return 0;

*/

/*
    zmq::context_t context (1);

    zmq::socket_t subscriber (context, ZMQ_SUB);
    subscriber.connect("tcp://127.0.0.1:9000");

    const char *filter = "";
    subscriber.setsockopt(ZMQ_SUBSCRIBE, filter, strlen (filter));


    while(1){
        zmq::message_t update;
        int aContent;

        subscriber.recv(&update);
        std::cout << "Received ";
        std::istringstream iss(static_cast<char*>(update.data()));
        iss >> aContent ;

        std::cout << "Received number "<< aContent;


    }
    */
    return 0;
}

