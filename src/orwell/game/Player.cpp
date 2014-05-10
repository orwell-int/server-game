#include "orwell/game/Player.hpp"

using namespace std;

namespace orwell {
namespace game {

Player::Player(string const & iName) : m_name(iName)
{

}

Player::~Player()
{

}

void Player::setRobot(std::shared_ptr< Robot > aRobot)
{
	m_robot = aRobot;
}

string const & Player::getName() const
{
	return m_name;
}

std::shared_ptr< Robot > const Player::getRobot() const
{
	return m_robot.lock();
}


}} // namespaces

