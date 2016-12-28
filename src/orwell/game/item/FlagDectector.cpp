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
			if (FlagDetector::kFrontierColourCode == iColourCode)
			{
				m_state = States::FRONTIER;
			}
			break;
		}
		case States::FRONTIER:
		{
			std::shared_ptr< orwell::game::Item > aItem = game::Item::GetItemByColour(iColourCode);
			if (aItem)
			{
				m_state = States::INSIDE;
				m_lastItem = aItem;
				aItemChanged = true;
				m_contactHandler.robotIsInContactWith(m_robot->getRobotId(), aItem);
			}
			else
			{
				if (not m_lastItem.expired())
				{
					m_lastItem.reset();
					m_contactHandler.robotDropsContactWith(m_robot->getRobotId(), aItem);
				}
				m_state = States::UNKNOWN_FROM_FRONTIER;
			}
			break;
		}
		case States::UNKNOWN_FROM_FRONTIER:
		{
			std::shared_ptr< orwell::game::Item > aItem = game::Item::GetItemByColour(iColourCode);
			if (aItem)
			{
				auto aLastItem = m_lastItem.lock();
				if ((aLastItem) and (aItem.get() == aLastItem.get()))
				{
					m_state = States::INSIDE;
				}
				else
				{
					ORWELL_LOG_WARN("Inconsistent colour transition, force UNKNOWN_FROM_FRONTIER -> OUTSIDE");
					m_state = States::OUTSIDE;
					if (not m_lastItem.expired())
					{
						m_lastItem.reset();
						m_contactHandler.robotDropsContactWith(m_robot->getRobotId(), aItem);
					}
				}
			}
			break;
		}
		case States::INSIDE:
		{
			std::shared_ptr< orwell::game::Item > aItem = game::Item::GetItemByColour(iColourCode);
			if (aItem)
			{
				if (not m_lastItem.expired())
				{
					ORWELL_LOG_WARN("Inconsistent colour transition, force INSIDE -> OUTSIDE");
					m_state = States::OUTSIDE;
					m_lastItem.reset();
					m_contactHandler.robotDropsContactWith(m_robot->getRobotId(), aItem);
				}
			}
			else
			{
				m_state = States::TRANSITION_FROM_INSIDE;
			}
			break;
		}
		case States::TRANSITION_FROM_INSIDE:
		{
			std::shared_ptr< orwell::game::Item > aItem = game::Item::GetItemByColour(iColourCode);
			if (aItem)
			{
				auto aLastItem = m_lastItem.lock();
				if ((aLastItem) and (aItem.get() == aLastItem.get()))
				{
					m_state = States::INSIDE;
				}
				else
				{
					ORWELL_LOG_WARN("Inconsistent colour transition, force TRANSITION_FROM_INSIDE -> OUTSIDE");
					m_state = States::OUTSIDE;
					m_lastItem.reset();
					m_contactHandler.robotDropsContactWith(m_robot->getRobotId(), aItem);
				}
			}
			else
			{
				if (FlagDetector::kFrontierColourCode == iColourCode)
				{
					m_state = States::FRONTIER;
				}
				else
				{
					ORWELL_LOG_WARN("Inconsistent colour transition, force UNKNOWN_FROM_FRONTIER -> OUTSIDE");
					m_state = States::OUTSIDE;
					if (not m_lastItem.expired())
					{
						m_lastItem.reset();
					}
					m_contactHandler.robotDropsContactWith(m_robot->getRobotId(), aItem);
				}
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
