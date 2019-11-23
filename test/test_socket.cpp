#include <gtest/gtest.h>

#include "orwell/com/Socket.hpp"
#include "orwell/game/Game.hpp"

#include "Common.hpp"

TEST(Socket, Repr)
{
	using orwell::com::Socket;
	EXPECT_EQ(Socket::Repr(std::string{"\x15"}), std::string{"\\x15"});
	EXPECT_EQ(Socket::Repr(std::string{"\x01"}), std::string{"\\x01"});
	EXPECT_EQ(Socket::Repr(std::string{"Toto"}), std::string{"Toto"});
}

int main(int argc, char **argv)
{
	return RunTest(argc, argv, "test_socket");
}
