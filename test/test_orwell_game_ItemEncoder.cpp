#include "orwell/game/Robot.hpp"
#include "orwell/game/Team.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdint>

#include <log4cxx/ndc.h>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Item.hpp"
#include "orwell/game/Flag.hpp"

#include "Common.hpp"


class TestOrwellGameItemEncoder : public ::testing::Test
{
protected:
	TestOrwellGameItemEncoder()
		: m_type("flag")
		, m_name("FLAG")
		, m_rfids{"RFID1"}
		, m_colourCode(0)
		, m_flag(
				m_type,
				m_name,
				m_rfids,
				m_colourCode)
	{
	}

	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
	}

	std::string m_type;
	std::string m_name;
	std::set< std::string > m_rfids;
	int32_t m_colourCode;
	orwell::game::Ruleset m_ruleSet;
	orwell::game::Flag m_flag;
};


TEST_F(TestOrwellGameItemEncoder, Create)
{
	EXPECT_EQ(m_teamName, m_robot.getTeam().getName())
		<< "The team should be the same.";
	EXPECT_EQ(m_robotName, m_robot.getName())
		<< "The robot name should be the same.";
	EXPECT_EQ(m_robotId, m_robot.getRobotId())
		<< "The robot id should be the same.";
	EXPECT_EQ(m_videoPort, m_robot.getVideoRetransmissionPort())
		<< "The video port should be the same.";
	EXPECT_EQ(m_commandPort, m_robot.getServerCommandPort())
		<< "The command port should be the same.";
	EXPECT_FALSE(m_robot.getIsAvailable())
		<< "A robot should start as NOT available.";
	EXPECT_FALSE(m_robot.getHasPlayer())
		<< "A robot should start without a player.";
	EXPECT_EQ("", m_robot.getVideoUrl())
		<< "A robot should start without a video URL.";
}


TEST_F(TestOrwellGameItemEncoder, StartVideoWithoutURL)
{
	using ::testing::_;
	EXPECT_CALL(m_fakeSystemProxy, mkstemp(_)).Times(0);
	EXPECT_CALL(m_fakeSystemProxy, close(_)).Times(0);
	EXPECT_CALL(m_fakeSystemProxy, system(_)).Times(0);
	m_robot.startVideo();
}


TEST_F(TestOrwellGameItemEncoder, StartVideoWithURL)
{
	using ::testing::_;
	EXPECT_CALL(m_fakeSystemProxy, mkstemp(_)).Times(1);
	EXPECT_CALL(m_fakeSystemProxy, close(_)).Times(1);
	EXPECT_CALL(m_fakeSystemProxy, system(_)).Times(1);
	m_robot.setVideoUrl("http://url.test:1234");
	m_robot.startVideo();
}


TEST_F(TestOrwellGameItemEncoder, StartVideoWithURL_nc)
{
	using ::testing::_;
	m_robot.setVideoUrl("nc:12.34.56.78:90");
	m_robot.startVideo();
	EXPECT_CALL(m_fakeSystemProxy, mkstemp(_)).Times(0);
	EXPECT_CALL(m_fakeSystemProxy, system(_)).Times(0);
}


int main(int argc, char ** argv)
{
	orwell::support::GlobalLogger::Create("test_orwell_game_ItemEncoder", "test_orwell_game_ItemEncoder.log", true);
	log4cxx::NDC ndc("test_orwell_game_ItemEncoder");
	ORWELL_LOG_INFO("Test starts\n");
	::testing::InitGoogleTest(&argc, argv);
	int aResult = RUN_ALL_TESTS();
	orwell::support::GlobalLogger::Clear();
	return aResult;
}
