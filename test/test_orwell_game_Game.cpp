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
		, m_replierUrl("tcp://localhost:9002")
		, m_game(
				m_fakeSystemProxy,
				boost::posix_time::time_duration(0, 0, m_gameDuration),
				m_ruleset,
				m_server)
		, m_now(boost::posix_time::microsec_clock::local_time())
		, m_robotId("robot_id")
		, m_teamName("team")
		, m_twoSeconds(boost::posix_time::seconds(2))

	{
	}

	virtual void SetUp()
	{
		m_ruleset.m_timeToCapture = boost::posix_time::milliseconds(1000);
		m_ruleset.m_pointsOnCapture = 1;
		m_ruleset.m_scoreToWin = 1;
		m_game.addTeam(m_teamName);
		m_game.addRobot("robot", m_teamName, 1, 2, m_robotId);
		std::shared_ptr< orwell::game::Robot > aRobot =
			m_game.getRobotWithoutRealRobot("temporary_id");
		aRobot->setVideoUrl("nc:12.34.56.78:90");
		aRobot->setHasRealRobot(true); // fake register
		std::string const aPlayerName("player");
		m_game.addPlayer(aPlayerName);
		m_game.getRobotForPlayer(aPlayerName);
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
	std::string const m_replierUrl;
	FakeServer m_server;
	orwell::game::Game m_game;
	boost::posix_time::ptime m_now;
	std::string const m_robotId;
	std::string const m_teamName;
	boost::posix_time::seconds const m_twoSeconds;
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
	m_game.start(true);
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
	m_game.robotIsInContactWith(m_robotId, aNewItem);
	m_game.step();
	m_game.robotDropsContactWith(m_robotId, aNewItem);
	m_game.step();
	ORWELL_LOG_DEBUG("m_game.getWinner() = " << m_game.getWinner());
	if (m_game.getWinner())
	{
		ORWELL_LOG_DEBUG("*m_game.getWinner() = " << *m_game.getWinner());
	}
	EXPECT_FALSE(m_game.getWinner()) << "Game has no winner yet.";
	m_game.robotIsInContactWith(m_robotId, aNewItem);
	m_game.setTime(m_now + m_twoSeconds);
	m_game.step();
	m_game.setTime(m_now + m_twoSeconds * 2);
	m_game.step();
	EXPECT_TRUE(m_game.getWinner()) << "Game has a winner.";
	EXPECT_EQ(m_teamName, *m_game.getWinner()) << "Game has the proper winner.";
}

TEST_F(TestOrwellGameGame, RestartGame)
{
	// override the time because it is the only way to control it
	m_game.start(true, m_now);
	EXPECT_TRUE(m_game.getIsRunning()) << "The game is now running.";
	ORWELL_LOG_INFO("m_now = " << m_now);
	ORWELL_LOG_INFO("m_game.getTime() = " << m_game.getTime());
	EXPECT_EQ(m_gameDuration, m_game.getSecondsLeft())
		<< "No seconds have been consummed yet.";
	ORWELL_LOG_INFO("Add two seconds");
	m_game.setTime(m_now + m_twoSeconds);
	ORWELL_LOG_INFO("m_game.getTime() = " << m_game.getTime());
	EXPECT_GE(
		static_cast< uint64_t >(m_gameDuration) - m_twoSeconds.total_seconds(),
		m_game.getSecondsLeft())
		<< "Roughly " << m_twoSeconds << " later.";
	m_game.step();
	EXPECT_GE(
		static_cast< uint64_t >(m_gameDuration) - m_twoSeconds.total_seconds(),
		m_game.getSecondsLeft())
		<< "Roughly " << m_twoSeconds << " later.";
	m_game.start(true, m_now);
	EXPECT_EQ(m_gameDuration, m_game.getSecondsLeft())
		<< "We are back to the begining.";
}

TEST_F(TestOrwellGameGame, RestartGameAfterWin)
{
	// override the time because it is the only way to control it
	m_game.start(true, m_now);
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
	m_game.robotIsInContactWith(m_robotId, aNewItem);
	m_game.step();
	m_game.robotDropsContactWith(m_robotId, aNewItem);
	m_game.step();
	ORWELL_LOG_DEBUG("m_game.getWinner() = " << m_game.getWinner());
	if (m_game.getWinner())
	{
		ORWELL_LOG_DEBUG("*m_game.getWinner() = " << *m_game.getWinner());
	}
	EXPECT_FALSE(m_game.getWinner()) << "Game has no winner yet.";
	m_game.robotIsInContactWith(m_robotId, aNewItem);
	m_game.setTime(m_now + m_twoSeconds);
	m_game.step();
	m_game.setTime(m_now + m_twoSeconds * 2);
	m_game.step();
	EXPECT_TRUE(m_game.getWinner()) << "Game has a winner.";
	EXPECT_EQ(m_teamName, *m_game.getWinner()) << "Game has the proper winner.";
	m_game.start(true, m_now);
	EXPECT_EQ(m_gameDuration, m_game.getSecondsLeft())
		<< "We are back to the begining.";
	EXPECT_FALSE(m_game.getWinner()) << "Game has a no winner yet.";
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
