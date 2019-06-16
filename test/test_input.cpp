#include <stdlib.h>
#include <thread>

#include <gtest/gtest.h>

#include <log4cxx/ndc.h>

#include "controller.pb.h"

#include "orwell/com/RawMessage.hpp"
#include "orwell/com/Socket.hpp"

#include "orwell/support/GlobalLogger.hpp"

template <typename MessageType>
static MessageType BuildProtobuf(
		orwell::com::RawMessage const & iMessage)
{
	MessageType aMessage;
	bool aSuccess = aMessage.ParsePartialFromString(iMessage._payload);
	ORWELL_LOG_DEBUG("BuildProtobuf success : " << aSuccess);
	ORWELL_LOG_DEBUG("Resulting protobuf : size = " << aMessage.ByteSize());
	return aMessage;
}

int main()
{
	orwell::support::GlobalLogger::Create("test_input", "test_input.log", true);
	log4cxx::NDC ndc("test_input");
	ORWELL_LOG_INFO("Test starts\n");

	orwell::messages::Input aInput;
	aInput.mutable_fire()->set_weapon1(true);
	aInput.mutable_fire()->set_weapon2(false);
	aInput.mutable_move()->set_left(0);
	aInput.mutable_move()->set_right(0);

	std::string const aPayload = aInput.SerializeAsString();
	ORWELL_LOG_INFO("payload = " << orwell::com::Socket::Repr(aPayload));

	char aChars1 [] = "\n\x12\t\x00\x00\x00\x00\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x00\x12\x04\x08\x01\x10\x00";
	std::string aStr1 = {aChars1, sizeof(aChars1) / sizeof(aChars1[0])};
	EXPECT_EQ(
			aStr1,
			aPayload);
	orwell::com::RawMessage aRawMessage("robot_one", "Input", aPayload);

	orwell::messages::Input aInput2 = BuildProtobuf< orwell::messages::Input >(aRawMessage);

	EXPECT_EQ(aInput.fire().weapon1(), aInput2.fire().weapon1());
	EXPECT_EQ(aInput.fire().weapon2(), aInput2.fire().weapon2());
	EXPECT_EQ(aInput.move().left(), aInput2.move().left());
	EXPECT_EQ(aInput.move().right(), aInput2.move().right());

	char aChars3 [] = "\x0b\t\x00\x00\x00\x00\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x13\x08\x01\x10\x00\x14";
	std::string aStr3 {aChars3, sizeof(aChars1) / sizeof(aChars1[0])};
	orwell::com::RawMessage aRawMessageClient(
			"robot_one",
			"Input",
			aStr3);
			//"\x0b\t\x00\x00\x00\x00\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x13\x08\x01\x10\x00\x14");
			//"\x0b\x09\x00\x00\x00\x00\x00\x00\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x13\x08\x01\x10\x00\x14");
	orwell::messages::Input aInput3 = BuildProtobuf< orwell::messages::Input >(aRawMessageClient);
	ORWELL_LOG_INFO("1: " << orwell::com::Socket::Repr(aStr1));
	ORWELL_LOG_INFO("3: " << orwell::com::Socket::Repr(aStr3));
	EXPECT_EQ(aInput.fire().weapon1(), aInput3.fire().weapon1());
	EXPECT_EQ(aInput.fire().weapon2(), aInput3.fire().weapon2());
	EXPECT_EQ(aInput.move().left(), aInput3.move().left());
	EXPECT_EQ(aInput.move().right(), aInput3.move().right());
	EXPECT_EQ(std::string{"Toto"}, std::string{"Toto"});
	orwell::support::GlobalLogger::Clear();
	return 0;
}

