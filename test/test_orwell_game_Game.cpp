#include "orwell/game/Game.hpp"
#include "orwell/game/Item.hpp"
#include "orwell/game/Team.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Ruleset.hpp"
#include "orwell/Server.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional_io.hpp>

#include <set>
#include <iostream>
#include <unistd.h>
#include <cstdint>

#include <log4cxx/ndc.h>

//#include <gtest/gtest.hpp>

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"


class TestOrwellGameGame : public ::testing::Test
{
protected:
	TestOrwellGameGame()
		: m_gameDuration(60)
		, m_ticDuration(120)
		, m_agentUrl("tcp://localhost:9003")
		, m_pullUrl("tcp://localhost:9001")
		, m_publishUrl("tcp://localhost:9000")
		, m_game(
				m_fakeSystemProxy,
				boost::posix_time::time_duration(0, 0, m_gameDuration),
				m_ruleset,
				m_server)
	{
	}

	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
	}

	uint32_t const m_gameDuration;
	long const m_ticDuration;
	FakeSystemProxy m_fakeSystemProxy;
	FakeAgentProxy m_fakeAgentProxy;
	orwell::game::Ruleset m_ruleset;
	std::string const m_agentUrl;
	std::string const m_pullUrl;
	std::string const m_publishUrl;
	FakeServer m_server;
	orwell::game::Game m_game;
};


TEST_F(TestOrwellGameGame, Create)
{
	EXPECT_FALSE(m_game.getIsRunning())
		<< "The game is not running when created.";
	EXPECT_EQ(m_gameDuration, m_game.getSecondsLeft())
		<< "No seconds have been consummed yet.";
}

TEST_F(TestOrwellGameGame, AddAndRemoveContact)
{
	std::string const aRobotId("robot_id");
	std::string const aTeamName("team");
	m_ruleset.m_timeToCapture = boost::posix_time::milliseconds(1000);
	m_ruleset.m_pointsOnCapture = 1;
	m_ruleset.m_scoreToWin = 1;
	m_game.addTeam(aTeamName);
	m_game.addRobot("robot", aTeamName, 1, 2, aRobotId);
	std::shared_ptr< orwell::game::Robot > aRobot = m_game.getRobotWithoutRealRobot("temporary_id");
	aRobot->setVideoUrl("nc:12.34.56.78:90");
	aRobot->setHasRealRobot(true); // fake register
	std::string const aPlayerName("player");
	m_game.addPlayer(aPlayerName);
	m_game.getRobotForPlayer(aPlayerName);
	boost::posix_time::ptime aNow = boost::posix_time::microsec_clock::local_time();
	m_game.setTime(aNow);
	m_game.start();
	EXPECT_TRUE(m_game.getIsRunning()) << "The game is now running.";
	std::set< std::string > aSetItemRfid;
	aSetItemRfid.insert("RFID");
	std::shared_ptr< orwell::game::Item > aNewItem = orwell::game::Item::CreateItem(
			"flag",
			"demo flag",
			aSetItemRfid,
			-1,
			m_ruleset);
	EXPECT_TRUE(nullptr != aNewItem.get()) << "Item properly created.";
	m_game.robotIsInContactWith(aRobotId, aNewItem);
	m_game.step();
	m_game.robotDropsContactWith(aRobotId, aNewItem);
	m_game.step();
	ORWELL_LOG_DEBUG("m_game.getWinner() = " << m_game.getWinner());
	if (m_game.getWinner())
	{
		ORWELL_LOG_DEBUG("*m_game.getWinner() = " << *m_game.getWinner());
	}
	EXPECT_FALSE(m_game.getWinner()) << "Game has no winner yet.";
	m_game.robotIsInContactWith(aRobotId, aNewItem);
	auto const aDelta = boost::posix_time::seconds(2);
	m_game.setTime(aNow + aDelta);
	m_game.step();
	m_game.setTime(aNow + aDelta * 2);
	m_game.step();
	EXPECT_TRUE(m_game.getWinner()) << "Game has a winner.";
	EXPECT_EQ(aTeamName, *m_game.getWinner()) << "Game has the proper winner.";
}

int main(int argc, char ** argv)
{
	orwell::support::GlobalLogger::Create("test_orwell_game_Game", "test_orwell_game_Game.log", true);
	log4cxx::NDC ndc("test_orwell_game_Game");
	ORWELL_LOG_INFO("Test starts\n");
	::testing::InitGoogleTest(&argc, argv);
	int aResult = RUN_ALL_TESTS();
	orwell::support::GlobalLogger::Clear();
	return aResult;
}
