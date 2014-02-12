/* This class stores the information about a robot that is connected to the server */

#pragma once

#include <string>

namespace orwell {
namespace messages {
class RobotState;
}
namespace game {

class Robot
{
	public:
		Robot(std::string const & iName);
		~Robot();

        void setPlayerName(std::string const & iName);
		std::string const &  getName() const;
		std::string const &  getPlayerName() const;

		void fillRobotStateMessage( messages::RobotState & oMessage );

    private:
        std::string _name;
        std::string _playerName;
};

}} //end namespace

