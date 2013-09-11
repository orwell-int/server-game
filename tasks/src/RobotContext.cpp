#include "RobotContext.hpp"
#include <iostream>

using namespace std;

namespace orwell {
namespace tasks {

RobotContext::RobotContext(string const & iName) : _name(iName)
{

}

RobotContext::~RobotContext()
{

}

void RobotContext::setControlledBy(string const & iName)
{
    _controlledBy = iName;
}

string const & RobotContext::getName() const
{
    return _name;
}

string const &  RobotContext::getControlledBy() const
{
    return _controlledBy;
}


}} // namespaces


