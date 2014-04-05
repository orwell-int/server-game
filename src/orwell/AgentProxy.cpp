#include "orwell/AgentProxy.hpp"

#include "orwell/Application.hpp"

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
	if ("start" == iAction)
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

void AgentProxy::addRobot(
		std::string const & iRobotName)
{
	ORWELL_LOG_INFO("add robot " << iRobotName);
}

void AgentProxy::removeRobot(
		std::string const & iRobotName)
{
	ORWELL_LOG_INFO("remove robot " << iRobotName);
}

void AgentProxy::addPlayer(
		std::string const & iPlayerName)
{
	ORWELL_LOG_INFO("add player " << iPlayerName);
}

void AgentProxy::removePlayer(
		std::string const & iPlayerName)
{
	ORWELL_LOG_INFO("remove player " << iPlayerName);
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

