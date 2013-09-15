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

void RobotContext::setPlayerName(string const & iName)
{
    _playerName = iName;
}

string const & RobotContext::getName() const
{
    return _name;
}

string const &  RobotContext::getPlayerName() const
{
    return _playerName;
}


}} // namespaces


