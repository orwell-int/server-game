#pragma once

#include <map>
#include <string>

#include "orwell/proxy/Reply.hpp"

namespace orwell
{
class Application;
namespace game
{
class Game;
}
}

namespace orwell
{
namespace proxy
{

enum class OutputMode;

class IAction;

typedef std::map< std::string, std::reference_wrapper< IAction const > > ActionMap;

class IAction
{
public:
	virtual ~IAction();

	ActionMap::value_type get() const;

	std::string const getName() const;

	virtual Reply process(
			OutputMode const iOutputMode,
			std::istringstream & ioStream,
			orwell::Application & ioApplication,
			orwell::game::Game const& iGame) const = 0;
	IAction(IAction const& iOther) = delete;
	IAction& operator =(IAction const& iOther) = delete;

protected:
	IAction(std::string const& iName);
private:
	std::string const m_name;
};

}
}
