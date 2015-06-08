#pragma once

#include <string>

#include <unistd.h>
#include <thread>

#include "orwell/Application.hpp"

#include "Common.hpp"

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
	ItemTester(orwell::Application::Parameters const & aParameters);
	void run();
private:
	orwell::Application::Parameters const & m_parameters;
	TestAgent m_testAgent;
	std::thread m_applicationThread;
	std::thread m_proxySendsRobotStateThread;
};
