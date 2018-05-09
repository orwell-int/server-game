#include "orwell/callbacks/InterfaceProcess.hpp"
#include <stdexcept>
#include "orwell/game/Game.hpp"
#include "orwell/support/GlobalLogger.hpp"

#include "orwell/com/RawMessage.hpp"

namespace orwell
{
namespace callbacks
{

InterfaceProcess::InterfaceProcess(
		std::shared_ptr< com::Sender > ioPublisher,
		std::shared_ptr< com::Socket > ioReplier)
	: m_publisher(ioPublisher)
	, m_replier(ioReplier)
	, m_msg(nullptr)
	, m_game(nullptr)
{

}

InterfaceProcess::InterfaceProcess(
		game::Game & ioGame,
		std::shared_ptr< com::Sender > ioPublisher,
		std::shared_ptr< com::Socket > ioReplier)
	: InterfaceProcess(ioPublisher, ioReplier)
{
	m_game = &ioGame;
}

InterfaceProcess::~InterfaceProcess()
{
}

void InterfaceProcess::init(
		google::protobuf::MessageLite * ioMsg,
		com::Channel const iChannel)
{
	m_msg = ioMsg;
	m_channel = iChannel;
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

void InterfaceProcess::reply(com::RawMessage const & iRawMessage)
{
	switch (m_channel)
	{
		case com::Channel::PUBLISH:
		{
			ORWELL_LOG_DEBUG("Reply through PUBLISH channel");
			m_publisher->send(iRawMessage);
			break;
		}
		case com::Channel::REPLY:
		{
			ORWELL_LOG_DEBUG("Reply through REPLY channel");
			m_replier->send(iRawMessage);
			break;
		}
	}
}

}
}
