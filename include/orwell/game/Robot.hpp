/* This class stores the information about a robot that is connected to the server */

#pragma once

#include <string>

#include <memory>

namespace orwell {
namespace messages {
class RobotState;
}
namespace game
{
class Player;

class Robot
{
public:
	Robot(std::string const & iName);
	~Robot();

	void setPlayer(std::shared_ptr< Player > const iPlayer);
	std::shared_ptr< Player > const getPlayer() const;

	//void setPlayerName(std::string const & iName);

	std::string const & getName() const;

	//std::string const & getPlayerName() const;

	void fillRobotStateMessage( messages::RobotState & oMessage );

private:
	std::string m_name;
	std::weak_ptr< Player > m_player;
};

}} //end namespace

