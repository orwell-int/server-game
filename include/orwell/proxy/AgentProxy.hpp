
#pragma once

#include <string>
#include <stdint.h>

#include "orwell/proxy/IAgentProxy.hpp"

namespace orwell
{
class Application;

namespace proxy
{
enum class OutputMode;

class SwitchOutputMode;

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
	void stopApplication();

	/// add team <name>
	void addTeam(
			std::string const & iTeamName);

	/// remove team <name>
	void removeTeam(std::string const & iTeamName);

	/// set property <property> of team
	void setTeam(
			std::string const & iTeamName,
			std::string const & iProperty,
			std::string const & iValue);

	/// add robot <name>
	void addRobot(
			std::string const & iRobotName,
			std::string const & iTeamName);

	/// remove robot <name>
	void removeRobot(std::string const & iRobotName);

	/// register robot <name>
	void registerRobot(std::string const & iRobotName);

	/// unregister robot <name>
	void unregisterRobot(std::string const & iRobotName);

	/// set property <property> of robot <name>
	void setRobot(
			std::string const & iRobotName,
			std::string const & iProperty,
			std::string const & iValue);

	/// add player <name>
	void addPlayer(std::string const & iPlayerName);

	/// remove player <name>
	void removePlayer(std::string const & iPlayerName);

	/// start game
	void startGame();

	/// stop game
	void stopGame();

	/// set property <property> of game
	void setGame(
			std::string const & iProperty,
			uint32_t const iValue);

protected :
	friend class SwitchOutputMode;
	void setOutputMode(OutputMode const iOutputMode);
	OutputMode getOutputMode() const;

private :
	AgentProxy(AgentProxy const & iRight) = delete;
	AgentProxy & operator=(AgentProxy const & iRight) = delete;

	orwell::Application & m_application;
	OutputMode m_outputMode;
};

}
}
