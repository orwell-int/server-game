#include "orwell/game/Robot.hpp"
#include "orwell/game/Team.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdint>

#include <log4cxx/ndc.h>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Item.hpp"
#include "orwell/game/Ruleset.hpp"
#include "orwell/game/Game.hpp"

#include "Common.hpp"


class TestOrwellGameItem : public ::testing::Test
{
protected:
	TestOrwellGameItem()
		: m_type("flag")
		, m_name("FLAG")
		, m_rfids{"RFID1", "RFID2", "RFID3"}
		, m_colourCode(-1)
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
	orwell::game::Ruleset m_ruleset;
};


TEST_F(TestOrwellGameItem, CreateMultiFlag)
{
	orwell::game::Item::CreateItem(
			m_type,
			m_name,
			m_rfids,
			m_colourCode,
			m_ruleset);
	EXPECT_EQ(1, orwell::game::Item::GetAllItems().size())
		<< "Only one flag created with multiple RFID codes.";
}


int main(int argc, char ** argv)
{
	orwell::support::GlobalLogger::Create("test_orwell_game_Item", "test_orwell_game_Item.log", true);
	log4cxx::NDC ndc("test_orwell_game_Item");
	ORWELL_LOG_INFO("Test starts\n");
	::testing::InitGoogleTest(&argc, argv);
	int aResult = RUN_ALL_TESTS();
	orwell::support::GlobalLogger::Clear();
	return aResult;
}
