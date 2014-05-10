#pragma once

#include <map>
#include <memory>

#include "orwell/game/Player.hpp"
#include "orwell/game/Team.hpp"

#include "server-game.pb.h"

namespace orwell {
namespace com {
class Sender;
}
namespace game {
class Robot;

class Game
{
public:
	Game();
	~Game();

//	std::shared_ptr< com::Sender > getPublisher();

	std::shared_ptr<Robot> accessRobot(std::string const & iRobotName);
	std::map<std::string, std::shared_ptr<Robot> > const & getRobots();


	std::shared_ptr< Player > accessPlayer( std::string const & iPlayerName );
	std::map< std::string, std::shared_ptr< Player > > const & getPlayers();

	void start() { m_isRunning = true; }
	void stop() { m_isRunning = false; }

	//add empty PlayerContext
	bool addPlayer(std::string const & iName);
	//add empty RobotContext
	bool addRobot(std::string const & iName);
	/// Remove a robot named #iName if found.
	/// \param iName
	///  The name of the robot to remove.
	/// \return
	///  True if and only if the robot was found and removed.
	bool removeRobot(std::string const & iName);

	std::string const getRobotNameForPlayer(std::string const & iPlayer) const;
	std::shared_ptr<Robot> getAvailableRobot() const;
	void fillGameStateMessage( messages::GameState & oGameState);

private:
	// Is the game started and running or not ?
	bool m_isRunning;
	// Each connected robot has a robotContext in this map. The key is the robot name.
	std::map<std::string, std::shared_ptr<Robot> > m_robots;
	// Each connected controller has a playerContext in this map. The key is the player name.
	std::map< std::string, std::shared_ptr< Player > > m_players;
	// Each connected controller has a playerContext in this map. The key is the team name.
	std::map<std::string, Team> m_teams;

};

}} //end namespace
