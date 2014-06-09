
#pragma once

#include <string>
#include <stdint.h>

namespace orwell
{

/// Class that gives access to commands to be run from an agent.
class IAgentProxy
{
public :
	virtual ~IAgentProxy();

	/// \return
	///  True if and only if the command was successfully parsed.
	///
	virtual bool step(std::string const & iCommand) = 0;

	/// stop application
	virtual void stopApplication() = 0;

	/// get the list of robots
	virtual void listRobot(
			std::string const & iReplyAddress,
			uint16_t const iReplyPort) = 0;

	/// add robot <name>
	virtual void addRobot(std::string const & iRobotName) = 0;

	/// remove robot <name>
	virtual void removeRobot(std::string const & iRobotName) = 0;

	/// register robot <name>
	virtual void registerRobot(std::string const & iRobotName) = 0;

	/// register robot <name>
	virtual void unregisterRobot(std::string const & iRobotName) = 0;

	/// set property <property> of robot <name>
	virtual void setRobot(
			std::string const & iRobotName,
			std::string const & iProperty,
			std::string const & iValue) = 0;

	/// get the list of players
	virtual void listPlayer(
			std::string const & iReplyAddress,
			uint16_t const iReplyPort) = 0;

	/// add player <name>
	virtual void addPlayer(std::string const & iPlayerName) = 0;

	/// remove player <name>
	virtual void removePlayer(std::string const & iPlayerName) = 0;

	/// start game
	virtual void startGame() = 0;

	/// stop game
	virtual void stopGame() = 0;

};
}

