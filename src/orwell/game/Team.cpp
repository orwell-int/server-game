#include "orwell/game/Team.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Item.hpp"

#include <algorithm>

#include "orwell/support/GlobalLogger.hpp"

namespace orwell
{
namespace game
{

Team::Team()
	: m_score(0)
	, m_isNeutralTeam(true)
{
}

Team::Team(std::string const & iName)
	: m_name(iName)
	, m_score(0)
	, m_isNeutralTeam(false)
{
}

Team::~Team()
{
}

Team & Team::GetNeutralTeam()
{
	static Team gNeutralTeam;
	return gNeutralTeam;
}

std::string const & Team::getName() const
{
	return m_name;
}

void Team::increaseScore(uint16_t const iAmount)
{
	m_score += iAmount;
	ORWELL_LOG_DEBUG("Score increased to " << m_score);
}

uint32_t Team::getScore() const
{
	return m_score;
}

void Team::setScore(uint32_t const iScore)
{
	m_score = iScore;
}

void Team::resetScore()
{
	m_score = 0;
}

void Team::addRobot(std::shared_ptr<Robot> ioRobot)
{
	if (std::find(m_robots.begin(), m_robots.end(), ioRobot) == m_robots.end())
	{
		m_robots.push_back(ioRobot);
	}
}

void Team::removeRobot(std::shared_ptr<Robot> ioRobot)
{
	m_robots.remove(ioRobot);
}

void Team::captureItem(std::shared_ptr< Item > ioItem)
{
	ioItem->capture(*this);
}

std::vector< std::string > Team::getRobots() const
{
	std::vector< std::string > aRobots;
	std::transform(
			m_robots.begin(),
			m_robots.end(),
			std::back_inserter(aRobots),
			[](std::shared_ptr< Robot > aRobot) { return aRobot->getName(); });
	return aRobots;
}

bool Team::getIsNeutralTeam() const
{
	return m_isNeutralTeam;
}

} // game
} // orwell

bool operator ==(
		orwell::game::Team const & iLeft,
		orwell::game::Team const & iRight)
{
	return (iLeft.getName() == iRight.getName());
}

