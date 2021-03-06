#include "orwell/proxy/List.hpp"

#include "orwell/proxy/OutputMode.hpp"
#include "orwell/proxy/Tokens.hpp"
#include "orwell/proxy/SimplePlayer.hpp"
#include "orwell/proxy/SimpleRobot.hpp"
#include "orwell/proxy/SimpleItem.hpp"

#include "orwell/game/Game.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Team.hpp"
#include "orwell/game/Item.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orwell
{
namespace proxy
{

/// List all the players present.
///
Reply ListPlayer(
		OutputMode const iOutputMode,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	std::map< std::string, std::shared_ptr< orwell::game::Player > > aPlayers =
		iGame.getPlayers();
	ORWELL_LOG_INFO("list player " << iOutputMode << " (" << aPlayers.size() << ")");
	switch (iOutputMode)
	{
		case OutputMode::kText:
		{
			aReply = objects::reply::kPlayers + ":\n";
			for (auto const & aPair : aPlayers)
			{
				aReply += "\t" + aPair.first + " -> ";
				aReply += aPair.second->getAsString() + "\n";
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
			aJsonPlayers[objects::reply::kPlayers] = aPlayerList;
			aReply = aJsonPlayers.dump();
			break;
		}
	}
	return aReply;
}

/// List all the robots present.
///
Reply ListRobot(
		OutputMode const iOutputMode,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	std::map< std::string, std::shared_ptr< orwell::game::Robot > > aRobots =
		iGame.getRobots();
	ORWELL_LOG_INFO("list robot " << iOutputMode << " (" << aRobots.size() << ")");
	switch (iOutputMode)
	{
		case OutputMode::kText:
		{
			aReply = objects::reply::kRobots + ":\n";
			for (auto const & aPair : aRobots)
			{
				aReply += "\t" + aPair.first + " -> ";
				aReply += aPair.second->getAsString() + "\n";
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
			aJsonRobots[objects::reply::kRobots] = aRobotList;
			aReply = aJsonRobots.dump();
			break;
		}
	}
	return aReply;
}

/// List all the teams present.
///
Reply ListTeam(
		OutputMode const iOutputMode,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	std::vector< std::string > aTeams;
	iGame.getTeams(aTeams);
	ORWELL_LOG_INFO("list team " << iOutputMode << " (" << aTeams.size() << ")");
	switch (iOutputMode)
	{
		case OutputMode::kText:
		{
			aReply = objects::reply::kTeams + ":\n";
			for (auto const & aTeam : aTeams)
			{
				aReply += "\t" + aTeam + "\n";
			}
			break;
		}
		case OutputMode::kJson:
		{
			json aJsonTeams;
			aJsonTeams[objects::reply::kTeams] = aTeams;
			aReply = aJsonTeams.dump();
			break;
		}
	}
	return aReply;
}


/// List all the flags present.
///
Reply ListFlag(
		OutputMode const iOutputMode,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	orwell::game::Item::ItemVector aItems = orwell::game::Item::GetAllItems();
	ORWELL_LOG_INFO("list flag " << iOutputMode);
	switch (iOutputMode)
	{
		case OutputMode::kText:
		{
			aReply = objects::reply::kFlags + ":\n";
			for (auto const & aItem : aItems)
			{
				aReply += "\t" + aItem->getAsString() + "\n";
			}
			break;
		}
		case OutputMode::kJson:
		{
			json aJsonFlags;
			// for now all items are flags
			std::vector< SimpleItem > aFlagList;
			for (auto const & aItem : aItems)
			{
				aFlagList.push_back({ aItem });
			}
			aJsonFlags[objects::reply::kFlags] = aFlagList;
			aReply = aJsonFlags.dump();
			break;
		}
	}
	return aReply;
}


List::List()
	: IAction("list")
{
}

Reply List::process(
		OutputMode const iOutputMode,
		std::istringstream & ioStream,
		orwell::Application & ioApplication,
		orwell::game::Game const& iGame) const
{
	ORWELL_LOG_DEBUG("List::process");
	Reply aReply;
	std::string aObject;
	ioStream >> aObject;
	if (ioStream.eof())
	{
		if (objects::kPlayer == aObject)
		{
			aReply = ListPlayer(iOutputMode, iGame);
		}
		else if (objects::kRobot == aObject)
		{
			aReply = ListRobot(iOutputMode, iGame);
		}
		else if (objects::kTeam == aObject)
		{
			aReply = ListTeam(iOutputMode, iGame);
		}
		else if (objects::kFlag == aObject)
		{
			aReply = ListFlag(iOutputMode, iGame);
		}
		else
		{
			aReply.fail("Invalid object for list: '" + aObject + "'");
		}
	}
	else
	{
		aReply.fail(
				"Unexpected text after list "
				+ aObject + ": '" + ioStream.str() + "'");
	}
	return aReply;
}

}
}
