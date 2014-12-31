#include "orwell/game/Contact.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include "orwell/game/Item.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Team.hpp"

namespace orwell
{
namespace game
{

Contact::Contact(
		boost::posix_time::ptime const & iStartTime,
		boost::posix_time::time_duration const & iTimerDuration,
		std::shared_ptr<Robot> iRobot,
		std::shared_ptr<Item> iItem)
	: m_robot(iRobot)
	, m_item(iItem)
	, m_stopTime(iStartTime + iTimerDuration)
{
}

Contact::~Contact()
{
}

StepSignal Contact::step(boost::posix_time::ptime const & iCurrentTime)
{
	StepSignal aResult = StepSignal::SILENCEIKILLU;
	if (iCurrentTime > m_stopTime)
	{
		m_robot->getTeam().captureItem(m_item);
	}
	else
	{
		aResult = StepSignal::AH_AH_AH_AH_STAYINGALIVE;
	}
	return aResult;
}

} // game
} // orwell

