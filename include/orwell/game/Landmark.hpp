#pragma once

#include <string>
#include <set>
#include <map>
#include <memory>
#include <ostream>

#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <orwell/game/RGBColour.hpp>
#include <orwell/game/Coordinates.hpp>

namespace orwell
{
namespace game
{
class Team;
class Ruleset;

class Landmark
{
public:
	Landmark(
		Coordinates const & iPosition,
		RGBColour const & iColour);

	virtual ~Landmark();

	Coordinates const & getPosition() const;

	RGBColour const & getColour() const;

	bool operator==(orwell::game::Landmark const & iOther) const;

	static Landmark ParseConfig(
			std::string const & iMapLimitName,
			boost::property_tree::ptree const & iPtree);

private:
	Coordinates m_position;
	RGBColour m_colour;
};

} // game
} // orwell

std::ostream & operator<<(
		std::ostream & ioOstream,
		orwell::game::Landmark const & iLandmark);
