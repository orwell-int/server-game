#pragma once

#include "orwell/support/ISystemProxy.hpp"

namespace orwell
{
namespace support
{
class SystemProxy : public ISystemProxy
{
public :
	int mkstemp(char * ioTemplate) const override;

	int close(int const iFileDescriptor) const override;

	int system(char const * iCommand) const override;
};
}
}
