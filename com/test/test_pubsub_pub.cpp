

#include <iostream>
#include <zmq.hpp>
#include <string>
#include <unistd.h>

#include <zmq.hpp>
#include <iostream>
#include <sstream>

#include <unistd.h>

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

int main()
{
/*
 // Prepare our context and publisher
zmq::context_t context (1);
zmq::socket_t publisher (context, ZMQ_PUB);
publisher.bind("tcp://*:5556");
publisher.bind("ipc://weather.ipc");

// Initialize random number generator
srandom ((unsigned) time (NULL));

int zipcode, temperature, relhumidity;
usleep(0.3);
for (int i = 0 ; i< 3 ; ++i)
{


// Get values that will fool the boss
zipcode = 10001;
//within (100000);
temperature = within (215) - 80;
relhumidity = within (50) + 10;

cout << " " << zipcode << " " << temperature << " " << relhumidity << " " << endl;

// Send message to all subscribers
zmq::message_t message(20);
snprintf ((char *) message.data(), 20 ,
"%05d %d %d", zipcode, temperature, relhumidity);
publisher.send(message);

}
*/

return 0;




/*
    zmq::context_t context (1);
    zmq::socket_t publisher (context, ZMQ_PUB);
    publisher.bind("tcp://127.0.0.1:9000");

    // Initialize random number generator
    srandom ((unsigned) time (NULL));

    int aRand;

    // Get values that will fool the boss
    aRand = within (215) - 80;

cout << "sending random number " << aRand << endl;

    // Send message to all subscribers
    zmq::message_t message(20);
    snprintf ((char *) message.data(), 20 ,
    "%d", aRand);
    publisher.send(message);

    return 0;
    */
}

