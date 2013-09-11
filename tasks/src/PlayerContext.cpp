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

bool PlayerContext::giveRobot(std::map<std::string, RobotContext> & iRobotMap)
{
    //search for the first robot which is not already associated to a player
    std::map<std::string, RobotContext>::iterator aIterOnRobots;
    aIterOnRobots = iRobotMap.begin();
    while ( !aIterOnRobots->second.getControlledBy().empty() )
    {
        ++aIterOnRobots;
    }

    if (aIterOnRobots == iRobotMap.end())
    {
  //      LOG4CXX_WARN(_logger, "Cannot associate robot : no more free robot");
        return false;
    }
    else
    {
        //mark the robot as controlled by this player, mark the player as controlling the robot
        aIterOnRobots->second.setControlledBy(_name);
        _robot = aIterOnRobots->first;
    //    LOG4CXX_INFO(_logger, "Player " << iName << " now controls robot " << aIterOnRobots->first);
        return true;
    }

}


}} // namespaces


