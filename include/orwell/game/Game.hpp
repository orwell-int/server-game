#pragma once

#include <map>
#include <memory>

#include "orwell/game/Player.hpp"
#include "orwell/game/Team.hpp"

#include "server-game.pb.h"

namespace orwell
{
namespace com
{
class Sender;
}
namespace game
{
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

	bool getIsRunning() const;

	void start();
	void stop();

	//add empty PlayerContext
	bool addPlayer(std::string const & iName);

	/// Remove a player named #iName if found.
	/// \param iName
	///  The name of the player to remove.
	/// \return
	///  True if and only if the player was found and removed.
	bool removePlayer(std::string const & iName);

	//add empty RobotContext
	bool addRobot(
			std::string const & iName,
			uint16_t const aVideoRetransmissionPort,
			std::string iRobotId = "");

	/// Remove a robot named #iName if found.
	/// \param iName
	///  The name of the robot to remove.
	/// \return
	///  True if and only if the robot was found and removed.
	bool removeRobot(std::string const & iName);

	std::shared_ptr< Robot > getRobotWithoutRealRobot(
			std::string const & iTemporaryRobotId) const;
	std::shared_ptr< Robot > getRobotForPlayer(std::string const & iPlayer) const;
	std::shared_ptr<Robot> getAvailableRobot() const;
	void fillGameStateMessage( messages::GameState & oGameState);

private:
	/// \return
	///  A RobotID that is not already used.
	std::string getNewRobotId() const;
	/// True if and only if the game is running
	bool m_isRunning;
	/// Each connected robot has a robotContext in this map. The key is the robot name.
	std::map<std::string, std::shared_ptr<Robot> > m_robots;
	/// Each connected controller has a playerContext in this map. The key is the player name.
	std::map< std::string, std::shared_ptr< Player > > m_players;
	/// Each connected controller has a playerContext in this map. The key is the team name.
	std::map<std::string, Team> m_teams;
	/// Stores the temporary robots ID sent by the different robots in case
	/// they send the same value again to send back the same information
	/// the following times.
	mutable std::map< std::string, std::string > m_registeredRobots;
	/// stores the temp files containing the pids of the webservers, to kill them later
	std::vector<std::string> m_tmpFiles;
};

}
} //end namespace

