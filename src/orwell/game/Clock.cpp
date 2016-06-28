#include "orwell/game/Clock.hpp"

#include "orwell/support/GlobalLogger.hpp"

namespace orwell
{
namespace game
{

Clock::Clock(boost::posix_time::time_duration const & iDuration)
	: m_duration(iDuration)
	, m_isRunning(false)
{
}

Clock::~Clock()
{
}

bool Clock::getIsRunning() const
{
	return m_isRunning;
}

uint64_t Clock::getSecondsLeft() const
{
	return (m_isRunning)
		? m_duration.total_seconds() + (m_startTime - m_time).total_seconds()
		: m_duration.total_seconds();
}

void Clock::start()
{
	if (m_isRunning)
	{
		return;
	}
	m_isRunning = true;
	m_time = boost::posix_time::microsec_clock::local_time();
	m_startTime = m_time;
}

void Clock::stop()
{
	if (not m_isRunning)
	{
		return;
	}
	m_isRunning = false;
}

void Clock::tickDelta(boost::posix_time::time_duration const & iTickDuration)
{
	if (m_isRunning)
	{
		tickInternal(m_time + iTickDuration);
	}
}

void Clock::tick()
{
	if (m_isRunning)
	{
		tickInternal(boost::posix_time::microsec_clock::local_time());
	}
}

void Clock::tickInternal(boost::posix_time::ptime const & iNewTime)
{
	m_time = iNewTime;
	checkReachedDuration();
}

void Clock::checkReachedDuration()
{
	if ((m_time - m_startTime) >= m_duration)
	{
		m_isRunning = false;
	}
}

} // game
} // orwell

