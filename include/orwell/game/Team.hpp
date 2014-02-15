#pragma once

#include <string>
#include <list>
#include <memory>

namespace orwell {
namespace game {
class Player;

class Team
{
	public:
		Team(std::string const & iName);
		~Team();

		void addPlayer( std::shared_ptr<Player> iPlayer );
		void removePlayer( std::shared_ptr<Player> iPlayer );

	private:
		std::string _name;
		std::list< std::shared_ptr<Player> > _players;
};

}} //end namespace
