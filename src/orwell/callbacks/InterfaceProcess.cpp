#include "orwell/callbacks/InterfaceProcess.hpp"
#include <stdexcept>
#include "orwell/game/Game.hpp"

namespace orwell {
namespace callbacks {

InterfaceProcess::InterfaceProcess(
		std::shared_ptr< com::Sender > ioPublisher)
	: m_publisher(ioPublisher)
	, m_msg(nullptr)
	, m_game(nullptr)
{

}

InterfaceProcess::InterfaceProcess(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher)
{
	m_game = &ioGame;
}

InterfaceProcess::~InterfaceProcess()
{
}

void InterfaceProcess::setGameContext(game::Game & ioGame)
{
	m_game = &ioGame;
}

void InterfaceProcess::init(
		google::protobuf::MessageLite * ioMsg,
		game::Game * ioGame)
{
	m_msg = ioMsg;

	if (ioGame /*!= nullptr*/)
	{
		m_game = ioGame;
	}
}

void InterfaceProcess::insertArgument(const Argument & iArgument)
{
	m_dictionary.push_back(iArgument);
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
	auto anIterator = m_dictionary.begin();

	for (Argument const & anArgument: m_dictionary)
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
		m_dictionary.erase(anIterator);
	}
}

InterfaceProcess::Argument const & InterfaceProcess::getArgument(const Key & iKey)
{
	for (Argument const & anArgument: m_dictionary)
	{
		if (anArgument.first == iKey)
			return anArgument;
	}

	throw 1;
}

InterfaceProcess::Argument & InterfaceProcess::accessArgument(const Key & iKey)
{
	for (Argument & anArgument: m_dictionary)
	{
		if (anArgument.first == iKey)
			return anArgument;
	}

	throw 1;
}

}} //namespaces

