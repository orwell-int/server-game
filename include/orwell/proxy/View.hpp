#pragma once

#include <string>

#include "orwell/proxy/IAction.hpp"

namespace orwell
{
namespace proxy
{

class View : public IAction
{
public:
	View();

	View(View const& iOther) = delete;
	View & operator = (View const& iOther) = delete;

	virtual Reply process(
			OutputMode const iOutputMode,
			std::istringstream & ioStream,
			orwell::Application & ioApplication,
			orwell::game::Game const& iGame) const;
};

}
}
