#pragma once


#include "orwell/callbacks/InterfaceProcess.hpp"
#include "orwell/com/Channel.hpp"

#include <map>
#include <memory>

namespace orwell
{

namespace game
{
class Game;
} // namespace game

namespace com
{
class RawMessage;
class Sender;
} // namespace com

namespace callbacks
{

class ProcessDecider
{
	typedef std::pair<std::string, std::unique_ptr<InterfaceProcess>> Couple;

public:
	ProcessDecider(
			game::Game & ioGame,
			std::shared_ptr< com::Sender > ioPublisher,
			std::shared_ptr< com::Socket > ioReplier);

	~ProcessDecider();

	void process(
			com::RawMessage const & iMessage,
			com::Channel const iChannel);

private:
	std::map<std::string, std::unique_ptr<InterfaceProcess>> _map;
};

} // namespace callbacks
} // namespace orwell
