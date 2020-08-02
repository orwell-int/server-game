#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace orwell
{

namespace game
{
class Robot;
}

namespace proxy
{


struct SimpleRobot
{
	SimpleRobot(std::shared_ptr< game::Robot const > const iRobot);

	SimpleRobot(SimpleRobot const & iOther);

	std::string const m_name;
	std::string const m_player;
	bool const m_registered;
	std::string const m_team;
	std::string const m_videoUrl;
};

void to_json(nlohmann::json & oJson, SimpleRobot const & iRobot);

}
}
