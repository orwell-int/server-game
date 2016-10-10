#include <stdlib.h>
#include <thread>

#include <log4cxx/ndc.h>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/game/Landmark.hpp"
#include "orwell/game/RGBColour.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/RawMessage.hpp"
#include "server-game.pb.h"

#include "Common.hpp"

int g_status = 0;

static void ExpectGameState(
		bool const iGameStarted,
		orwell::com::Receiver & ioSubscriber,
		std::vector< orwell::game::Landmark > const & iMapLimits)
{
	orwell::com::RawMessage aResponse;
	bool aGotWhatExpected = false;
	int const aMaxLoops = 100;
	int aCurrentLoop = 0;

	while (not aGotWhatExpected)
	{
		if (not Common::ExpectMessage("GameState", ioSubscriber, aResponse))
		{
			ORWELL_LOG_DEBUG("Expected gamestate but we got something else : " << aResponse._type);
		}
		else
		{
			orwell::messages::GameState aGameState;
			aGameState.ParsePartialFromString(aResponse._payload);

			if (aGameState.playing() == iGameStarted)
			{
				ORWELL_LOG_DEBUG("State of the game is what was expected: " << aGameState.playing());
				if (iMapLimits.size() == static_cast< size_t >(aGameState.map_limits_size()))
				{
					aGotWhatExpected = true;
					std::vector< orwell::game::Landmark >::const_iterator aExpectedLandmark = iMapLimits.begin();
					for (int i = 0 ;
						(i < aGameState.map_limits_size()) && (aGotWhatExpected) ;
						++i, ++aExpectedLandmark)
					{
						orwell::messages::Landmark const & aMapLimit = aGameState.map_limits(i);
						orwell::messages::Coordinates const & aCoordinates = aMapLimit.position();
						orwell::messages::RGBColour const & aRGBColour = aMapLimit.colour();
						orwell::game::Landmark const aComputedLandmark(
								orwell::game::Coordinates(aCoordinates.x(), aCoordinates.y()),
								orwell::game::RGBColour(
									aRGBColour.r(),
									aRGBColour.g(),
									aRGBColour.b()));
						aGotWhatExpected = (aComputedLandmark == *aExpectedLandmark);
						if (not aGotWhatExpected)
						{
							ORWELL_LOG_ERROR("Different map limits at index " << i << ".");
							//// epic failure
							//ORWELL_LOG_ERROR("Expected: " << *aExpectedLandmark);
							//ORWELL_LOG_ERROR("Received: " << aComputedLandmark);
							{
								std::ostringstream aStream;
								aStream << *aExpectedLandmark;
								ORWELL_LOG_DEBUG("Expected landmark: " << aStream.str());
							}
							{
								std::ostringstream aStream;
								aStream << aComputedLandmark;
								ORWELL_LOG_DEBUG("Computed landmark: " << aStream.str());
							}
						}
					}
				}
				else
				{
					ORWELL_LOG_ERROR("Different sizes of map limits.");
					ORWELL_LOG_ERROR("Expected: " << iMapLimits.size());
					ORWELL_LOG_ERROR("Received: " << aGameState.map_limits_size());
				}
			}
		}
		++aCurrentLoop;
		if (aCurrentLoop > aMaxLoops)
		{
			ORWELL_LOG_ERROR("State of the game is not what was expected");
			g_status = -1;
			break;
		}
	}
}

static void Application()
{
	system("../server_main -A 9003 --publisher-port 9001 --puller-port 9002 --tic 10 --gamefile orwell-game_test.ini");
}

static void Stopper()
{
	TestAgent aTestAgent(9003);
	ORWELL_LOG_INFO("create subscriber");
	zmq::context_t aContext(1);
	orwell::com::Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, orwell::com::ConnectionMode::CONNECT, aContext);
	aTestAgent.sendCommand("ping", std::string("pong"));
	aTestAgent.sendCommand("add team TEAM");
	aTestAgent.sendCommand("add robot toto TEAM");
	aTestAgent.sendCommand("set robot toto video_url fake");
	aTestAgent.sendCommand("register robot toto");
	aTestAgent.sendCommand("start game");
	std::vector< orwell::game::Landmark > const aMapLimits = {
		orwell::game::Landmark(orwell::game::Coordinates(0, 0), orwell::game::RGBColour(255, 0, 0)),
		orwell::game::Landmark(orwell::game::Coordinates(100, 0), orwell::game::RGBColour(0, 255, 0)),
		orwell::game::Landmark(orwell::game::Coordinates(100, 100), orwell::game::RGBColour(0, 0, 255)),
		orwell::game::Landmark(orwell::game::Coordinates(0, 100), orwell::game::RGBColour(0, 255, 255))
	};
	ExpectGameState(true, aSubscriber, aMapLimits);
	aTestAgent.sendCommand("stop game");
	aTestAgent.sendCommand("stop application");
}

int main()
{
	orwell::support::GlobalLogger::Create("test_main_plus", "test_main_plus.log", true);
	log4cxx::NDC ndc("test_main_plus");
	ORWELL_LOG_INFO("Test starts\n");

	std::thread aApplicationThread(Application);
	std::thread aAgentThread(Stopper);
	aApplicationThread.join();
	aAgentThread.join();
	orwell::support::GlobalLogger::Clear();
	return g_status;
}

