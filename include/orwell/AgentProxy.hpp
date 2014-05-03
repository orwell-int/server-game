
#pragma once

#include <string>

#include "orwell/IAgentProxy.hpp"

namespace orwell
{
class Application;

/// Class that gives access to commands to be run from an agent.
class AgentProxy : public IAgentProxy
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

	/// List all the robots present.
	/// The reply is sent to <iReplyAddress>:<iReplyPort>.
	///
	void listRobot(
			std::string const & iReplyAddress,
			uint16_t const iReplyPort);

	/// add robot <name>
	void addRobot(std::string const & iRobotName);

	/// remove robot <name>
	void removeRobot(std::string const & iRobotName);

	/// List all the players present.
	/// The reply is sent to <iReplyAddress>:<iReplyPort>.
	///
	void listPlayer(
			std::string const & iReplyAddress,
			uint16_t const iReplyPort);

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

