/* This class stores most of the useful data of the server. */

#pragma once

#include <map>
#include <memory>

#include "RobotContext.hpp"
#include "PlayerContext.hpp"
#include "Sender.hpp"

#include <log4cxx/logger.h>

namespace orwell {
namespace tasks {

class GlobalContext
{
public:
	// constructor
	GlobalContext(std::shared_ptr< com::Sender > iPublisher);

	// destructor
	~GlobalContext();

	std::shared_ptr< com::Sender > getPublisher();
    RobotContext & accessRobot( std::string const & iRobotName );
    std::map<std::string, RobotContext> const & getRobots();
    PlayerContext & accessPlayer( std::string const & iPlayerName );
    std::map<std::string, PlayerContext> const & getPlayers();

    //add empty PlayerContext
    bool addPlayer(std::string const & iName);
    //add empty RobotContext
    bool addRobot(std::string const & iName);

    std::string getAvailableRobot();

private:
	std::shared_ptr< com::Sender > _publisher;
	log4cxx::LoggerPtr _logger;

    // Each connected robot has a robotContext in this map. The key is the robot name.
    std::map<std::string, RobotContext> _robots;
    // Each connected controller has a playerContext in this map. The key is the player name.
	std::map<std::string, PlayerContext> _players;

};

}} //end namespace

