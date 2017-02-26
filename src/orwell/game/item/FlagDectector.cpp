#include "orwell/game/item/FlagDetector.hpp"

#include <ostream>

#include "MissingFromTheStandard.hpp"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Team.hpp"
#include "orwell/game/IContactHandler.hpp"
#include "orwell/game/Robot.hpp"

namespace orwell
{
namespace game
{
namespace item
{

FlagDetector::FlagDetector(
		IContactHandler & iContactHandler,
		std::shared_ptr< orwell::game::Robot > iRobot)
	: m_colourCode(kFrontierColourCode)
	, m_state(States::OUTSIDE)
	, m_contactHandler(iContactHandler)
	, m_robot(iRobot)
{
}

std::weak_ptr< orwell::game::Item > FlagDetector::setColour(
		int32_t const iColourCode,
		boost::posix_time::ptime const iCurrentTime)
{
	bool aItemChanged = false;
	switch (m_state)
	{
		case States::OUTSIDE:
		{
			ORWELL_LOG_DEBUG("OUTSIDE");
			if (FlagDetector::kFrontierColourCode == iColourCode)
			{
				m_state = States::FRONTIER;
				ORWELL_LOG_DEBUG(" -> FRONTIER");
			}
			break;
		}
		case States::FRONTIER:
		{
			ORWELL_LOG_DEBUG("FRONTIER");
			std::shared_ptr< orwell::game::Item > aItem = game::Item::GetItemByColour(iColourCode);
			if (aItem)
			{
				m_state = States::INSIDE;
				ORWELL_LOG_DEBUG(" -> INSIDE");
				m_lastItem = aItem;
				m_colourCode = iColourCode;
				aItemChanged = true;
				m_contactHandler.robotIsInContactWith(m_robot->getRobotId(), aItem);
			}
			else
			{
				m_state = States::UNKNOWN_FROM_FRONTIER;
				ORWELL_LOG_DEBUG(" -> UNKNOWN_FROM_FRONTIER");
			}
			break;
		}
		case States::UNKNOWN_FROM_FRONTIER:
		{
			ORWELL_LOG_DEBUG("UNKNOWN_FROM_FRONTIER");
			std::shared_ptr< orwell::game::Item > aItem = game::Item::GetItemByColour(iColourCode);
			if (aItem)
			{
				m_colourCode = iColourCode;
				auto aLastItem = m_lastItem.lock();
				if (aLastItem)
				{
					if ((aItem.get() == aLastItem.get()))
					{
					}
					else
					{
						ORWELL_LOG_WARN("Finding a different item from the frontier ?");
					}
				}
				else
				{
					m_lastItem = aItem;
					aItemChanged = true;
					m_contactHandler.robotIsInContactWith(m_robot->getRobotId(), aItem);
				}
				m_state = States::INSIDE;
				ORWELL_LOG_DEBUG(" -> INSIDE");
			}
			break;
		}
		case States::INSIDE:
		{
			ORWELL_LOG_DEBUG("INSIDE");
			if (FlagDetector::kFrontierColourCode == iColourCode)
			{
				m_state = States::FRONTIER;
				m_colourCode = kFrontierColourCode;
				auto aLastItem = m_lastItem.lock();
				if (aLastItem)
				{
					m_contactHandler.robotDropsContactWith(m_robot->getRobotId(), aLastItem);
				}
				m_lastItem.reset();
				ORWELL_LOG_DEBUG(" -> FRONTIER");
			}
			else if (iColourCode != m_colourCode)
			{
				m_state = States::TRANSITION_FROM_INSIDE;
				ORWELL_LOG_DEBUG(" -> TRANSITION_FROM_INSIDE");
			}
			break;
		}
		case States::TRANSITION_FROM_INSIDE:
		{
			ORWELL_LOG_DEBUG("TRANSITION_FROM_INSIDE");
			if (iColourCode == m_colourCode)
			{
				auto aLastItem = m_lastItem.lock();
				if (aLastItem)
				{
					m_state = States::INSIDE;
					ORWELL_LOG_DEBUG(" -> INSIDE");
				}
			}
			else if (FlagDetector::kFrontierColourCode == iColourCode)
			{
				m_state = States::FRONTIER;
				m_colourCode = kFrontierColourCode;
				auto aLastItem = m_lastItem.lock();
				if (aLastItem)
				{
					m_contactHandler.robotDropsContactWith(m_robot->getRobotId(), aLastItem);
				}
				m_lastItem.reset();
				ORWELL_LOG_DEBUG(" -> FRONTIER");
			}
			break;
		}
	}
	std::weak_ptr< orwell::game::Item > aNewItem;
	if (aItemChanged)
	{
		aNewItem = m_lastItem;
	}
	return aNewItem;
}

std::ostream & operator<<(std::ostream& oOstream, const FlagDetector & aFlagDetector)
{
	return oOstream << "this is a flag detector";
}

} // item
} // game
} // orwell
