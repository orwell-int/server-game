#include "orwell/game/Robot.hpp"
#include "orwell/game/Team.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdint>

#include <log4cxx/ndc.h>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Item.hpp"
#include "orwell/game/ItemEncoder.hpp"
#include "orwell/game/Ruleset.hpp"
#include "orwell/game/item/Flag.hpp"
#include "orwell/game/Game.hpp"
#include "server-game.pb.h"

#include "Common.hpp"


class TestOrwellGameItemEncoder : public ::testing::Test
{
protected:
	TestOrwellGameItemEncoder()
		: m_type("flag")
		, m_name("FLAG")
		, m_rfids{"RFID1"}
		, m_timeToCapture(5000)
		, m_colourCode(0)
		, m_pointsOnCapture(1)
		, m_flagRFID(
				m_name,
				m_rfids,
				m_timeToCapture,
				m_pointsOnCapture)
		, m_flagColour(
				m_name,
				m_colourCode,
				m_timeToCapture,
				m_pointsOnCapture)
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
	boost::posix_time::milliseconds m_timeToCapture;
	int32_t m_colourCode;
	orwell::game::Ruleset m_ruleset;
	uint32_t m_pointsOnCapture;
	orwell::game::item::Flag m_flagRFID;
	orwell::game::item::Flag m_flagColour;
};


TEST_F(TestOrwellGameItemEncoder, Create)
{
	EXPECT_EQ(m_name, m_flagRFID.getName());
	EXPECT_EQ(m_rfids, m_flagRFID.getRfids());
	EXPECT_EQ(-1, m_flagRFID.getColour());
	EXPECT_EQ(m_name, m_flagColour.getName());
	EXPECT_EQ(m_colourCode, m_flagColour.getColour());
	EXPECT_EQ(0, m_flagColour.getRfids().size());
	EXPECT_EQ("", m_flagRFID.getTeam());
	EXPECT_EQ("", m_flagColour.getTeam());

	std::unique_ptr< orwell::game::ItemEncoder > aRFIDEncoder = m_flagRFID.getEncoder();
	orwell::messages::Item lItem;
	aRFIDEncoder->encode(lItem);
	EXPECT_EQ(orwell::messages::FLAG, lItem.type());
	EXPECT_EQ(m_name, lItem.name());
	EXPECT_FALSE(lItem.has_capture_status());
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
