#pragma once

#include <string>

#include "orwell/proxy/IAction.hpp"

namespace orwell
{
namespace proxy
{

class Get : public IAction
{
public:
	Get();

	Get(Get const& iOther) = delete;
	Get & operator = (Get const& iOther) = delete;

	virtual Reply process(
			OutputMode const iOutputMode,
			std::istringstream & ioStream,
			orwell::Application & ioApplication,
			orwell::game::Game const& iGame) const;
};

}
}
