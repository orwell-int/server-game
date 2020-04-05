#include "orwell/proxy/SimplePlayer.hpp"

#include "orwell/game/Robot.hpp"
#include "orwell/game/Player.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orwell
{

namespace game
{
class Player;
}

namespace proxy
{

void to_json(json & oJson, SimplePlayer const & iPlayer)
{
	oJson = json {
			{"name", iPlayer.m_name},
			{"robot", iPlayer.m_robot}
	};
}

SimplePlayer::SimplePlayer(std::shared_ptr< game::Player > const iPlayer)
	: m_name(iPlayer->getName())
	, m_robot(iPlayer->getHasRobot() ? iPlayer->getRobot()->getName() : "")
{
}

SimplePlayer::SimplePlayer(SimplePlayer const & iOther)
	: m_name(iOther.m_name)
	, m_robot(iOther.m_robot)
{
}

}
}
