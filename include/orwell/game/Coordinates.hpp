#pragma once

#include <string>
#include <set>
#include <map>
#include <memory>
#include <ostream>

#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace orwell
{
namespace game
{
class Team;
class Ruleset;

class Coordinates
{
public:
	Coordinates(
			uint64_t const iX,
			uint64_t const iY);

	virtual ~Coordinates();

	uint64_t getX() const;
	uint64_t getY() const;

	static Coordinates ParseConfig(
		std::string const & iCoordinatesName,
		boost::property_tree::ptree const & iPtree);
private:
	uint64_t m_x;
	uint64_t m_y;
};

} // game
} // orwell

std::ostream & operator<<(std::ostream& oOstream, const orwell::game::Coordinates & aCoordinates);

bool operator==(
		orwell::game::Coordinates const & iLeft,
		orwell::game::Coordinates const & iRight);
