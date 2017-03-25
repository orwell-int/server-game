#include "orwell/game/Robot.hpp"
#include "orwell/game/Team.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdint>

#include <log4cxx/ndc.h>

//#include <gtest/gtest.hpp>

#include "orwell/game/Game.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"


class TestOrwellGameRobot : public ::testing::Test
{
protected:
	TestOrwellGameRobot()
		: m_teamName("Team Name")
		, m_robotName("Robot Name")
		, m_robotId("robot_id")
		, m_videoPort(42)
		, m_commandPort(43)
		, m_team(m_teamName)
		, m_robot(
			m_fakeSystemProxy,
			m_robotName,
			m_robotId,
			m_team,
			m_videoPort,
			m_commandPort)
	{
	}

	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
	}

	FakeSystemProxy m_fakeSystemProxy;
	std::string const m_teamName;
	std::string const m_robotName;
	std::string const m_robotId;
	uint16_t const m_videoPort;
	uint16_t const m_commandPort;
	orwell::game::Team m_team;
	orwell::game::Robot m_robot;
};


TEST_F(TestOrwellGameRobot, Create)
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


TEST_F(TestOrwellGameRobot, StartVideoWithoutURL)
{
	using ::testing::_;
	EXPECT_CALL(m_fakeSystemProxy, mkstemp(_)).Times(0);
	EXPECT_CALL(m_fakeSystemProxy, close(_)).Times(0);
	EXPECT_CALL(m_fakeSystemProxy, system(_)).Times(0);
	m_robot.startVideo();
}


TEST_F(TestOrwellGameRobot, StartVideoWithURL)
{
	using ::testing::_;
	EXPECT_CALL(m_fakeSystemProxy, mkstemp(_)).Times(0);
	EXPECT_CALL(m_fakeSystemProxy, close(_)).Times(0);
	EXPECT_CALL(m_fakeSystemProxy, system(_)).Times(0);
	m_robot.setVideoUrl("http://url.test:1234");
	m_robot.startVideo();
}


TEST_F(TestOrwellGameRobot, StartVideoWithURL_nc)
{
	using ::testing::_;
	EXPECT_CALL(m_fakeSystemProxy, mkstemp(_)).Times(0);
	EXPECT_CALL(m_fakeSystemProxy, system(_)).Times(0);
	m_robot.setVideoUrl("nc:12.34.56.78:90");
	m_robot.startVideo();
}


int main(int argc, char ** argv)
{
	orwell::support::GlobalLogger::Create("test_orwell_game_Robot", "test_orwell_game_Robot.log", true);
	log4cxx::NDC ndc("test_orwell_game_Robot");
	ORWELL_LOG_INFO("Test starts\n");
	::testing::InitGoogleTest(&argc, argv);
	int aResult = RUN_ALL_TESTS();
	orwell::support::GlobalLogger::Clear();
	return aResult;
}
