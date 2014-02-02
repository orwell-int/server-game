#include "Robot.hpp"
#include <iostream>

#include "robot.pb.h"

using namespace std;
using orwell::messages::RobotState;

namespace orwell {
namespace game {

Robot::Robot(string const & iName) : _name(iName)
{

}

Robot::~Robot()
{

}

void Robot::setPlayerName(string const & iName)
{
    _playerName = iName;
}

string const & Robot::getName() const
{
    return _name;
}

string const &  Robot::getPlayerName() const
{
    return _playerName;
}

void fillRobotStateMessage( messages::RobotState & oMessage )
{
	//todo
	//those are just bullshit hardcoded values to have the syntax
	oMessage.set_life(1);
	oMessage.mutable_move()->set_left(0);
	oMessage.mutable_move()->set_right(0);
	oMessage.set_active(true);
}



}} // namespaces


