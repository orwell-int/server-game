#include "orwell/AgentProxy.hpp"

#include "orwell/Application.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Robot.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <sstream>
#include <functional>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orwell
{

enum class OutputMode
{
	kText,
	kJson,
};

class SwitchOutputMode
{
public:
	explicit SwitchOutputMode(OutputMode const iOutputMode, orwell::AgentProxy & ioAgentProxy)
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
	orwell::AgentProxy & m_agentProxy;
	OutputMode const m_previousOutputMode;
	OutputMode const m_newOutputMode;
	bool m_restored;
};

std::ostream & operator<<(std::ostream & ioStream, OutputMode const iOutputMode)
{
	switch (iOutputMode)
	{
		case OutputMode::kText:
		{
			ioStream << "text";
			break;
		}
		case OutputMode::kJson:
		{
			ioStream << "json";
			break;
		}
	}
	return ioStream;
}

struct SimpleRobot
{
	SimpleRobot(std::shared_ptr< game::Robot > const iRobot)
		: m_name(iRobot->getName())
		, m_player(iRobot->getHasPlayer() ? iRobot->getPlayer()->getName() : "")
		, m_registered(iRobot->getHasRealRobot())
		, m_team(iRobot->getTeam().getName())
		, m_videoUrl(iRobot->getVideoUrl())
	{
	}

	SimpleRobot(SimpleRobot const & iOther)
		: m_name(iOther.m_name)
		, m_player(iOther.m_player)
		, m_registered(iOther.m_registered)
		, m_team(iOther.m_team)
		, m_videoUrl(iOther.m_videoUrl)
	{
	}

	std::string const m_name;
	std::string const m_player;
	bool const m_registered;
	std::string const m_team;
	std::string const m_videoUrl;
};

struct SimplePlayer
{
	SimplePlayer(std::shared_ptr< game::Player > const iPlayer)
		: m_name(iPlayer->getName())
		, m_robot(iPlayer->getHasRobot() ? iPlayer->getRobot()->getName() : "")
	{
	}

	SimplePlayer(SimplePlayer const & iOther)
		: m_name(iOther.m_name)
		, m_robot(iOther.m_robot)
	{
	}

	std::string const m_name;
	std::string const m_robot;
};

struct SimpleTeam
{
	SimpleTeam(game::Team const & iTeam)
		: m_name(iTeam.getName())
		, m_score(iTeam.getScore())
		, m_robots(iTeam.getRobots())
	{
	}

	SimpleTeam(SimpleTeam const & iOther)
		: m_name(iOther.m_name)
		, m_score(iOther.m_score)
		, m_robots(iOther.m_robots)
	{
	}

	std::string const m_name;
	uint32_t const m_score;
	std::vector< std::string > const m_robots;
};

void to_json(json & oJson, SimplePlayer const & iPlayer)
{
	oJson = json {
			{"name", iPlayer.m_name},
			{"robot", iPlayer.m_robot}
	};
}

void to_json(json & oJson, SimpleRobot const & iRobot)
{
	oJson = json {
			{"name", iRobot.m_name},
			{"player", iRobot.m_player},
			{"registered", iRobot.m_registered},
			{"team", iRobot.m_team},
			{"video_url", iRobot.m_videoUrl},
	};
}

void to_json(json & oJson, SimpleTeam const & iTeam)
{
	oJson = json {
			{"name", iTeam.m_name},
			{"score", iTeam.m_score},
			{"robots", iTeam.m_robots}
	};
}

AgentProxy::AgentProxy(orwell::Application & ioApplication)
	: m_application(ioApplication)
	, m_outputMode(OutputMode::kText)
{
}

void echo(std::string const & iMessage)
{
	ORWELL_LOG_DEBUG("iMessage = '" << iMessage << "'");
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

/// Read a single string if no '"' is found at the begining ; otherwise
/// read all strings available until one contains a trailing '"' and join
/// them with ' '.
///
/// \param ioStream
///  The stream to read from.
///
/// \param ioValue
///  This will be updated with what has been read.
///
static void ReadName(
		std::istringstream & ioStream,
		std::string & ioValue)
{
	std::string aArg;
	ioStream >> aArg;
	size_t aLength = aArg.size();
	if (0 < aLength)
	{
		if ('"' == aArg.front())
		{
			aArg = aArg.substr(1);
			--aLength;
			bool aContinue = true;
			bool aFirst = true;
			while (aContinue)
			{
				if (0 < aLength)
				{
					aContinue = (aArg.back() != '"');
					if (not aContinue)
					{
						aArg = aArg.substr(0, aLength - 1);
					}
				}
				if (not aFirst)
				{
					ioValue += " ";
				}
				else
				{
					aFirst = false;
				}
				ioValue += aArg;
				if ((aContinue) and (not ioStream.eof()))
				{
					ioStream >> aArg;
					aLength = aArg.size();
				}
			}
		}
		else
		{
			ioValue = aArg;
		}
	}
}

static void DispatchArgument(
		std::istringstream & ioStream,
		std::function< void(std::string const &) > iFunction,
		bool & oResult,
		std::string & ioReply)
{
	std::string aArg;
	ReadName(ioStream, aArg);
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
	if ("list" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		std::string aIgnored;
		aResult = aStream.eof();
		if (aResult)
		{
			if ("player" == aObject)
			{
				listPlayer(ioReply);
			}
			else if ("robot" == aObject)
			{
				listRobot(ioReply);
			}
			else if ("team" == aObject)
			{
				listTeam(ioReply);
			}
			else
			{
				aResult = false;
			}
		}
	}
	else if ("get" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		std::string aName;
		if (("robot" == aObject) or ("team" == aObject))
		{
			ReadName(aStream, aName);
		}
		std::string aProperty;
		aStream >> aProperty;
		if ("robot" == aObject)
		{
			aResult = getRobot(aName, aProperty, ioReply);
		}
		else if ("team" == aObject)
		{
			aResult = getTeam(aName, aProperty, ioReply);
		}
		else if ("game" == aObject)
		{
			aResult = getGame(aProperty, ioReply);
		}
	}
	else if ("view" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		std::string aName;
		if ("team" == aObject)
		{
			ReadName(aStream, aName);
			viewTeam(aName, ioReply);
			aResult = true;
		}
	}
	else if (OutputMode::kText == m_outputMode)
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
				std::string aName;
				ReadName(aStream, aName);
				std::string aTeam;
				ReadName(aStream, aTeam);
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
				std::string aName;
				ReadName(aStream, aName);
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
				std::string aName;
				ReadName(aStream, aName);
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
	return aResult;
}

void AgentProxy::stopApplication()
{
	ORWELL_LOG_INFO("stop application");
	m_application.stop();
}

void AgentProxy::listTeam(std::string & ioReply) const
{
	ORWELL_LOG_INFO("list team " << m_outputMode);
	std::vector< std::string > aTeams;
	m_application.accessServer()->accessContext().getTeams(aTeams);
	switch (m_outputMode)
	{
		case OutputMode::kText:
		{
			ioReply = "Teams:\n";
			for (auto const & aTeam : aTeams)
			{
				ioReply += "\t" + aTeam + "\n";
			}
			break;
		}
		case OutputMode::kJson:
		{
			json aJsonTeams;
			aJsonTeams["Teams"] = aTeams;
			ioReply = aJsonTeams.dump();
			break;
		}
	}
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

bool AgentProxy::getTeam(
		std::string const & iTeamName,
		std::string const & iProperty,
		std::string & oValue) const
{
	bool aOK = false;
	ORWELL_LOG_INFO("get team " << iTeamName << " " << iProperty);
	try
	{
		orwell::game::Team aTeam =
				m_application.accessServer()->accessContext().getTeam(iTeamName);
		if (orwell::game::Team::GetNeutralTeam() == aTeam)
		{
			ORWELL_LOG_WARN("Invalid team name '" << iTeamName << "'");
			return aOK;
		}
		switch (m_outputMode)
		{
			case OutputMode::kText:
			{
				if ("score" == iProperty)
				{
					oValue = boost::lexical_cast< std::string >(aTeam.getScore());
					ORWELL_LOG_INFO("score = " << oValue);
					aOK = true;
				}
				else
				{
					oValue = "KO";
					ORWELL_LOG_WARN("Unknown property for a team: '" << iProperty << "'");
				}
				break;
			}

			case OutputMode::kJson:
			{
				json aJsonTeam;
				if ("score" == iProperty)
				{
					aJsonTeam[iProperty] = aTeam.getScore();
					aOK = true;
				}
				else
				{
					oValue = "KO";
					ORWELL_LOG_WARN("Unknown property for a team: '" << iProperty << "'");
				}
				oValue = aJsonTeam.dump();
				ORWELL_LOG_INFO(iProperty << " = " << oValue);
				break;
			}
		}
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
	return aOK;
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

void AgentProxy::listRobot(std::string & ioReply) const
{
	ORWELL_LOG_INFO("list robot " << m_outputMode);
	std::map< std::string, std::shared_ptr< orwell::game::Robot > > aRobots =
		m_application.accessServer()->accessContext().getRobots();
	switch (m_outputMode)
	{
		case OutputMode::kText:
		{
			ioReply = "Robots:\n";
			for (auto const & aPair : aRobots)
			{
				ioReply += "\t" + aPair.first + " -> ";
				ioReply += "name = " + aPair.second->getName() + " ; ";
				if (not aPair.second->getHasRealRobot())
				{
					ioReply += "not ";
				}
				ioReply += "registered ; ";
				ioReply += "video_url = " + aPair.second->getVideoUrl() + " ; ";
				bool aHasPlayer(aPair.second->getPlayer());
				if (aHasPlayer)
				{
					ioReply += "player = " + aPair.second->getPlayer()->getName() + "\n";
				}
				else
				{
					ioReply += "player = \n";
				}
			}
			break;
		}
		case OutputMode::kJson:
		{
			json aJsonRobots;
			std::vector< SimpleRobot > aRobotList;
			for (auto const & aPair : aRobots)
			{
				aRobotList.push_back({ aPair.second });
			}
			aJsonRobots["Robots"] = aRobotList;
			ioReply = aJsonRobots.dump();
			break;
		}
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

bool AgentProxy::getRobot(
		std::string const & iRobotName,
		std::string const & iProperty,
		std::string & oValue) const
{
	bool aOK = false;
	ORWELL_LOG_INFO("get robot '" << iRobotName << "' " << iProperty);
	try
	{
		std::shared_ptr< orwell::game::Robot > aRobot =
			m_application.accessServer()->accessContext().accessRobot(iRobotName);
		switch (m_outputMode)
		{
			case OutputMode::kText:
			{
				if ("id" == iProperty)
				{
					oValue = aRobot->getRobotId();
					ORWELL_LOG_INFO("id = " << oValue);
					aOK = true;
				}
				else if ("video_url" == iProperty)
				{
					oValue = aRobot->getVideoUrl();
					aOK = true;
				}
				else if ("video_port" == iProperty)
				{
					oValue = boost::lexical_cast< std::string >(aRobot->getVideoRetransmissionPort());
					ORWELL_LOG_INFO("video retransmission port = " << oValue);
					aOK = true;
				}
				else if ("video_command_port" == iProperty)
				{
					oValue = boost::lexical_cast< std::string >(aRobot->getServerCommandPort());
					ORWELL_LOG_INFO("video retransmission port = " << oValue);
					aOK = true;
				}
				else
				{
					oValue = "KO";
					ORWELL_LOG_WARN("Unknown property for a robot: '" << iProperty << "'");
				}
				break;
			}

			case OutputMode::kJson:
			{
				json aJsonRobot;
				if ("id" == iProperty)
				{
					aJsonRobot[iProperty] = aRobot->getRobotId();
					aOK = true;
				}
				// the other properties make no sense now
				else
				{
					ORWELL_LOG_WARN("Unknown property for a robot: '" << iProperty << "'");
				}
				oValue = aJsonRobot.dump();
				ORWELL_LOG_INFO(iProperty << " = " << oValue);
				break;
			}
		}
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
	return aOK;
}

void AgentProxy::listPlayer(std::string & ioReply) const
{
	ORWELL_LOG_INFO("list player " << m_outputMode);
	std::map< std::string, std::shared_ptr< orwell::game::Player > > aPlayers =
		m_application.accessServer()->accessContext().getPlayers();
	switch (m_outputMode)
	{
		case OutputMode::kText:
		{
			ioReply = "Players:\n";
			for (auto const & aPair : aPlayers)
			{
				ioReply += "\t" + aPair.first + " -> ";
				ioReply += "name = " + aPair.second->getName() + " ; ";
				bool aHasRobot(aPair.second->getRobot());
				if (aHasRobot)
				{
					ioReply += "robot = " + aPair.second->getRobot()->getName() + "\n";
				}
				else
				{
					ioReply += "robot = \n";
				}
			}
			break;
		}
		case OutputMode::kJson:
		{
			json aJsonPlayers;
			std::vector< SimplePlayer > aPlayerList;
			for (auto const & aPair : aPlayers)
			{
				aPlayerList.push_back({ aPair.second });
			}
			aJsonPlayers["Players"] = aPlayerList;
			ioReply = aJsonPlayers.dump();
			break;
		}
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

bool AgentProxy::getGame(
		std::string const & iProperty,
		std::string & oValue) const
{
	bool aOK = false;
	ORWELL_LOG_INFO("get game " << iProperty);
	try
	{
		orwell::game::Game const& aGame = m_application.accessServer()->accessContext();
		switch (m_outputMode)
		{
			case OutputMode::kText:
			{
				if ("time" == iProperty)
				{
					oValue = boost::lexical_cast< std::string >(aGame.getSecondsLeft());
					ORWELL_LOG_INFO("time = " << oValue);
					aOK = true;
				}
				else if ("running" == iProperty)
				{
					oValue = boost::lexical_cast< std::string >(aGame.getIsRunning());
					ORWELL_LOG_INFO("running = " << oValue);
					aOK = true;
				}
				else if ("duration" == iProperty)
				{
					oValue = boost::lexical_cast< std::string >(aGame.getDuration().total_seconds());
					ORWELL_LOG_INFO("duration = " << oValue);
					aOK = true;
				}
				else
				{
					oValue = "KO";
					ORWELL_LOG_WARN(
							"Unknown property for a game: '" << iProperty << "'");
				}
				break;
			}

			case OutputMode::kJson:
			{
				json aJsonGame;
				if ("time" == iProperty)
				{
					aJsonGame[iProperty] = aGame.getSecondsLeft();
					aOK = true;
				}
				else if ("running" == iProperty)
				{
					aJsonGame[iProperty] = aGame.getIsRunning();
					aOK = true;
				}
				else if ("duration" == iProperty)
				{
					aJsonGame[iProperty] = aGame.getDuration().total_seconds();
					aOK = true;
				}
				else
				{
					ORWELL_LOG_WARN(
							"Unknown property for a game: '" << iProperty << "'");
				}
				oValue = aJsonGame.dump();
				ORWELL_LOG_INFO(iProperty << " = " << oValue);
				break;
			}
		}
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
	return aOK;
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

void AgentProxy::viewTeam(
		std::string const & iName,
		std::string & oReply) const
{
	ORWELL_LOG_INFO("view team '" << iName << "' " << m_outputMode);
	game::Team const & aTeam = m_application.accessServer()->accessContext().getTeam(iName);
	switch (m_outputMode)
	{
		case OutputMode::kText:
		{
			if (aTeam.getIsNeutralTeam())
			{
				oReply = "Invalid team name (" + iName + ")";
				return;
			}
			oReply = "Team " + iName + ":\n";
			oReply += "\tscore = " + std::to_string(aTeam.getScore())  + " ; ";
			oReply += "robots = [";

			bool aFirst(true);
			for (std::string const & aRobotName: aTeam.getRobots())
			{
				if (aFirst)
				{
					aFirst = false;
				}
				else
				{
					oReply += ", ";
				}
				oReply += "\"" + aRobotName + "\"";
			}
			oReply += "]";
			break;
		}
		case OutputMode::kJson:
		{
			json aJsonTeam;
			if (aTeam.getIsNeutralTeam())
			{
				aJsonTeam["Team"] = nullptr;
			}
			else
			{
				aJsonTeam["Team"] = SimpleTeam(aTeam);
			}
			oReply = aJsonTeam.dump();
			break;
		}
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
