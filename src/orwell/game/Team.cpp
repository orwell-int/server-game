#include "orwell/game/Team.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Item.hpp"

#include <algorithm>

#include "orwell/support/GlobalLogger.hpp"

namespace orwell
{
namespace game
{

Team::Team()
	: m_score(0)
{
}

Team::Team(std::string const & iName)
	: m_name(iName)
	, m_score(0)
{
}

Team::~Team()
{
}

Team & Team::GetNeutralTeam()
{
	static Team gNeutralTeam("");
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

void Team::resetScore()
{
	m_score = 0;
}

void Team::addPlayer(std::shared_ptr<Player> ioPlayer)
{
	if (std::find(m_players.begin(), m_players.end(), ioPlayer) == m_players.end())
	{
		m_players.push_back(ioPlayer);
	}
}

void Team::removePlayer(std::shared_ptr<Player> ioPlayer)
{
	m_players.remove(ioPlayer);
}

void Team::captureItem(std::shared_ptr< Item > ioItem)
{
	ioItem->capture(*this);
}

} // game
} // orwell

