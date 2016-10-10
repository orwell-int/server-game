#include <stdlib.h>
#include <thread>

#include <log4cxx/ndc.h>

#include <boost/lexical_cast.hpp>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/game/Landmark.hpp"
#include "orwell/game/RGBColour.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/RawMessage.hpp"
#include "server-game.pb.h"

#include "Common.hpp"

int g_status = 0;

class DurationPredicate
{

public :
	virtual void check(uint64_t const iTime) const = 0;

protected :
	DurationPredicate(uint64_t const iSeconds)
		: m_seconds(iSeconds)
	{
	}

	uint64_t m_seconds;
};

class LessThan : public DurationPredicate
{
public :
	LessThan(uint64_t const iSeconds)
		: DurationPredicate(iSeconds)
	{
	}

	void check(uint64_t const iTime) const override
	{
		std::ostringstream aStream;
		aStream << "This is false: " << iTime << "(var) < " << m_seconds << "(const)";
		g_status -= 1;
		ORWELL_ASSERT_TRUE((iTime < m_seconds), aStream.str());
		g_status += 1;
	}
};

class MoreThan : public DurationPredicate
{
public :
	MoreThan(uint64_t const iSeconds)
		: DurationPredicate(iSeconds)
	{
	}

	void check(uint64_t const iTime) const override
	{
		std::ostringstream aStream;
		aStream << "This is false: " << iTime << "(var) > " << m_seconds << "(const)";
		g_status -= 1;
		ORWELL_ASSERT_TRUE((iTime > m_seconds), aStream.str());
		g_status += 1;
	}
};

class Equals : public DurationPredicate
{
public :
	Equals(uint64_t const iSeconds)
		: DurationPredicate(iSeconds)
	{
	}

	void check(uint64_t const iTime) const override
	{
		std::ostringstream aStream;
		aStream << "This is false: " << iTime << "(var) == " << m_seconds << "(const)";
		g_status -= 1;
		ORWELL_ASSERT_TRUE((iTime == m_seconds), aStream.str());
		g_status += 1;
	}
};

static void ExpectGameTime(
		DurationPredicate const & iDurationPredicate,
		TestAgent & ioTestAgent)
{
	std::string aGameTimeAsString = ioTestAgent.sendCommand("get game time", boost::none);
	uint64_t aGameTime = boost::lexical_cast< uint64_t >(aGameTimeAsString);
	iDurationPredicate.check(aGameTime);
}

static void Application()
{
	system("../server_main -A 9003 --publisher-port 9001 --puller-port 9002 --tic 10 --game-duration 2 --gamefile orwell-game_test.ini");
}

static void Stopper()
{
	TestAgent aTestAgent(9003);
	ORWELL_LOG_INFO("create subscriber");
	zmq::context_t aContext(1);
	orwell::com::Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, orwell::com::ConnectionMode::CONNECT, aContext);
	aTestAgent.sendCommand("ping", std::string("pong"));
	aTestAgent.sendCommand("add team TEAM");
	aTestAgent.sendCommand("add robot toto TEAM");
	aTestAgent.sendCommand("set robot toto video_url fake");
	aTestAgent.sendCommand("register robot toto");
	ExpectGameTime(Equals(2), aTestAgent);
	aTestAgent.sendCommand("start game");
	usleep(100 * 1000);
	bool aIsRunning = boost::lexical_cast< bool >(
			aTestAgent.sendCommand("get game running", boost::none));
	ORWELL_ASSERT_TRUE(aIsRunning, "Game should be running.");
	usleep(1000 * 1000);
	ExpectGameTime(MoreThan(0), aTestAgent);
	ExpectGameTime(LessThan(2), aTestAgent);
	usleep(1000 * 1000);
	aIsRunning = boost::lexical_cast< bool >(
			aTestAgent.sendCommand("get game running", boost::none));
	ORWELL_ASSERT_TRUE((!aIsRunning), "Game should not be running any more.");
	aTestAgent.sendCommand("stop application");
}

int main()
{
	orwell::support::GlobalLogger::Create(
			"test_stop_after_game_duration",
			"test_stop_after_game_duration.log",
			true);
	log4cxx::NDC ndc("test_main_plus");
	ORWELL_LOG_INFO("Test starts\n");

	std::thread aApplicationThread(Application);
	std::thread aAgentThread(Stopper);
	aApplicationThread.join();
	aAgentThread.join();
	orwell::support::GlobalLogger::Clear();
	return g_status;
}

