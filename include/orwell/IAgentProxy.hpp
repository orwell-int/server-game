
#pragma once

#include <string>

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

	/// add robot <name>
	virtual void addRobot(std::string const & iRobotName) = 0;

	/// remove robot <name>
	virtual void removeRobot(std::string const & iRobotName) = 0;

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

