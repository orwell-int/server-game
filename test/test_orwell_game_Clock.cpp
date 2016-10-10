#include "orwell/game/Clock.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdint>

#include <log4cxx/ndc.h>

#include "orwell/game/Game.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"


class Failer
{
public :
	Failer();

	void addFailure();

	uint64_t getFailures() const;
private :
	uint64_t m_failures;
};

Failer::Failer()
	: m_failures(0)
{
}

void Failer::addFailure()
{
	++m_failures;
}

uint64_t Failer::getFailures() const
{
	return m_failures;
}

#define ORWELL_FAIL(Expected, Received, Message, Failer) \
{\
	if (Expected != Received)\
	{\
		ORWELL_LOG_ERROR("expected: " << Expected);\
		ORWELL_LOG_ERROR("received: " << Received);\
		ORWELL_LOG_ERROR(Message);\
		Failer.addFailure();\
	}\
}\

#define ORWELL_FAIL_TRUE(Condition, Message, Failer) \
{\
	if (!Condition)\
	{\
		ORWELL_LOG_ERROR(Message);\
		Failer.addFailure();\
	}\
}\

void test_Clock_not_running(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_not_running");
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(10));
	ORWELL_FAIL_TRUE(
			not aClock.getIsRunning(),
			"The clock is not running before being started",
			ioFailer);
}

void test_Clock_initial_getSecondsLeft(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_initial_getSecondsLeft");
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(10));
	ORWELL_FAIL(
			10,
			aClock.getSecondsLeft(),
			"The clock is not running before being started",
			ioFailer);
}

void test_Clock_start(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_start");
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(10));
	aClock.start();
	ORWELL_FAIL_TRUE(
			aClock.getIsRunning(),
			"The clock is running after being started",
			ioFailer);
	usleep(1e6);
	ORWELL_FAIL(
			10,
			aClock.getSecondsLeft(),
			"Time is not decremented by itself.",
			ioFailer);
}

void test_Clock_tickDelta(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_tickDelta");
	uint64_t aSeconds = 10;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(aSeconds));
	aClock.start();
	ORWELL_FAIL_TRUE(
			aClock.getIsRunning(),
			"The clock is running after being started",
			ioFailer);
	aClock.tickDelta(boost::posix_time::seconds(1));
	--aSeconds;
	ORWELL_FAIL(
			aSeconds,
			aClock.getSecondsLeft(),
			"Time is decremented.",
			ioFailer);
	for (auto i = 0 ; i < 5 ; ++i)
	{
		aClock.tickDelta(boost::posix_time::seconds(1));
		--aSeconds;
	}
	ORWELL_FAIL(
			aSeconds,
			aClock.getSecondsLeft(),
			"Time is decremented.",
			ioFailer);
}


void test_Clock_final_tickDelta(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_final_tickDelta");
	uint64_t const kOneSecond = 1;
	uint64_t const kStartSeconds = 10 * kOneSecond;
	uint64_t aSeconds = kStartSeconds;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(aSeconds));
	aClock.start();
	ORWELL_FAIL_TRUE(
			aClock.getIsRunning(),
			"The clock is running after being started",
			ioFailer);
	for (uint64_t i = 0 ; i < kStartSeconds ; ++i)
	{
		aClock.tickDelta(boost::posix_time::seconds(kOneSecond));
		--aSeconds;
	}
	ORWELL_FAIL(
			kStartSeconds,
			aClock.getSecondsLeft(),
			"Time is decremented.",
			ioFailer);
	ORWELL_FAIL_TRUE(
			not aClock.getIsRunning(),
			"The clock is not running when reaching the duration.",
			ioFailer);
}

void test_Clock_harmless_start(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_harmless_start");
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(10));
	aClock.start();
	aClock.start();
	ORWELL_FAIL_TRUE(
			aClock.getIsRunning(),
			"The clock is running even with start called multiple times",
			ioFailer);
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.start();
	ORWELL_FAIL_TRUE(
			aClock.getIsRunning(),
			"The clock is running even with start called multiple times",
			ioFailer);
}

void test_Clock_start_stop(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_start_stop");
	uint64_t const kSeconds = 10;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(kSeconds));
	aClock.start();
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.stop();
	ORWELL_FAIL_TRUE(
			not aClock.getIsRunning(),
			"The clock is not running after being stopped",
			ioFailer);
	ORWELL_FAIL(
			kSeconds,
			aClock.getSecondsLeft(),
			"Time is back to its initial value after stop.",
			ioFailer);
}

void test_Clock_harmless_stop(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_harmless_stop");
	uint64_t const kSeconds = 10;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(kSeconds));
	aClock.stop();
	ORWELL_FAIL_TRUE(
			not aClock.getIsRunning(),
			"The clock is not running after being wrongly stopped",
			ioFailer);
}

void test_Clock_loop_start_stop(Failer & ioFailer)
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
	ORWELL_FAIL_TRUE(
			not aClock.getIsRunning(),
			"The clock is not running after being stopped",
			ioFailer);
	ORWELL_FAIL(
			kSeconds,
			aClock.getSecondsLeft(),
			"Time is back to its initial value after stop.",
			ioFailer);
}

void test_Clock_harmless_tickDelta(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_harmless_tickDelta");
	uint64_t const kSeconds = 10;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(kSeconds));
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.tickDelta(boost::posix_time::seconds(1));
	aClock.start();
	ORWELL_FAIL(
			kSeconds,
			aClock.getSecondsLeft(),
			"Time remains at initial value (harmless tickDelta).",
			ioFailer);
}

void test_Clock_tick_2(Failer & ioFailer)
{
	log4cxx::NDC aLocalNDC("test_Clock_tick_2");
	uint64_t const kSeconds = 1;
	auto aClock = orwell::game::Clock(boost::posix_time::seconds(kSeconds));
	aClock.start();
	for (uint64_t i = 0 ; i < 10 ; ++i)
	{
		ORWELL_FAIL_TRUE(
				aClock.getIsRunning(),
				"The clock is running",
				ioFailer);
		usleep(kSeconds * 1e5);
		aClock.tick();
	}
	ORWELL_FAIL_TRUE(
			not aClock.getIsRunning(),
			"The clock reached the desired duration",
			ioFailer);
}

int main()
{
	orwell::support::GlobalLogger::Create("test_orwell_game_Clock", "test_orwell_game_Clock.log", true);
	log4cxx::NDC ndc("test_orwell_game_Clock");
	ORWELL_LOG_INFO("Test starts\n");

	Failer aFailer;

	test_Clock_not_running(aFailer);
	test_Clock_initial_getSecondsLeft(aFailer);
	test_Clock_start(aFailer);
	test_Clock_tickDelta(aFailer);
	test_Clock_final_tickDelta(aFailer);
	test_Clock_harmless_start(aFailer);
	test_Clock_start_stop(aFailer);
	test_Clock_harmless_stop(aFailer);
	test_Clock_loop_start_stop(aFailer);
	test_Clock_harmless_tickDelta(aFailer);
	test_Clock_tick_2(aFailer);

	orwell::support::GlobalLogger::Clear();
	return aFailer.getFailures();
}
