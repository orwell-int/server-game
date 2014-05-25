#include "orwell/AgentProxy.hpp"

#include "orwell/Application.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/com/Url.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <sstream>
#include <functional>

namespace orwell
{

AgentProxy::AgentProxy(orwell::Application & ioApplication)
	: m_application(ioApplication)
{
}

void echo(std::string const & iMessage)
{
	ORWELL_LOG_DEBUG("iMessage = '" << iMessage << "'");
}

static void Dispatch(
		std::istringstream & ioStream,
		std::function< void() > iFunction,
		bool & oResult)
{
	oResult = ioStream.eof();
	if (oResult)
	{
		iFunction();
	}
}

static void DispatchArgument(
		std::istringstream & ioStream,
		std::function< void(std::string const &) > iFunction,
		bool & oResult)
{
	std::string aArg;
	ioStream >> aArg;
	oResult = ioStream.eof();
	if (oResult)
	{
		iFunction(aArg);
	}
}

bool AgentProxy::step(std::string const & iCommand)
{
	bool aResult = false;
	std::string iAction;
	using std::placeholders::_1;
	std::istringstream aStream(iCommand);
	aStream >> iAction;
	if ("list" == iAction)
	{
		std::string aObject;
		aStream >> aObject;
		std::string aAddress;
		aStream >> aAddress;
		uint16_t aPort;
		aStream >> aPort;
		if ("player" == aObject)
		{
			Dispatch(
				aStream,
				std::bind(&AgentProxy::listPlayer, this, aAddress, aPort),
				aResult);
		}
		else if ("robot" == aObject)
		{
			Dispatch(
				aStream,
				std::bind(&AgentProxy::listRobot, this, aAddress, aPort),
				aResult);
		}
	}
	else if ("start" == iAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("game" == aObject)
		{
			Dispatch(aStream, std::bind(&AgentProxy::startGame, this), aResult);
		}
	}
	else if ("stop" == iAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("application" == aObject)
		{
			Dispatch(aStream, std::bind(&AgentProxy::stopApplication, this), aResult);
		}
		else if ("game" == aObject)
		{
			Dispatch(aStream, std::bind(&AgentProxy::stopGame, this), aResult);
		}
	}
	else if ("add" == iAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("robot" == aObject)
		{
			DispatchArgument(aStream, std::bind(&AgentProxy::addRobot, this, _1), aResult);
		}
		else if ("player" == aObject)
		{
			DispatchArgument(aStream, std::bind(&AgentProxy::addPlayer, this, _1), aResult);
		}
	}
	else if ("remove" == iAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("robot" == aObject)
		{
			DispatchArgument(aStream, std::bind(&AgentProxy::removeRobot, this, _1), aResult);
		}
		else if ("player" == aObject)
		{
			DispatchArgument(aStream, std::bind(&AgentProxy::removePlayer, this, _1), aResult);
		}
	}
	ORWELL_LOG_DEBUG("Parsing result = " << aResult);
	if (not aResult)
	{
		ORWELL_LOG_WARN("Command not parsed sucessfully: '" << iCommand << "'");
	}
	return aResult;
}

void AgentProxy::stopApplication()
{
	ORWELL_LOG_INFO("stop application");
	m_application.stop();
}

void AgentProxy::listRobot(
		std::string const & iReplyAddress,
		uint16_t const iReplyPort)
{
	ORWELL_LOG_INFO("list robot " << iReplyAddress << " " << iReplyPort);
	std::map< std::string, std::shared_ptr< orwell::game::Robot > > aRobots =
		m_application.accessServer()->accessContext().getRobots();
	std::string aReply = "Robots:\n";
	for (auto const & aPair : aRobots)
	{
		aReply += "\t" + aPair.first + " -> ";
		aReply += "name = " + aPair.second->getName() + " ; ";
		bool aHasPlayer(aPair.second->getPlayer());
		if (aHasPlayer)
		{
			aReply += "player = " + aPair.second->getPlayer()->getName() + "\n";
		}
		else
		{
			aReply += "player = \n";
		}
	}
	orwell::com::Url aUrl;
	aUrl.setHost(iReplyAddress);
	aUrl.setPort(iReplyPort);
	m_application.accessServer()->push(aUrl.toString(), aReply);
}


void AgentProxy::addRobot(
		std::string const & iRobotName)
{
	ORWELL_LOG_INFO("add robot " << iRobotName);
	m_application.accessServer()->accessContext().addRobot(iRobotName);
}

void AgentProxy::removeRobot(
		std::string const & iRobotName)
{
	ORWELL_LOG_INFO("remove robot " << iRobotName);
	m_application.accessServer()->accessContext().removeRobot(iRobotName);
}

void AgentProxy::listPlayer(
		std::string const & iReplyAddress,
		uint16_t const iReplyPort)
{
	ORWELL_LOG_INFO("list player " << iReplyAddress << " " << iReplyPort);
	std::map< std::string, std::shared_ptr< orwell::game::Player > > aPlayers =
		m_application.accessServer()->accessContext().getPlayers();
	std::string aReply = "Players:\n";
	for (auto const & aPair : aPlayers)
	{
		aReply += "\t" + aPair.first + " -> ";
		aReply += "name = " + aPair.second->getName() + " ; ";
		bool aHasRobot(aPair.second->getRobot());
		if (aHasRobot)
		{
			aReply += "robot = " + aPair.second->getRobot()->getName() + "\n";
		}
		else
		{
			aReply += "robot = \n";
		}
	}
	orwell::com::Url aUrl;
	aUrl.setHost(iReplyAddress);
	aUrl.setPort(iReplyPort);
	m_application.accessServer()->push(aUrl.toString(), aReply);
}


void AgentProxy::addPlayer(
		std::string const & iPlayerName)
{
	ORWELL_LOG_INFO("add player " << iPlayerName);
	m_application.accessServer()->accessContext().addPlayer(iPlayerName);
}

void AgentProxy::removePlayer(
		std::string const & iPlayerName)
{
	ORWELL_LOG_INFO("remove player " << iPlayerName);
	m_application.accessServer()->accessContext().removePlayer(iPlayerName);
}

void AgentProxy::startGame()
{
	ORWELL_LOG_INFO("start game");
}

void AgentProxy::stopGame()
{
	ORWELL_LOG_INFO("stop game");
}

// protected

}

