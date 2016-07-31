#pragma once

namespace orwell
{
namespace support
{
class ISystemProxy
{
public :
	virtual ~ISystemProxy();

	virtual void mkstemp(char * ioTemplate) const = 0;

	virtual int system(char const * iCommand) const = 0;
};
}
}
