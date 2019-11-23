#include "orwell/game/Clock.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdint>

#include <log4cxx/ndc.h>

#include "orwell/game/Game.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"

TEST(Application, test_Clock_not_running)
{
	log4cxx::NDC aLocalNDC("test_Clock_not_running");
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(10));
	EXPECT_FALSE(aClock.getIsRunning())
		<< "The clock is not running before being started";
}

TEST(Application, test_Clock_initial_getSecondsLeft)
{
	log4cxx::NDC aLocalNDC("test_Clock_initial_getSecondsLeft");
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(10));
	EXPECT_EQ(uint64_t{10}, aClock.getSecondsLeft())
		<< "The clock is not running before being started";
}

TEST(Application, test_Clock_start)
{
	log4cxx::NDC aLocalNDC("test_Clock_start");
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(10));
	aClock.start();
	EXPECT_TRUE(aClock.getIsRunning())
		<< "The clock is running after being started";
	usleep(1e6);
	EXPECT_EQ(uint64_t{10}, aClock.getSecondsLeft())
		<< "Time is not decremented by itself.";
}

TEST(Application, test_Clock_tickDelta)
{
	log4cxx::NDC aLocalNDC("test_Clock_tickDelta");
	uint64_t aSeconds = 10;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(aSeconds));
	aClock.start();
	EXPECT_TRUE(aClock.getIsRunning())
		<< "The clock is running after being started";
	aClock.tickDelta(boost::posix_time::seconds(1));
	--aSeconds;
	EXPECT_EQ(aSeconds, aClock.getSecondsLeft())
		<< "Time is decremented.";
	for (auto i = 0 ; i < 5 ; ++i)
	{
		aClock.tickDelta(boost::posix_time::seconds(1));
		--aSeconds;
	}
	EXPECT_EQ(aSeconds, aClock.getSecondsLeft())
		<< "Time is decremented.";
}


TEST(Application, test_Clock_final_tickDelta)
{
	log4cxx::NDC aLocalNDC("test_Clock_final_tickDelta");
	uint64_t const kOneSecond = 1;
	uint64_t const kStartSeconds = 10 * kOneSecond;
	uint64_t aSeconds = kStartSeconds;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(aSeconds));
	aClock.start();
	EXPECT_TRUE(aClock.getIsRunning())
		<< "The clock is running after being started";
	for (uint64_t i = 0 ; i < kStartSeconds ; ++i)
	{
		aClock.tickDelta(boost::posix_time::seconds(kOneSecond));
		--aSeconds;
	}
	EXPECT_EQ(kStartSeconds, aClock.getSecondsLeft())
		<< "Time is decremented.";
	EXPECT_FALSE(aClock.getIsRunning())
		<< "The clock is not running when reaching the duration.";
}

TEST(Application, test_Clock_harmless_start)
{
	log4cxx::NDC aLocalNDC("test_Clock_harmless_start");
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(10));
	aClock.start();
	aClock.start();
	EXPECT_TRUE(aClock.getIsRunning())
		<< "The clock is running even with start called multiple times";
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.start();
	EXPECT_TRUE(aClock.getIsRunning())
		<< "The clock is running even with start called multiple times";
}

TEST(Application, test_Clock_start_stop)
{
	log4cxx::NDC aLocalNDC("test_Clock_start_stop");
	uint64_t const kSeconds = 10;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(kSeconds));
	aClock.start();
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.stop();
	EXPECT_FALSE(aClock.getIsRunning())
		<< "The clock is not running after being stopped";
	EXPECT_EQ(kSeconds, aClock.getSecondsLeft())
		<< "Time is back to its initial value after stop.";
}

TEST(Application, test_Clock_harmless_stop)
{
	log4cxx::NDC aLocalNDC("test_Clock_harmless_stop");
	uint64_t const kSeconds = 10;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(kSeconds));
	aClock.stop();
	EXPECT_FALSE(aClock.getIsRunning())
		<< "The clock is not running after being wrongly stopped";
}

TEST(Application, test_Clock_loop_start_stop)
{
	log4cxx::NDC aLocalNDC("test_Clock_loop_start_stop");
	uint64_t const kSeconds = 10;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(kSeconds));
	for (uint64_t i = 0 ; i < 20 ; ++i)
	{
		aClock.start();
		aClock.tickDelta(boost::posix_time::seconds(1));
		aClock.stop();
	}
	EXPECT_FALSE(aClock.getIsRunning())
		<< "The clock is not running after being stopped";
	EXPECT_EQ(kSeconds, aClock.getSecondsLeft())
		<< "Time is back to its initial value after stop.";
}

TEST(Application, test_Clock_harmless_tickDelta)
{
	log4cxx::NDC aLocalNDC("test_Clock_harmless_tickDelta");
	uint64_t const kSeconds = 10;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(kSeconds));
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.start();
	EXPECT_EQ(kSeconds, aClock.getSecondsLeft())
		<< "Time remains at initial value (harmless tickDelta).";
}

TEST(Application, test_Clock_tick_2)
{
	log4cxx::NDC aLocalNDC("test_Clock_tick_2");
	uint64_t const kSeconds = 1;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(kSeconds));
	aClock.start();
	for (uint64_t i = 0 ; i < 5 ; ++i)
	{
		EXPECT_TRUE(aClock.getIsRunning())
			<< "The clock is running";
		usleep(kSeconds * 1e5);
		aClock.tick();
	}
	// last iterations might exceed the duration
	for (uint64_t i = 0 ; i < 5 ; ++i)
	{
		usleep(kSeconds * 1e5);
		aClock.tick();
	}
	aClock.tick();
	EXPECT_FALSE(aClock.getIsRunning())
		<< "The clock reached the desired duration";
}

int main(int argc, char ** argv)
{
	return RunTest(argc, argv, "test_orwell_game_Clock");
}
