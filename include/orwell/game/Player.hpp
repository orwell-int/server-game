/* This class stores the information about a Player that is connected to the server */

#pragma once

#include <string>
#include <map>
#include <Robot.hpp>

namespace orwell {
namespace game {

class Player
{
	public:
		Player(std::string const & iName);
		~Player();

        void setRobot(std::string const & iName);
		std::string getName();
		std::string getRobot();

    private:
        std::string _name;
        std::string _robot;
};

}} //end namespace
