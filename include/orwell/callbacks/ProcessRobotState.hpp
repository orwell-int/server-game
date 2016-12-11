#pragma once

#include "orwell/callbacks/InterfaceProcess.hpp"

#include <set>

namespace orwell
{

namespace game
{
class Item;
} // namespace game

namespace messages
{
class ServerRobotState;
} // namespace messages

namespace callbacks
{

class ProcessRobotState : public InterfaceProcess
{
public:
	ProcessRobotState(
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	void execute();

	void lookForRfid(
			std::string const & iDestination,
			orwell::messages::ServerRobotState const & iRobotStateMsg,
			std::set< std::shared_ptr< orwell::game::Item > > & ioVisitedItems);

	void lookForColour(
			std::string const & iDestination,
			orwell::messages::ServerRobotState const & iRobotStateMsg,
			std::set< std::shared_ptr< orwell::game::Item > > & ioVisitedItems);
};

} // namespace callbacks
} // namespace orwell
