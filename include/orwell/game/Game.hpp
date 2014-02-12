#pragma once

#include <map>
#include <memory>
#include <log4cxx/logger.h>

#include "orwell/game/Robot.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Team.hpp"

namespace orwell {
namespace com {
class Sender;
}
namespace game {

class Game
{
public:
	Game(std::shared_ptr< com::Sender > iPublisher);
	~Game();

	std::shared_ptr< com::Sender > getPublisher();

    Robot & accessRobot( std::string const & iRobotName );
    std::map<std::string, Robot> const & getRobots();

    Player & accessPlayer( std::string const & iPlayerName );
    std::map<std::string, Player> const & getPlayers();

    void start() { _isRunning = true; }
    void stop() { _isRunning = false; }

    //add empty PlayerContext
    bool addPlayer(std::string const & iName);
    //add empty RobotContext
    bool addRobot(std::string const & iName);

    std::string getAvailableRobot();

private:
	std::shared_ptr< com::Sender > _publisher;
	log4cxx::LoggerPtr _logger;

	// Is the game started and running or not ?
	bool _isRunning;
    // Each connected robot has a robotContext in this map. The key is the robot name.
    std::map<std::string, Robot> _robots;
    // Each connected controller has a playerContext in this map. The key is the player name.
	std::map<std::string, Player> _players;
    // Each connected controller has a playerContext in this map. The key is the player name.
	std::map<std::string, Team> _teams;

};

}} //end namespace

