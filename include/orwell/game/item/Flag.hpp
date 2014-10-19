/*  */

#include "orwell/game/Item.hpp"

#pragma once

#include <string>
#include <memory>

namespace orwell {
namespace game {
namespace item
{

class Flag : public Item
{
public:
	Flag(std::string const & iName,
			std::string const & iRfid);
	Flag(std::string const & iName,
			int32_t const iColorCode);

	virtual ~Flag();


};

}}} //end namespace
