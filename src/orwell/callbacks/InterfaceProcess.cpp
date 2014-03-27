#include "orwell/callbacks/InterfaceProcess.hpp"
#include <stdexcept>
#include "orwell/game/Game.hpp"

namespace orwell {
namespace callbacks {

InterfaceProcess::InterfaceProcess(
		std::shared_ptr< com::Sender > ioPublisher)
	: _publisher(ioPublisher)
	, _msg(nullptr)
	, _game(nullptr)
{

}

InterfaceProcess::InterfaceProcess(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher)
{
	_game = &ioGame;
}

InterfaceProcess::~InterfaceProcess()
{
}

void InterfaceProcess::setGameContext(game::Game & ioGame)
{
	_game = &ioGame;
}

void InterfaceProcess::init(
		google::protobuf::MessageLite * ioMsg,
		game::Game * ioGame)
{
	_msg = ioMsg;

	if (ioGame /*!= nullptr*/)
	{
		_game = ioGame;
	}
}

void InterfaceProcess::insertArgument(const Argument & iArgument)
{
	_dictionary.push_back(iArgument);
}

void InterfaceProcess::insertArgument(
		const Key & iKey,
		const Value & iValue)
{
	Argument anArgument(iKey, iValue);
	insertArgument(anArgument);
}

void InterfaceProcess::removeArgument(const Key & iKey)
{
	bool found(false);
	auto anIterator = _dictionary.begin();

	for (Argument const & anArgument: _dictionary)
	{
		if (anArgument.first == iKey)
		{
			found = true;
			break;
		}
		++anIterator;
	}

	if (found)
	{
		_dictionary.erase(anIterator);
	}
}

InterfaceProcess::Argument const & InterfaceProcess::getArgument(const Key & iKey)
{
	for (Argument const & anArgument: _dictionary)
	{
		if (anArgument.first == iKey)
			return anArgument;
	}

	throw 1;
}

InterfaceProcess::Argument & InterfaceProcess::accessArgument(const Key & iKey)
{
	for (Argument & anArgument: _dictionary)
	{
		if (anArgument.first == iKey)
			return anArgument;
	}

	throw 1;
}

}} //namespaces

