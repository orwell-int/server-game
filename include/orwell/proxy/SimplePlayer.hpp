#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace orwell
{

namespace game
{
class Player;
}

namespace proxy
{


struct SimplePlayer
{
	SimplePlayer(std::shared_ptr< game::Player > const iPlayer);

	SimplePlayer(SimplePlayer const & iOther);

	std::string const m_name;
	std::string const m_robot;
};

void to_json(nlohmann::json & oJson, SimplePlayer const & iPlayer);

}
}
