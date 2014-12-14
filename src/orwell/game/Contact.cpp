#include "orwell/game/Contact.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include "orwell/game/Item.hpp"
#include "orwell/game/Robot.hpp"

namespace orwell {
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
{}

StepSignal Contact::step(boost::posix_time::ptime const & iCurrentTime)
{
	return StepSignal::Ah_AH_AH_AH_STAYINGALIVE;
}

}} // namespaces


