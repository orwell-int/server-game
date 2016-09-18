#pragma once
#include "orwell/game/TimeBound.hpp"

#include <memory>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace orwell
{
namespace game
{
class Item;
class Robot;

class Contact : public TimeBound
{
public:
	Contact(
			boost::posix_time::ptime const & iStartTime,
			boost::posix_time::time_duration const & iTimerDuration,
			std::shared_ptr<Robot> iRobot,
			std::shared_ptr<Item> iItem);
	~Contact();

	StepSignal step(boost::posix_time::ptime const & iCurrentTime) override;

private:
	std::shared_ptr< Robot > m_robot;
	std::shared_ptr< Item > m_item;
	boost::posix_time::ptime const m_stopTime;
	bool m_finished;
};

}
}
