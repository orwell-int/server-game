#pragma once

#include <string>
#include <iosfwd>

namespace orwell
{
namespace proxy
{

namespace modes
{
static std::string const kJson("json");
}

namespace objects
{
static std::string const kApplication("application");
static std::string const kGame("game");
static std::string const kRobot("robot");
static std::string const kPlayer("player");
static std::string const kTeam("team");
namespace reply
{
static std::string const kTeam("Team");
static std::string const kTeams("Teams");
static std::string const kPlayers("Players");
static std::string const kRobots("Robots");
}
}

namespace properties
{
namespace robot
{
static std::string const kId("id");
}
namespace team
{
static std::string const kScore("score");
}
namespace game
{
static std::string const kDuration("duration");
static std::string const kRunning("running");
static std::string const kTime("time");
}
}


/// Read a single string if no '"' is found at the begining ; otherwise
/// read all strings available until one contains a trailing '"' and join
/// them with ' '.
///
/// \param ioStream
///  The stream to read from.
///
/// \return
///  What has been read.
///
std::string ReadName(std::istringstream & ioStream);

}
}
