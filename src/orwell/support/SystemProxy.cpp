#include "orwell/support/SystemProxy.hpp"

namespace orwell
{
namespace support
{

void SystemProxy::mkstemp(char * ioTemplate) const
{
	mkstemp(ioTemplate);
}

int SystemProxy::system(char const * iCommand) const
{
	return system(iCommand);
}

}
}
