#pragma once

#include <map>
#include <set>
#include <memory>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace orwell
{
class Server;

namespace game
{
class Clock
{
public :
	/// \param iDuration
	///		The duration the clock is expected to run once started.
	Clock(boost::posix_time::time_duration const & iDuration);
	~Clock();

	bool getIsRunning() const;

	uint64_t getSecondsLeft() const;

	void start();
	void stop();

	/// Simulated time tick
	void tickDelta(boost::posix_time::time_duration const & iTickDuration);

	/// Realtime tick
	void tick();

private :
	void tickInternal(boost::posix_time::ptime const & iNewTime);
	void checkReachedDuration();
	boost::posix_time::time_duration m_duration;
	/// True if and only if the clock is running
	bool m_isRunning;
	boost::posix_time::ptime m_time;
	boost::posix_time::ptime m_startTime;
};

} // game
} // orwell


