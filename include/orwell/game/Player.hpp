/* This class stores the information about a Player that is connected to the server */

#pragma once

#include <string>
#include <memory>

namespace orwell
{
namespace game
{

class Robot;

class Player
{
public:
	Player(std::string const & iName);
	~Player();

	void setRobot(std::shared_ptr< Robot > aRobot);
	std::string const & getName() const;
	std::shared_ptr< Robot > getRobot() const;
	bool getHasRobot() const;
	void setAddress(std::string const & iAddress);
	std::string const & getAddress() const;

	std::string getAsString() const;
private:
	std::string m_name;
	std::weak_ptr< Robot > m_robot;
	std::string m_address;
};

}
}
