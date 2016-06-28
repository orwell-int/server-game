
#pragma once

#include <string>
#include <stdint.h>

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
	bool step(
			std::string const & iCommand,
			std::string & ioReply) override;

	/// stop application
	void stopApplication() override;

	/// List all the teams present.
	///
	void listTeam(std::string & ioReply) override;

	/// add team <name>
	void addTeam(
			std::string const & iTeamName) override;

	/// remove team <name>
	void removeTeam(std::string const & iTeamName) override;

	/// get property <property> of team <name>
	void getTeam(
			std::string const & iTeamName,
			std::string const & iProperty,
			std::string & oValue) override;

	/// List all the robots present.
	///
	void listRobot(std::string & ioReply) override;

	/// add robot <name>
	void addRobot(
			std::string const & iRobotName,
			std::string const & iTeamName) override;

	/// remove robot <name>
	void removeRobot(std::string const & iRobotName) override;

	/// register robot <name>
	void registerRobot(std::string const & iRobotName) override;

	/// unregister robot <name>
	void unregisterRobot(std::string const & iRobotName) override;

	/// set property <property> of robot <name>
	void setRobot(
			std::string const & iRobotName,
			std::string const & iProperty,
			std::string const & iValue) override;

	/// get property <property> of robot <name>
	void getRobot(
			std::string const & iRobotName,
			std::string const & iProperty,
			std::string & oValue) override;

	/// List all the players present.
	///
	void listPlayer(std::string & ioReply) override;

	/// add player <name>
	void addPlayer(std::string const & iPlayerName) override;

	/// remove player <name>
	void removePlayer(std::string const & iPlayerName) override;

	/// start game
	void startGame() override;

	/// stop game
	void stopGame() override;

	/// get property <property> of game
	void getGame(
			std::string const & iProperty,
			std::string & oValue) override;

protected :

private :
	AgentProxy(AgentProxy const & iRight) = delete;
	AgentProxy & operator=(AgentProxy const & iRight) = delete;

	orwell::Application & m_application;
};
}

