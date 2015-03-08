#pragma once

/*
 * Abstract class of which all callbacks inherit
 */

#include <memory>
#include <map>
#include <string>

#include "orwell/com/Sender.hpp"

namespace google
{
namespace protobuf
{
class  MessageLite;
}
}

namespace orwell {
namespace game {
class Game;
}
namespace callbacks {

class InterfaceProcess
{
	typedef std::string Key;
	typedef std::string Value;
	typedef std::pair<Key, Value> Argument;
	typedef std::map< Key, Value > DictionaryOfArguments;

public:
	InterfaceProcess(
			std::shared_ptr< com::Sender > ioPublisher);

	InterfaceProcess(
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	virtual ~InterfaceProcess();
	virtual void execute() = 0;

	void init(
			google::protobuf::MessageLite * ioMsg,
			game::Game * ioGame = nullptr);

	void setGameContext(game::Game & ioGame);

	//void insertArgument(Argument const & iArgument);

	void insertArgument(Key const & iKey, Value const & iValue);

	void removeArgument(Key const & iKey);

	Value & accessArgument(Key const & iKey);

	Value const & getArgument(Key const & iKey) const;

protected:
	std::shared_ptr< com::Sender > m_publisher;

	google::protobuf::MessageLite * m_msg;

	game::Game * m_game;

	DictionaryOfArguments m_dictionary;

};

}} //namespaces

