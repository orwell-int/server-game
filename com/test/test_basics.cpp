// Test serialization and de-serialization of messages

#include <iostream>
#include <message.hpp>
#include <version1.pb.h>

int main()
{
	orwell::com::message rmsg("LOGIN");

	rmsg.accessInternalMessage<orwell::com::loginMessage>().set_client_id("c_id");
	rmsg.accessInternalMessage<orwell::com::loginMessage>().set_wished_robot_type("TANK");
	rmsg.commit();

	orwell::com::loginMessage const & mymsg(rmsg.getInternalMessage<orwell::com::loginMessage>());
	if (mymsg.client_id() != "c_id") return 1;
	if (mymsg.wished_robot_type() != "TANK") return 1;

	return 0;
}

