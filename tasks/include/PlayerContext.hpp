/* This class stores the information about a Player that is connected to the server */

#pragma once

#include <string>
#include <map>
#include <RobotContext.hpp>

namespace orwell {
namespace tasks {

class PlayerContext
{
	public:
		PlayerContext(std::string const & iName);
		~PlayerContext();

        void setRobot(std::string const & iName);
		std::string getName();
		std::string getRobot();

		bool giveRobot(std::map<std::string, RobotContext> & iRobotMap);

    private:
        std::string _name;
        std::string _robot;
};

}} //end namespace
