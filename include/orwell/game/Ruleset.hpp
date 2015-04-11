
#pragma once

#include <string>
#include <stdint.h>

#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace orwell
{
namespace game
{

class Ruleset
{
public :

	Ruleset();

	void parseConfig(
			std::string const & iRulesetName,
			boost::property_tree::ptree const & iPtree);

	std::string m_gameName;
	uint32_t m_scoreToWin;
	uint32_t m_pointsOnCapture;
	boost::posix_time::milliseconds m_timeToCapture;

};
} // game
} // orwell

