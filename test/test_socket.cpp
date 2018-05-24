#include <stdlib.h>
#include <thread>

#include <gtest/gtest.h>

#include <log4cxx/ndc.h>

#include "orwell/com/Socket.hpp"

#include "orwell/support/GlobalLogger.hpp"

int main()
{
	orwell::support::GlobalLogger::Create(
			"test_socket", "test_socket.log", true);
	log4cxx::NDC ndc("test_socket");
	ORWELL_LOG_INFO("Test starts\n");

	using orwell::com::Socket;
	EXPECT_EQ(std::string{"\\x15"}, Socket::Repr(std::string{"\x15"}));
	EXPECT_EQ(std::string{"\\x01"}, Socket::Repr(std::string{"\x01"}));
	EXPECT_EQ(std::string{"Toto"}, Socket::Repr(std::string{"Toto"}));
	orwell::support::GlobalLogger::Clear();
	return 0;
}
