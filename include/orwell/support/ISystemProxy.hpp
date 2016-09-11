#pragma once

namespace orwell
{
namespace support
{
class ISystemProxy
{
public :
	virtual ~ISystemProxy();

	virtual int mkstemp(char * ioTemplate) const = 0;

	virtual int close(int const iFileDescriptor) const = 0;

	virtual int system(char const * iCommand) const = 0;
};
}
}
