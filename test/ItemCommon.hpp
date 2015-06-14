#pragma once

#include <string>

#include <unistd.h>
#include <thread>

#include "orwell/Application.hpp"

#include "Common.hpp"

struct FlagAndTime
{
	std::string m_flag;
	uint64_t m_timeStamp;
	bool m_hasWinner;
};

class TestParameters
{
public:
	TestParameters(
			orwell::Application::CommandLineParameters const & iCommandLineArguments);
	orwell::Application::Parameters const & getParameters() const;
private:
	orwell::Application::Parameters m_parameters;
};

class ItemTester
{
public:
	ItemTester(
			orwell::Application::Parameters const & aParameters,
			std::vector< FlagAndTime > const & iFlagsAndTimes,
			uint64_t const iScore);
	void run();
private:
	orwell::Application::Parameters const & m_parameters;
	std::string m_score;
	TestAgent m_testAgent;
	std::thread m_applicationThread;
	std::thread m_proxySendsRobotStateThread;
};
