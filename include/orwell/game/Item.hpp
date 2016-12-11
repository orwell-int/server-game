#pragma once

#include <string>
#include <set>
#include <map>
#include <memory>
#include <ostream>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "orwell/game/CaptureState.hpp"

namespace orwell
{
namespace game
{
class ItemEncoder;
class Team;
class Ruleset;

class Item
{
protected:
	Item(
			std::string const & iName,
			std::set< std::string > const & iRfids,
			boost::posix_time::milliseconds const & iTimeToCapture);

	Item(
			std::string const & iName,
			int32_t const iColourCode,
			boost::posix_time::milliseconds const & iTimeToCapture);

	virtual ~Item();

public:
	std::string const & getName() const;
	std::set< std::string > const & getRfids() const;
	int32_t getColour() const;
	std::string const & getTeam() const;

	virtual std::unique_ptr< ItemEncoder > getEncoder() const = 0;

	static void InitializeStaticMaps();

	static std::shared_ptr<Item> GetItemByRfid(
			std::string const & iRfid);

	static std::shared_ptr<Item> GetItemByColour(
			int32_t const iColourCode);

	static std::shared_ptr<Item> CreateItem(
			std::string const & iType,
			std::string const & iName,
			std::set< std::string > const & iRfids,
			int32_t const iColourCode,
			Ruleset const & iRuleset);

	static std::vector< std::shared_ptr< Item > > GetAllItems();

	virtual std::string toLogString() const;

	void startCapture(std::string const & iCapturingTeam);
	void abortCapture();
	std::string const & getCapturingTeam() const;
	CaptureState getCaptureState() const;
	void capture(Team & ioTeam);

private:
	enum class Kind
	{
		COLOUR,
		RFIDS
	};
	Kind m_kind;
	std::string m_name;
	std::set< std::string > m_rfids;
	int32_t m_colour;
	std::string m_owningTeam;
	std::string m_capturingTeam;
	CaptureState m_captureState;
	boost::posix_time::milliseconds m_timeToCapture;

	static std::map< std::string, std::shared_ptr< Item > > s_itemsByRfid;
	static std::map< int32_t, std::shared_ptr< Item > > s_itemsByColour;
	static std::vector< std::shared_ptr< Item > > s_allItems;

	virtual void innerCapture(Team & ioTeam) = 0;
};

} // game
} // orwell

std::ostream & operator<<(std::ostream& oOstream, const orwell::game::Item & aItem);

