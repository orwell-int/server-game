#include "orwell/proxy/IAction.hpp"

#include <functional>
#include <iostream>

namespace orwell
{
namespace proxy
{

IAction::~IAction()
{
}

ActionMap::value_type IAction::get() const
{
	return ActionMap::value_type{ m_name, std::reference_wrapper< IAction const >(*this) };
}

std::string const IAction::getName() const
{
	return m_name;
}

IAction::IAction(std::string const& iName)
	: m_name(iName)
{
}

}
}
