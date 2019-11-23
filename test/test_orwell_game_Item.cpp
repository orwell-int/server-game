#include "orwell/game/Robot.hpp"
#include "orwell/game/Team.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdint>

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
		, m_rfidsNone{}
		, m_colourCodeNone(-1)
		, m_colourCode(3)
	{
	}

	virtual void SetUp()
	{
		orwell::game::Item::GetAllItems().clear();
	}

	virtual void TearDown()
	{
		orwell::game::Item::GetAllItems().clear();
	}

	std::string m_type;
	std::string m_name;
	std::set< std::string > m_rfids;
	std::set< std::string > m_rfidsNone;
	int32_t m_colourCodeNone;
	int32_t m_colourCode;
	orwell::game::Ruleset m_ruleset;
};


TEST_F(TestOrwellGameItem, CreateMultiFlag)
{
	orwell::game::Item::CreateItem(
			m_type,
			m_name,
			m_rfids,
			m_colourCodeNone,
			m_ruleset);
	EXPECT_EQ(size_t{1}, orwell::game::Item::GetAllItems().size())
		<< "Only one flag created with multiple RFID codes.";
}


TEST_F(TestOrwellGameItem, Capture)
{
	auto aItem = orwell::game::Item::CreateItem(
			m_type,
			m_name,
			m_rfidsNone,
			m_colourCode,
			m_ruleset);
	orwell::game::Team aTeam("team");
	ORWELL_LOG_INFO("capture state: " << static_cast< int >(aItem->getCaptureState()));
	EXPECT_EQ(orwell::game::CaptureState::PENDING, aItem->getCaptureState());
	aItem->startCapture(aTeam.getName());
	EXPECT_EQ(orwell::game::CaptureState::STARTED, aItem->getCaptureState());
	aItem->abortCapture();
	EXPECT_EQ(orwell::game::CaptureState::FAILED, aItem->getCaptureState());
	aItem->startCapture(aTeam.getName());
	EXPECT_EQ(orwell::game::CaptureState::STARTED, aItem->getCaptureState());
	aItem->capture(aTeam);
	EXPECT_EQ(orwell::game::CaptureState::SUCCEEDED, aItem->getCaptureState());
	EXPECT_EQ(aTeam.getName(), aItem->getOwningTeam());
	aItem->startCapture(aTeam.getName());
	EXPECT_EQ(orwell::game::CaptureState::SUCCEEDED, aItem->getCaptureState());
	EXPECT_EQ(aTeam.getName(), aItem->getOwningTeam());
	orwell::game::Team aOtherTeam("other_team");
	aItem->startCapture(aOtherTeam.getName());
	EXPECT_EQ(orwell::game::CaptureState::STARTED, aItem->getCaptureState());
	aItem->capture(aOtherTeam);
	EXPECT_EQ(orwell::game::CaptureState::SUCCEEDED, aItem->getCaptureState());
	EXPECT_EQ(aOtherTeam.getName(), aItem->getOwningTeam());
}


int main(int argc, char ** argv)
{
	return RunTest(argc, argv, "test_orwell_game_Item");
}
