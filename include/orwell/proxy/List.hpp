#pragma once

#include <string>

#include "orwell/proxy/IAction.hpp"

namespace orwell
{
namespace proxy
{

class List : public IAction
{
public:
	List();

	List(List const& iOther) = delete;
	List & operator = (List const& iOther) = delete;

	virtual Reply process(
			OutputMode const iOutputMode,
			std::istringstream & ioStream,
			orwell::Application & ioApplication,
			orwell::game::Game const& iGame) const;
};

}
}
