#include "orwell/support/SystemProxy.hpp"

#include <stdlib.h>
#include <unistd.h>

namespace orwell
{
namespace support
{

int SystemProxy::mkstemp(char * ioTemplate) const
{
	return ::mkstemp(ioTemplate);
}

int SystemProxy::close(int const iFileDescriptor) const
{
	return ::close(iFileDescriptor);
}

int SystemProxy::system(char const * iCommand) const
{
	return ::system(iCommand);
}

}
}
