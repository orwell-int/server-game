#include "orwell/game/Contact.hpp"

#include <stdexcept>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "orwell/support/GlobalLogger.hpp"
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
	, m_finished(false)
{
	ORWELL_LOG_DEBUG("Create a contact");
	m_item->startCapture(m_robot->getTeam().getName());
}

Contact::~Contact()
{
	if (not m_finished)
	{
		m_item->abortCapture();
	}
}

StepSignal Contact::step(boost::posix_time::ptime const & iCurrentTime)
{
	if (m_finished)
	{
		throw std::logic_error("step should not be called once the contact is 'finished'");
	}
	StepSignal aResult = StepSignal::SILENCEIKILLU;
	ORWELL_LOG_DEBUG("Step in a contact");
	if (iCurrentTime > m_stopTime)
	{
		m_robot->getTeam().captureItem(m_item);
		m_finished = true;
	}
	else
	{
		aResult = StepSignal::AH_AH_AH_AH_STAYINGALIVE;
	}
	return aResult;
}

} // game
} // orwell
