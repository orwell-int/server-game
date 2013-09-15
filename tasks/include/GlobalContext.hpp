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
	std::map<std::string, RobotContext> & accessRobots();
	std::map<std::string, RobotContext> const & getRobots();
	std::map<std::string, PlayerContext> & accessPlayers();
	std::map<std::string, PlayerContext> const & getPlayers();

	bool addPlayer(std::string const & iName);
	// add an empty robotContext to the map, and gives it the first integer that is not already in the keys of the map as an ID.
	bool addRobot(std::string const & iName);
	bool giveRobot(std::string const & iName);

	//	robotContext get_robot(unsigned int robot_index);

private:
	std::shared_ptr< com::Sender > _publisher;
	log4cxx::LoggerPtr _logger;

	// Each connected robot has a robotContext in this map. The key is the robot ID.
	std::map<std::string, RobotContext> _robots;
	// Each connected controller has a playerContext in this map. The key is the player name.
	std::map<std::string, PlayerContext> _players;


};

}} //end namespace

