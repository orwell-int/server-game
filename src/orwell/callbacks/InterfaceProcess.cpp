#include "orwell/callbacks/InterfaceProcess.hpp"
#include <stdexcept>
#include "orwell/game/Game.hpp"

namespace orwell
{
namespace callbacks
{

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

	if (nullptr != ioGame)
	{
		m_game = ioGame;
	}
}

void InterfaceProcess::insertArgument(
		const Key & iKey,
		const Value & iValue)
{
	m_dictionary[iKey] = iValue;
}

void InterfaceProcess::removeArgument(const Key & iKey)
{
	m_dictionary.erase(iKey);
}

InterfaceProcess::Value & InterfaceProcess::accessArgument(const Key & iKey)
{
	return m_dictionary.at(iKey);
}

InterfaceProcess::Value const & InterfaceProcess::getArgument(const Key & iKey) const
{
	return m_dictionary.at(iKey);
}

}
}
