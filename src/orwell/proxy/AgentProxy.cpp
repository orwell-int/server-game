#include "orwell/proxy/AgentProxy.hpp"

#include "orwell/Application.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Robot.hpp"

#include "orwell/proxy/Tokens.hpp"
#include "orwell/proxy/List.hpp"
#include "orwell/proxy/View.hpp"
#include "orwell/proxy/Get.hpp"
#include "orwell/proxy/OutputMode.hpp"
#include "orwell/proxy/SimpleTeam.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <sstream>
#include <functional>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orwell
{
namespace proxy
{

static const List GlobalList;
static const View GlobalView;
static const Get GlobalGet;

static const ActionMap GlobalActionMap =
{
	GlobalList.get(),
	GlobalView.get(),
	GlobalGet.get()
};

class SwitchOutputMode
{
public:
	explicit SwitchOutputMode(OutputMode const iOutputMode, orwell::proxy::AgentProxy & ioAgentProxy)
		: m_agentProxy(ioAgentProxy)
		, m_previousOutputMode(m_agentProxy.getOutputMode())
		, m_newOutputMode(iOutputMode)
		, m_restored(false)
	{
		m_agentProxy.setOutputMode(m_newOutputMode);
	}

	~SwitchOutputMode()
	{
		restore();
	}

	void restore()
	{
		if (not m_restored)
		{
			m_agentProxy.setOutputMode(m_previousOutputMode);
			m_restored = true;
		}
	}

private:
	orwell::proxy::AgentProxy & m_agentProxy;
	OutputMode const m_previousOutputMode;
	OutputMode const m_newOutputMode;
	bool m_restored;
};

AgentProxy::AgentProxy(orwell::Application & ioApplication)
	: m_application(ioApplication)
	, m_outputMode(OutputMode::kText)
{
}

static void Dispatch(
		std::istringstream & ioStream,
		std::function< void() > iFunction,
		bool & oResult,
		std::string & ioReply)
{
	oResult = ioStream.eof();
	if (oResult)
	{
		iFunction();
		ORWELL_LOG_DEBUG("Dispatch OK");
		ioReply = "OK";
	}
}

static void DispatchArgument(
		std::istringstream & ioStream,
		std::function< void(std::string const &) > iFunction,
		bool & oResult,
		std::string & ioReply)
{
	std::string const aArg = ReadName(ioStream);
	oResult = ioStream.eof();
	if (oResult)
	{
		iFunction(aArg);
		ORWELL_LOG_DEBUG("DispatchArgument OK");
		ioReply = "OK";
	}
}

bool AgentProxy::step(
		std::string const & iCommand,
		std::string & ioReply)
{
	ORWELL_LOG_DEBUG("AgentProxy::step(" << iCommand << ")");
	ioReply.clear();
	bool aResult = false;
	std::string aAction;
	using std::placeholders::_1;
	std::istringstream aStream(iCommand);
	aStream >> aAction;
	for (auto aPair: GlobalActionMap)
	{
		ORWELL_LOG_DEBUG("GlobalActionMap has key '" << aPair.first << "' -> name = '" << aPair.second.get().getName() << "'");
		ORWELL_LOG_DEBUG("Check get.first = '" << aPair.second.get().get().first << "'");
	}
	ActionMap::const_iterator aActionPair = GlobalActionMap.find(aAction);
	if (GlobalActionMap.end() != aActionPair)
	{
		ORWELL_LOG_DEBUG("found '" << aAction << "' in GlobalActionMap");
		Reply aReply = aActionPair->second.get().process(
				m_outputMode,
				aStream,
				m_application,
				m_application.accessServer()->accessContext());
		aResult = aReply;
		ioReply = (std::string)aReply;
	}
	else
	{
		ORWELL_LOG_DEBUG("Could NOT find '" << aAction << "' in GlobalActionMap");
		if (OutputMode::kText == m_outputMode)
		{
			if ("start" == aAction)
			{
				std::string aObject;
				aStream >> aObject;
				if ("game" == aObject)
				{
					Dispatch(
							aStream,
							std::bind(&AgentProxy::startGame, this),
							aResult,
							ioReply);
				}
			}
			else if ("stop" == aAction)
			{
				std::string aObject;
				aStream >> aObject;
				if ("application" == aObject)
				{
					Dispatch(
							aStream,
							std::bind(&AgentProxy::stopApplication, this),
							aResult,
							ioReply);
				}
				else if ("game" == aObject)
				{
					Dispatch(
							aStream,
							std::bind(&AgentProxy::stopGame, this),
							aResult,
							ioReply);
				}
			}
			else if ("add" == aAction)
			{
				std::string aObject;
				aStream >> aObject;
				if ("team" == aObject)
				{
					DispatchArgument(
							aStream,
							std::bind(&AgentProxy::addTeam, this, _1),
							aResult,
							ioReply);
				}
				else if ("robot" == aObject)
				{
					std::string const aName = ReadName(aStream);
					std::string const aTeam = ReadName(aStream);
					Dispatch(
							aStream,
							std::bind(&AgentProxy::addRobot, this, aName, aTeam),
							aResult,
							ioReply);
				}
				else if ("player" == aObject)
				{
					DispatchArgument(
							aStream,
							std::bind(&AgentProxy::addPlayer, this, _1),
							aResult,
							ioReply);
				}
			}
			else if ("remove" == aAction)
			{
				std::string aObject;
				aStream >> aObject;
				if ("team" == aObject)
				{
					DispatchArgument(
							aStream,
							std::bind(&AgentProxy::removeTeam, this, _1),
							aResult,
							ioReply);
				}
				else if ("robot" == aObject)
				{
					DispatchArgument(
							aStream,
							std::bind(&AgentProxy::removeRobot, this, _1),
							aResult,
							ioReply);
				}
				else if ("player" == aObject)
				{
					DispatchArgument(
							aStream,
							std::bind(&AgentProxy::removePlayer, this, _1),
							aResult,
							ioReply);
				}
			}
			else if ("register" == aAction)
			{
				std::string aObject;
				aStream >> aObject;
				if ("robot" == aObject)
				{
					DispatchArgument(
							aStream,
							std::bind(&AgentProxy::registerRobot, this, _1),
							aResult,
							ioReply);
				}
			}
			else if ("unregister" == aAction)
			{
				std::string aObject;
				aStream >> aObject;
				if ("robot" == aObject)
				{
					DispatchArgument(
							aStream,
							std::bind(&AgentProxy::unregisterRobot, this, _1),
							aResult,
							ioReply);
				}
			}
			else if ("set" == aAction)
			{
				std::string aObject;
				aStream >> aObject;
				if ("game" == aObject)
				{
					std::string aProperty;
					aStream >> aProperty;
					uint32_t aValue;
					aStream >> aValue;
					Dispatch(
							aStream,
							std::bind(&AgentProxy::setGame, this, aProperty, aValue),
							aResult,
							ioReply);
				}
				else if ("robot" == aObject)
				{
					std::string const aName = ReadName(aStream);
					std::string aProperty;
					aStream >> aProperty;
					std::string aValue;
					aStream >> aValue;
					Dispatch(
							aStream,
							std::bind(&AgentProxy::setRobot, this, aName, aProperty, aValue),
							aResult,
							ioReply);
				}
				else if ("team" == aObject)
				{
					std::string const aName = ReadName(aStream);
					std::string aProperty;
					aStream >> aProperty;
					std::string aValue;
					aStream >> aValue;
					Dispatch(
							aStream,
							std::bind(&AgentProxy::setTeam, this, aName, aProperty, aValue),
							aResult,
							ioReply);
				}
			}
			else if ("ping" == aAction)
			{
				ioReply = "pong";
				aResult = true;
			}
			else if ("json" == aAction)
			{
				SwitchOutputMode aSentinel(OutputMode::kJson, *this);
				std::string const aNewCommand(iCommand.substr(std::string("json ").size()));
				ORWELL_LOG_DEBUG("json stripped command: '" << aNewCommand << "'");
				aResult = step(aNewCommand, ioReply);
				aSentinel.restore();
			}
		}
		ORWELL_LOG_DEBUG("Parsing result = " << aResult);
		if (not aResult)
		{
			ORWELL_LOG_WARN("Command not parsed sucessfully: '" << iCommand << "'");
			if (ioReply.empty())
			{
				switch (m_outputMode)
				{
					case OutputMode::kText:
					{
						ioReply = "KO";
						break;
					}
					case OutputMode::kJson:
					{
						json aJson;
						ioReply = aJson.dump();
						break;
					}
				}
			}
		}
	}
	return aResult;
}

void AgentProxy::stopApplication()
{
	ORWELL_LOG_INFO("stop application");
	m_application.stop();
}

void AgentProxy::addTeam(std::string const & iTeamName)
{
	ORWELL_LOG_INFO("add team " << iTeamName);
	m_application.accessServer()->accessContext().addTeam(
			iTeamName);
}

void AgentProxy::removeTeam(std::string const & iTeamName)
{
	ORWELL_LOG_INFO("remove team " << iTeamName);
	m_application.accessServer()->accessContext().removeTeam(iTeamName);
}

void AgentProxy::setTeam(
		std::string const & iTeamName,
		std::string const & iProperty,
		std::string const & iValue)
{
	ORWELL_LOG_INFO("get team " << iTeamName << " " << iProperty);
	try
	{
		orwell::game::Team & aTeam =
				m_application.accessServer()->accessContext().accessTeam(iTeamName);
		if (orwell::game::Team::GetNeutralTeam() == aTeam)
		{
			ORWELL_LOG_WARN("Invalid team name '" << iTeamName << "'");
			return;
		}
		if ("score" == iProperty)
		{
			aTeam.setScore(boost::lexical_cast< uint32_t >(iValue));
		}
		else
		{
			ORWELL_LOG_WARN("Unknown property for a team: '" << iProperty << "'");
		}
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
}

void AgentProxy::addRobot(
		std::string const & iRobotName,
		std::string const & iTeamName)
{
	ORWELL_LOG_INFO("add robot " << iRobotName);
	m_application.accessServer()->accessContext().addRobot(
			iRobotName,
			iTeamName,
			m_application.popPort(),
			m_application.popPort());
}

void AgentProxy::removeRobot(std::string const & iRobotName)
{
	ORWELL_LOG_INFO("remove robot " << iRobotName);
	m_application.accessServer()->accessContext().removeRobot(iRobotName);
}

void AgentProxy::registerRobot(std::string const & iRobotName)
{
	ORWELL_LOG_INFO("register robot " << iRobotName);
	try
	{
		m_application.accessServer()->accessContext().accessRobot(iRobotName)
			->setHasRealRobot(true);
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
}

void AgentProxy::unregisterRobot(std::string const & iRobotName)
{
	ORWELL_LOG_INFO("unregister robot " << iRobotName);
	try
	{
		m_application.accessServer()->accessContext().accessRobot(iRobotName)
			->setHasRealRobot(false);
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
}

void AgentProxy::setRobot(
		std::string const & iRobotName,
		std::string const & iProperty,
		std::string const & iValue)
{
	ORWELL_LOG_INFO("set robot '" << iRobotName <<
		"' '" << iProperty << "' '" << iValue << "'");
	try
	{
		std::shared_ptr< orwell::game::Robot > aRobot =
			m_application.accessServer()->accessContext().accessRobot(iRobotName);
		if ("video_url" == iProperty)
		{
			aRobot->setVideoUrl(iValue);
		}
		else
		{
			ORWELL_LOG_WARN("Unknown property for a robot: '" << iProperty << "'");
		}
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
}

void AgentProxy::addPlayer(std::string const & iPlayerName)
{
	ORWELL_LOG_INFO("add player " << iPlayerName);
	m_application.accessServer()->accessContext().addPlayer(iPlayerName);
}

void AgentProxy::removePlayer(std::string const & iPlayerName)
{
	ORWELL_LOG_INFO("remove player " << iPlayerName);
	m_application.accessServer()->accessContext().removePlayer(iPlayerName);
}

void AgentProxy::startGame()
{
	ORWELL_LOG_INFO("start game");
	m_application.accessServer()->accessContext().start(true);
}

void AgentProxy::stopGame()
{
	ORWELL_LOG_INFO("stop game");
	m_application.accessServer()->accessContext().stop();
}

void AgentProxy::setGame(
		std::string const & iProperty,
		uint32_t const iValue)
{
	ORWELL_LOG_INFO("set game '" << iProperty << "' '" << iValue << "'");
	try
	{
		if ("duration" == iProperty)
		{
			m_application.accessServer()->accessContext().setDuration(
				boost::posix_time::seconds(iValue));
		}
		else
		{
			ORWELL_LOG_WARN("Unknown property for the game: '" << iProperty << "'");
		}
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
}

void AgentProxy::setOutputMode(OutputMode const iOutputMode)
{
	ORWELL_LOG_DEBUG("Set output mode to " << iOutputMode);
	m_outputMode = iOutputMode;
}

OutputMode AgentProxy::getOutputMode() const
{
	return m_outputMode;
}

// protected

}
}
