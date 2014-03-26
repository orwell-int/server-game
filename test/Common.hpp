#include <string>

#include <log4cxx/logger.h>

namespace orwell
{
namespace com
{
class Receiver;
class RawMessage;
}
}

class Common
{
public:
	static bool ExpectMessage(
			std::string const & iType,
			orwell::com::Receiver & iSubscriber,
			orwell::com::RawMessage & oReceived,
			unsigned int const iTimeout = 1500);

};

