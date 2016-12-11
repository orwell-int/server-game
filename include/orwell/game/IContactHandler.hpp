#pragma once

#include <memory>
#include <string>

namespace orwell
{
namespace game
{
class Item;

class IContactHandler
{
public:
	virtual ~IContactHandler();

	virtual void robotIsInContactWith(
			std::string const & iRobotId,
			std::shared_ptr< Item > const iItem) = 0;

	virtual void robotDropsContactWith(
			std::string const & iRobotId,
			std::shared_ptr< Item > const iItem) = 0;
};

} // namespace game
} // namespace orwell
