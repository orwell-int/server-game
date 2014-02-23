#pragma once

#include <map>
#include <memory>
#include <log4cxx/logger.h>

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


	Player & accessPlayer( std::string const & iPlayerName );
	std::map<std::string, Player> const & getPlayers();

	void start() { _isRunning = true; }
	void stop() { _isRunning = false; }

	//add empty PlayerContext
	bool addPlayer(std::string const & iName);
	//add empty RobotContext
	bool addRobot(std::string const & iName);

	std::string const getRobotNameForPlayer(std::string const & iPlayer) const;
	std::shared_ptr<Robot> getAvailableRobot() const;
	void fillGameStateMessage( messages::GameState & oGameState);

private:
	log4cxx::LoggerPtr _logger;

	// Is the game started and running or not ?
	bool _isRunning;
	// Each connected robot has a robotContext in this map. The key is the robot name.
	std::map<std::string, std::shared_ptr<Robot> > _robots;
	// Each connected controller has a playerContext in this map. The key is the player name.
	std::map<std::string, Player> _players;
	// Each connected controller has a playerContext in this map. The key is the team name.
	std::map<std::string, Team> _teams;

};

}} //end namespace
