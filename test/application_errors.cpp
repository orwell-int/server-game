#include "orwell/Server.hpp"
#include "orwell/BroadcastServer.hpp"

#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <cassert>

#include <log4cxx/ndc.h>

#include "orwell/Application.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/call_traits.hpp>

#include <vector>
#include <functional>

#include "Common.hpp"


enum class Status
{
	FAIL,
	PASS,
};


static void test_ReadParameters(
		Status const iTestStatus,
		Arguments iArguments)
{
	ORWELL_LOG_DEBUG("arguments:" << iArguments);
	orwell::Application::Parameters aParameters;
	bool result = orwell::Application::ReadParameters(
			iArguments.m_argc, iArguments.m_argv, aParameters);
	assert((Status::PASS == iTestStatus) == result);
}

static void test_nothing()
{
	ORWELL_LOG_DEBUG("test_nothing");
	// we get default arguments
	test_ReadParameters(Status::PASS, Common::GetArugments());
}

static void test_wrong_port_range_publisher_1()
{
	ORWELL_LOG_DEBUG("test_wrong_port_range_publisher_1");
	test_ReadParameters(Status::FAIL, Common::GetArugments(false, 0, 42, 43));
}

static void test_wrong_port_range_publisher_2()
{
	ORWELL_LOG_DEBUG("test_wrong_port_range_publisher_2");
	test_ReadParameters(Status::PASS, Common::GetArugments(false, -1024, 42, 43));
}

static void test_wrong_port_range_publisher_3()
{
	ORWELL_LOG_DEBUG("test_wrong_port_range_publisher_3");
	test_ReadParameters(Status::FAIL, Common::GetArugments(false, 99999, 42, 43));
}

static void test_same_ports_agent_publisher()
{
	ORWELL_LOG_DEBUG("test_same_ports_agent_publisher");
	test_ReadParameters(Status::FAIL, Common::GetArugments(false, 41, 42, 41));
}

static void test_same_ports_puller_publisher()
{
	ORWELL_LOG_DEBUG("test_same_ports_puller_publisher");
	test_ReadParameters(Status::FAIL, Common::GetArugments(false, 41, 41, 43));
}

static void test_same_ports_puller_agent()
{
	ORWELL_LOG_DEBUG("test_same_ports_puller_agent");
	test_ReadParameters(Status::FAIL, Common::GetArugments(false, 41, 42, 42));
}

static void test_most_arguments()
{
	test_ReadParameters(Status::PASS, Common::GetArugments(
			false, // help
			41, // publisher port
			42, // puller port
			43, // agent port
			boost::none, // std::string("orwell.rc"), // orwellrc
			666, // tick interval
			false, // version
			true, // debug log
			true, // no broadcast
			false)); // dry run
}

int main()
{
	orwell::support::GlobalLogger::Create("application_errors", "orwell.log", true);
	log4cxx::NDC ndc("application_errors");

	test_nothing();
	test_wrong_port_range_publisher_1();
	test_wrong_port_range_publisher_2();
	test_wrong_port_range_publisher_3();
	test_same_ports_agent_publisher();
	test_same_ports_puller_publisher();
	test_same_ports_puller_agent();
	test_most_arguments();

	orwell::support::GlobalLogger::Clear();
	return 0;
}

