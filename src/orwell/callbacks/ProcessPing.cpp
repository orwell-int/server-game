#include "orwell/callbacks/ProcessPing.hpp"

#include "orwell/com/RawMessage.hpp"

#include "controller.pb.h"

#include "orwell/support/GlobalLogger.hpp"

#include <unistd.h>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace log4cxx;
using namespace log4cxx::helpers;

using orwell::messages::Ping;
using orwell::com::RawMessage;

namespace orwell
{
namespace callbacks
{

ProcessPing::ProcessPing(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher, ioGame)
{
}

void ProcessPing::execute()
{
	ORWELL_LOG_INFO("ProcessPing::execute");
	std::string const & aDestination = getArgument("RoutingID");
	orwell::messages::Ping aPingMessage = static_cast<orwell::messages::Ping const & >(*m_msg);
	for (int64_t i = 0 ; i < aPingMessage.timing_size() ; ++i)
	{
		orwell::messages::Timing * aTiming = aPingMessage.mutable_timing(i);
		std::string const aLogger = aTiming->logger();
		uint64_t const aPingTimestamp = aTiming->timestamp();
		if (aTiming->has_elapsed())
		{
			uint64_t const aElapsed = aTiming->elapsed();
			ORWELL_LOG_INFO("ping: " << aLogger << " @" << aPingTimestamp << " elapsed = " << aElapsed);
		}
		else
		{
			ORWELL_LOG_INFO("ping: " << aLogger << " @" << aPingTimestamp << " elapsed = N/A");
		}
	}
	boost::posix_time::ptime const aNow = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime const aReference(boost::gregorian::date(1970, 1, 1)); 
	uint64_t const aTimestamp = (aNow - aReference).total_milliseconds();
	orwell::messages::Timing * aTiming = aPingMessage.add_timing();
	aTiming->set_logger("server_game");
	ORWELL_LOG_INFO("ping: @" << aTimestamp);
	aTiming->set_timestamp(aTimestamp);
	RawMessage aReply(aDestination, "Ping", aPingMessage.SerializeAsString());
	m_publisher->send(aReply);
}

}
}

