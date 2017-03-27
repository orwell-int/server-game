#pragma once

#include <string>
#include <list>
#include <memory>

namespace orwell
{
namespace game
{
class Player;
class Item;

class Team
{
public:
	Team();
	Team(std::string const & iName);
	~Team();

	static Team & GetNeutralTeam();

	std::string const & getName() const;

	void increaseScore(uint16_t const iAmount = 1);

	uint32_t getScore() const;

	void resetScore();

	void addPlayer(std::shared_ptr<Player> ioPlayer);
	void removePlayer(std::shared_ptr<Player> ioPlayer);

	void captureItem(std::shared_ptr< Item > ioItem);
private:
	std::string m_name;
	uint32_t m_score;
	std::list< std::shared_ptr< Player > > m_players;
};

} // game
} // orwell

