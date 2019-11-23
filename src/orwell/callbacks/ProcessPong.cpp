#include "orwell/callbacks/ProcessPong.hpp"

#include <unistd.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "orwell/com/RawMessage.hpp"

#include "robot.pb.h"

#include "orwell/support/GlobalLogger.hpp"

using orwell::messages::Pong;
using orwell::com::RawMessage;

namespace orwell
{
namespace callbacks
{

ProcessPong::ProcessPong(
		game::Game & ioGame,
		std::shared_ptr< com::Sender > ioPublisher,
		std::shared_ptr< com::Socket > ioReplier)
	: InterfaceProcess(ioGame, ioPublisher, ioReplier)
{
}

void ProcessPong::execute()
{
	ORWELL_LOG_INFO("ProcessPong::execute");
	std::string const & aDestination = getArgument("RoutingID");
	orwell::messages::Pong aPongMessage = static_cast<orwell::messages::Pong const & >(*m_msg);
	boost::posix_time::ptime const aNow = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime const aReference(boost::gregorian::date(1970, 1, 1)); 
	uint64_t const aTimestamp = (aNow - aReference).total_milliseconds();
	
	std::string const aExpectedLogger("server_game");
	for (int64_t i = 0 ; i < aPongMessage.timing_size() ; ++i)
	{
		orwell::messages::Timing * aTiming = aPongMessage.mutable_timing(i);
		std::string const aLogger = aTiming->logger();
		uint64_t const aPingTimestamp = aTiming->timestamp();
		uint64_t aElapsed = 0;
		if (aExpectedLogger == aLogger)
		{
			if (not aTiming->has_timestamp())
			{
				ORWELL_LOG_WARN("Missing timestamp in Pong message");
				break;
			}
			aElapsed = aTimestamp - aPingTimestamp;
			aTiming->set_elapsed(aElapsed);
		}
		else
		{
			if (aTiming->has_elapsed())
			{
				aElapsed = aTiming->elapsed();
			}
		}
		ORWELL_LOG_INFO("pong: " << aLogger << " @" << aPingTimestamp << " elapsed = " << aElapsed);
	}
	RawMessage aReply(aDestination, "Pong", aPongMessage.SerializeAsString());
	m_publisher->send(aReply);
}

}
}
