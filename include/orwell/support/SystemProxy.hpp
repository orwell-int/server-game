#pragma once

#include "orwell/support/ISystemProxy.hpp"

namespace orwell
{
namespace support
{
class SystemProxy : public ISystemProxy
{
public :
	void mkstemp(char * ioTemplate) const override;

	int system(char const * iCommand) const override;
};
}
}
