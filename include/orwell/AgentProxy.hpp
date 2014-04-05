
#pragma once

#include <string>

namespace orwell {
class Application;

/// Class that gives access to commands to be run from an agent.
class AgentProxy
{
public :
	/// \param ioApplication
	///  The application the agent will control.
	AgentProxy(orwell::Application & ioApplication);

	/// \return
	///  True if and only if the command was successfully parsed.
	bool step(std::string const & iCommand);

	/// stop application
	void stopApplication();

	/// add robot <name>
	void addRobot(std::string const & iRobotName);

	/// remove robot <name>
	void removeRobot(std::string const & iRobotName);

	/// add player <name>
	void addPlayer(std::string const & iPlayerName);

	/// remove player <name>
	void removePlayer(std::string const & iPlayerName);

	/// start game
	void startGame();

	/// stop game
	void stopGame();
protected :

private :
	AgentProxy(AgentProxy const & iRight) = delete;
	AgentProxy & operator=(AgentProxy const & iRight) = delete;

	orwell::Application & m_application;
};
}

