#include "orwell/game/Team.hpp"
#include "orwell/game/Player.hpp"

#include <algorithm>

using namespace std;

namespace orwell {
namespace game {

Team::Team(string const & iName)
	: _name(iName)
	, _players()
{
}

Team::~Team()
{
}

void Team::addPlayer( shared_ptr<Player> iPlayer)
{
	if ( std::find(_players.begin(), _players.end(), iPlayer) == _players.end())
	{
		_players.push_back( iPlayer );
	}
}

void Team::removePlayer( shared_ptr<Player> iPlayer)
{
	_players.remove( iPlayer );
}

}} // namespaces

