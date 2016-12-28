#pragma once

#include "orwell/game/Item.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>
#include <memory>
#include <map>

namespace orwell
{
namespace game
{
class IContactHandler;
class Game;
class Robot;
namespace item
{

class FlagDetector
{
public :
	FlagDetector(
			IContactHandler & iContactHandler,
			std::shared_ptr< orwell::game::Robot > iRobot);

	std::weak_ptr< orwell::game::Item > setColour(
			int32_t const iColourCode,
			boost::posix_time::ptime const iCurrentTime);

	static int32_t const kFrontierColourCode = 0;
	static int32_t const kNoneColourCode = -1;
private :
	enum class States
	{
		OUTSIDE,
		FRONTIER,
		UNKNOWN_FROM_FRONTIER,
		INSIDE,
		TRANSITION_FROM_INSIDE,
	};

	int32_t m_colourCode;

	States m_state;

	IContactHandler & m_contactHandler;
	std::shared_ptr< orwell::game::Robot > m_robot;

	std::weak_ptr< Item > m_lastItem;
};

} // item
} // game
} // orwell
