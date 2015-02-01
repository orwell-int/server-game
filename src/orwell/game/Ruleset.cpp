#include "orwell/game/Ruleset.hpp"
#include <boost/property_tree/ptree.hpp>



namespace orwell {
namespace game {

Ruleset::Ruleset()
	: m_timeToCapture(0)
{
}

void Ruleset::parseConfig(
		std::string const & iRulesetName,
		boost::property_tree::ptree const & iPtree)
{
	m_gameName = iPtree.get< std::string >(iRulesetName + ".game_name");
	m_scoreToWin = iPtree.get< uint32_t >(iRulesetName + ".score_to_win", 0);
	m_pointsOnCapture = iPtree.get< uint32_t >(iRulesetName + ".points_on_capture", 0);

	m_timeToCapture = boost::posix_time::milliseconds(iPtree.get< uint32_t >(iRulesetName + ".time_to_capture", 0));
}

}} // namespaces

