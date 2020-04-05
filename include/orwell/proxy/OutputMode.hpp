#pragma once

#include <iosfwd>

namespace orwell
{
namespace proxy
{

enum class OutputMode
{
	kText,
	kJson,
};

std::ostream & operator<<(std::ostream & ioStream, OutputMode const iOutputMode);

}
}
