#include "orwell/game/Robot.hpp"
#include "orwell/game/Team.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdint>

#include <log4cxx/ndc.h>

//#include <gtest/gtest.hpp>

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"


class TestOrwellGameRobot : public ::testing::Test
{
protected:
	TestOrwellGameRobot()
		: m_teamName("Team Name")
		, m_team(m_teamName)
		, m_robot(
			m_fakeSystemProxy,
			"Robot",
			"robot_id",
			m_team,
			42,
			43)
	{
	}

	FakeSystemProxy m_fakeSystemProxy;
	std::string const m_teamName;
	orwell::game::Team m_team;
	orwell::game::Robot m_robot;
};

TEST_F(TestOrwellGameRobot, Create)
{
	EXPECT_EQ(m_teamName, m_robot.getTeam().getName())
		<< "The team should be the same.";
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
