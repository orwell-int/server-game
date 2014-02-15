#include "Player.hpp"

using namespace std;

namespace orwell {
namespace game {

Player::Player(string const & iName) : _name(iName)
{

}

Player::~Player()
{

}

void Player::setRobot(string const & iName)
{
    _robot = iName;
}

string Player::getName()
{
    return _name;
}

string Player::getRobot()
{
    return _robot;
}


}} // namespaces


