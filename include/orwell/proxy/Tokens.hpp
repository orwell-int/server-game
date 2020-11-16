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

namespace actions
{
static std::string const kStart("start");
static std::string const kStop("stop");
static std::string const kAdd("add");
static std::string const kRemove("remove");
static std::string const kRegister("register");
static std::string const kUnregister("unregister");
static std::string const kSet("set");
static std::string const kPing("ping");
namespace reply
{
static std::string const kPong("pong");
static std::string const kKO("KO");
}
}

namespace objects
{
static std::string const kApplication("application");
static std::string const kFlag("flag");
static std::string const kGame("game");
static std::string const kPlayer("player");
static std::string const kRobot("robot");
static std::string const kTeam("team");
namespace reply
{
static std::string const kFlags("Flags");
static std::string const kPlayers("Players");
static std::string const kRobots("Robots");
static std::string const kTeam("Team");
static std::string const kTeams("Teams");
}
}

namespace properties
{
namespace robot
{
static std::string const kId("id");
static std::string const kVideoUrl("video_url");
static std::string const kVideoPort("video_port");
static std::string const kVideoCommandPort("video_command_port");
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
namespace player
{
static std::string const kAddress("address");
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
