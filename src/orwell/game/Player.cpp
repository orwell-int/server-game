#include "orwell/game/Player.hpp"

#include "orwell/game/Robot.hpp"

namespace orwell
{
namespace game
{

Player::Player(std::string const & iName)
	: m_name(iName)
{
}

Player::~Player()
{
}

void Player::setRobot(std::shared_ptr< Robot > aRobot)
{
	m_robot = aRobot;
}

std::string const & Player::getName() const
{
	return m_name;
}

std::shared_ptr< Robot > Player::getRobot() const
{
	return m_robot.lock();
}

bool Player::getHasRobot() const
{
	return not m_robot.expired();
}

void Player::setAddress(std::string const & iAddress)
{
	m_address = iAddress;
}

std::string const & Player::getAddress() const
{
	return m_address;
}

std::string Player::getAsString() const
{
	std::string aReply = "Player ";
	aReply += "name = " + m_name + " ; ";
	aReply += "address = " + m_address + " ; ";
	auto const& aRobot = getRobot();
	bool aHasRobot(aRobot);
	aReply += "robot = ";
	if (aHasRobot)
	{
		aReply += aRobot->getName();
	}
	return aReply;
}

}
}
