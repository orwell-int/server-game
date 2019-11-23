#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>

namespace orwell
{
namespace game
{
class Robot;
class Item;

class Team
{
public:
	Team();
	Team(std::string const & iName);
	~Team();

	Team(Team const & iOther) = default;

	Team & operator =(Team const & iOther) = default;

	static Team & GetNeutralTeam();

	std::string const & getName() const;

	void increaseScore(uint16_t const iAmount = 1);

	uint32_t getScore() const;

	void setScore(uint32_t const iScore);

	void resetScore();

	void addRobot(std::shared_ptr<Robot> ioRobot);
	void removeRobot(std::shared_ptr<Robot> ioRobot);

	void captureItem(std::shared_ptr< Item > ioItem);

	std::vector< std::string > getRobots() const;

	bool getIsNeutralTeam() const;
private:
	std::string m_name;
	uint32_t m_score;
	std::list< std::shared_ptr< Robot > > m_robots;
	bool m_isNeutralTeam;
};

} // game
} // orwell

bool operator ==(
		orwell::game::Team const & iLeft,
		orwell::game::Team const & iRight);
