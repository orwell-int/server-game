#pragma once

#include <string>
#include <stdint.h>

#include <boost/property_tree/ptree_fwd.hpp>

namespace orwell
{
namespace game
{
class RGBColour
{
public :
	RGBColour(
			uint8_t const iR,
			uint8_t const iG,
			uint8_t const iB);

	uint8_t getRed() const;
	uint8_t getGreen() const;
	uint8_t getBlue() const;

	bool operator==(RGBColour const & iOther) const;

	static RGBColour ParseConfig(
			std::string const & iMapLimitName,
			boost::property_tree::ptree const & iPtree);

private :
	uint8_t m_r;
	uint8_t m_g;
	uint8_t m_b;
};
}
}

std::ostream & operator<<(
		std::ostream & ioOstream,
		orwell::game::RGBColour const & iColour);
