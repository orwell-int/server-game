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
#include "orwell/game/Robot.hpp"
#include "orwell/game/item/Flag.hpp"
#include "orwell/game/item/FlagDetector.hpp"

#include "Common.hpp"

using ::testing::_;

class TestOrwellGameItemFlagDetector : public ::testing::Test
{
protected:
	TestOrwellGameItemFlagDetector()
		: m_type("flag")
		, m_name("FLAG")
		, m_colourCode(12)
		, m_teamName("Team Name")
		, m_robotName("Robot Name")
		, m_robotId("robot_id")
		, m_videoPort(42)
		, m_commandPort(43)
		, m_team(m_teamName)
		, m_robot(std::make_shared< orwell::game::Robot >(
				m_fakeSystemProxy,
				m_robotName,
				m_robotId,
				m_team,
				m_videoPort,
				m_commandPort))
		, m_flagDetector(
				m_contactHandler,
				m_robot)
	{
	}

	virtual void SetUp()
	{
		orwell::game::Item::CreateItem(
				m_type,
				m_name,
				m_rfids,
				m_colourCode,
				m_ruleset);
	}

	virtual void TearDown()
	{
	}

	void sendColour(uint32_t iColourCode);

	std::string m_type;
	std::string const m_name;
	std::set< std::string > m_rfids;
	int32_t const m_colourCode;
	orwell::game::Ruleset m_ruleset;
	FakeSystemProxy m_fakeSystemProxy;
	std::string const m_teamName;
	std::string const m_robotName;
	std::string const m_robotId;
	uint16_t const m_videoPort;
	uint16_t const m_commandPort;
	orwell::game::Team m_team;
	std::shared_ptr< orwell::game::Robot > m_robot;
	FakeContactHandler m_contactHandler;
	orwell::game::item::FlagDetector m_flagDetector;
};

void TestOrwellGameItemFlagDetector::sendColour(uint32_t iColourCode)
{
	m_flagDetector.setColour(
			iColourCode,
			boost::posix_time::microsec_clock::local_time());
}

TEST_F(TestOrwellGameItemFlagDetector, Frontier)
{
	EXPECT_CALL(m_contactHandler, robotIsInContactWith(_, _)).Times(0);
	EXPECT_CALL(m_contactHandler, robotDropsContactWith(_, _)).Times(0);
	sendColour(orwell::game::item::FlagDetector::kFrontierColourCode);
}

TEST_F(TestOrwellGameItemFlagDetector, Frontier_Colour_Frontier_Outside)
{
	EXPECT_CALL(m_contactHandler, robotIsInContactWith(_, _)).Times(1);
	EXPECT_CALL(m_contactHandler, robotDropsContactWith(_, _)).Times(1);
	ORWELL_LOG_DEBUG("Send kFrontierColourCode");
	sendColour(orwell::game::item::FlagDetector::kFrontierColourCode);
	ORWELL_LOG_DEBUG("Send " << m_colourCode);
	sendColour(m_colourCode);
	ORWELL_LOG_DEBUG("Send kFrontierColourCode");
	sendColour(orwell::game::item::FlagDetector::kFrontierColourCode);
	ORWELL_LOG_DEBUG("Send kNoneColourCode");
	sendColour(orwell::game::item::FlagDetector::kNoneColourCode);
}


TEST_F(TestOrwellGameItemFlagDetector, Frontier_Colour_Frontier)
{
	EXPECT_CALL(m_contactHandler, robotIsInContactWith(_, _)).Times(1);
	EXPECT_CALL(m_contactHandler, robotDropsContactWith(_, _)).Times(0);
	sendColour(orwell::game::item::FlagDetector::kFrontierColourCode);
	sendColour(m_colourCode);
}


TEST_F(TestOrwellGameItemFlagDetector, Frontier_Colour_Frontier_Outside_with_None)
{
	EXPECT_CALL(m_contactHandler, robotIsInContactWith(_, _)).Times(1);
	EXPECT_CALL(m_contactHandler, robotDropsContactWith(_, _)).Times(1);
	ORWELL_LOG_DEBUG("Send kFrontierColourCode");
	sendColour(orwell::game::item::FlagDetector::kFrontierColourCode);
	ORWELL_LOG_DEBUG("Send kNoneColourCode");
	sendColour(orwell::game::item::FlagDetector::kNoneColourCode);
	ORWELL_LOG_DEBUG("Send " << m_colourCode);
	sendColour(m_colourCode);
	ORWELL_LOG_DEBUG("Send kNoneColourCode");
	sendColour(orwell::game::item::FlagDetector::kNoneColourCode);
	ORWELL_LOG_DEBUG("Send kFrontierColourCode");
	sendColour(orwell::game::item::FlagDetector::kFrontierColourCode);
	ORWELL_LOG_DEBUG("Send kNoneColourCode");
	sendColour(orwell::game::item::FlagDetector::kNoneColourCode);
}


TEST_F(TestOrwellGameItemFlagDetector, Frontier_Colour_Frontier_with_None)
{
	EXPECT_CALL(m_contactHandler, robotIsInContactWith(_, _)).Times(1);
	EXPECT_CALL(m_contactHandler, robotDropsContactWith(_, _)).Times(0);
	sendColour(orwell::game::item::FlagDetector::kFrontierColourCode);
	sendColour(orwell::game::item::FlagDetector::kNoneColourCode);
	sendColour(m_colourCode);
}


int main(int argc, char ** argv)
{
	std::string const aName = "test_orwell_game_item_FlagDetector";
	orwell::support::GlobalLogger::Create(aName, aName, true);
	log4cxx::NDC ndc(aName);
	ORWELL_LOG_INFO("Test starts\n");
	::testing::InitGoogleTest(&argc, argv);
	int aResult = RUN_ALL_TESTS();
	orwell::support::GlobalLogger::Clear();
	return aResult;
}
