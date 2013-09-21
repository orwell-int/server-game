#include "PlayerContext.hpp"

using namespace std;

namespace orwell {
namespace tasks {

PlayerContext::PlayerContext(string const & iName) : _name(iName)
{

}

PlayerContext::~PlayerContext()
{

}

void PlayerContext::setRobot(string const & iName)
{
    _robot = iName;
}

string PlayerContext::getName()
{
    return _name;
}

string PlayerContext::getRobot()
{
    return _robot;
}


}} // namespaces


