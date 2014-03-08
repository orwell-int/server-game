#pragma once

#include <string>

namespace orwell
{

class BroadcastServer
{
public:
	BroadcastServer(
			std::string const & iPullerUrl,
			std::string const & iPublisherUrl);

	~BroadcastServer();

	/// run the broadcast receiver
	void runBroadcastReceiver();
	/// Correctly stop the server
	void stop();

private:
	bool _mainLoopRunning;
	bool _forcedStop;
	std::string const _pullerUrl;
	std::string const _publisherUrl;
};

}

