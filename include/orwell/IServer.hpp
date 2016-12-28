#pragma once

namespace orwell
{

namespace game
{
class Game;
}

class IServer
{
public:
	virtual ~IServer();

	virtual bool processMessageIfAvailable() = 0;

	/// Wait for 1 message and process it. Execute timed operations if needed.
	virtual void loopUntilOneMessageIsProcessed() = 0;

	/// Loop eternaly to process all incoming messages.
	virtual void loop() = 0;

	/// Correctly stop the server
	virtual void stop() = 0;

	virtual orwell::game::Game & accessContext() = 0;

	virtual void feedAgentProxy() = 0;
};

}
