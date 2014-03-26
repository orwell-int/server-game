#include "Common.hpp"

#include "orwell/com/Receiver.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "MissingFromTheStandard.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <unistd.h>

using namespace log4cxx;

bool Common::ExpectMessage(
		std::string const & iType,
		orwell::com::Receiver & iSubscriber,
		orwell::com::RawMessage & oReceived,
		unsigned int const iTimeout)
{
	ORWELL_LOG_DEBUG("Wait for message of type " << iType << " for " << iTimeout);
	bool aReceivedExpectedMessage(false);
	boost::posix_time::time_duration aTrueTimeout = boost::posix_time::milliseconds(iTimeout);
	boost::posix_time::time_duration aDuration;
	boost::posix_time::ptime aCurrentTime;
	boost::posix_time::ptime aStartTime = boost::posix_time::microsec_clock::local_time();
	bool aReceivedAnyMessage(false);
	while (not aReceivedExpectedMessage and (aDuration < aTrueTimeout))
	{
		aCurrentTime = boost::posix_time::microsec_clock::local_time();
		aDuration = aCurrentTime - aStartTime;
		aReceivedAnyMessage = iSubscriber.receive(oReceived);
		if (not aReceivedAnyMessage or oReceived._type != iType)
		{
			if (aReceivedAnyMessage)
			{
				ORWELL_LOG_DEBUG("Discarded message of type " << iType);
			}
			usleep( 10 );
		}
		else
		{
			ORWELL_LOG_DEBUG("Accepted message of type " << iType);
			aReceivedExpectedMessage = true;
		}
	}
	if (not aReceivedExpectedMessage)
	{
		if (aDuration >= aTrueTimeout)
		{
			ORWELL_LOG_DEBUG("Excpected message not received ; timeout ("
					<< aTrueTimeout << ") exceeded: " << aDuration);
		}
	}
	return aReceivedExpectedMessage;
}

