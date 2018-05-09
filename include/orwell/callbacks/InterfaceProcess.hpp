#pragma once

#include <memory>
#include <map>
#include <string>

#include "orwell/com/Sender.hpp"
#include "orwell/com/Channel.hpp"

namespace google
{
namespace protobuf
{
class  MessageLite;
}
}

namespace orwell
{
namespace com
{
class RawMessage;
}
namespace game
{
class Game;
}
namespace callbacks
{

class InterfaceProcess
{
	typedef std::string Key;
	typedef std::string Value;
	typedef std::pair<Key, Value> Argument;
	typedef std::map< Key, Value > DictionaryOfArguments;

public:
	InterfaceProcess(
			std::shared_ptr< com::Sender > ioPublisher,
			std::shared_ptr< com::Socket > ioReplier);

	InterfaceProcess(
			game::Game & ioGame,
			std::shared_ptr< com::Sender > ioPublisher,
			std::shared_ptr< com::Socket > ioReplier);

	virtual ~InterfaceProcess();
	virtual void execute() = 0;

	/// Warning: the channel only makes sense for some messages
	/// (Hello, Register) as the REPLY chanel requires a request
	/// to answer to. For the other messages, PUBLISH is used.
	void init(
			google::protobuf::MessageLite * ioMsg,
			com::Channel const iChannel);

	void insertArgument(Key const & iKey, Value const & iValue);

	void removeArgument(Key const & iKey);

	Value & accessArgument(Key const & iKey);

	Value const & getArgument(Key const & iKey) const;

protected:
	void reply(com::RawMessage const & iRawMessage);

protected:
	std::shared_ptr< com::Sender > m_publisher;
	std::shared_ptr< com::Socket > m_replier;

	google::protobuf::MessageLite * m_msg;

	game::Game * m_game;

	DictionaryOfArguments m_dictionary;

	com::Channel m_channel;
};

}
}
