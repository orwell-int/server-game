#include "orwell/callbacks/ProcessRobotState.hpp"

#include <set>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/game/Item.hpp"
#include "orwell/game/ItemEncoder.hpp"
#include "orwell/game/item/FlagDetector.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"
#include "robot.pb.h"

using orwell::messages::ServerRobotState;
using orwell::messages::PlayerState;
using orwell::messages::Item;
using orwell::messages::Battery;
using orwell::messages::Ultrasound;
using orwell::com::RawMessage;

namespace
{

void copy_battery(
		ServerRobotState const & iServerRobotState,
		PlayerState & ioPlayerState)
{
	if (iServerRobotState.has_battery())
	{
		Battery * aPbBattery = ioPlayerState.mutable_battery();
		aPbBattery->CopyFrom(iServerRobotState.battery());
	}
}

void copy_ultrasound(
		ServerRobotState const & iServerRobotState,
		PlayerState & ioPlayerState)
{
	if (iServerRobotState.has_ultrasound())
	{
		Ultrasound * aPbUltrasound = ioPlayerState.mutable_ultrasound();
		aPbUltrasound->CopyFrom(iServerRobotState.ultrasound());
	}
}

} // namespace

namespace orwell
{
namespace callbacks
{

ProcessRobotState::ProcessRobotState(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher, ioGame)
{
}

void ProcessRobotState::execute()
{
	std::string const & aDestination = getArgument("RoutingID");
	if (not m_game->getHasRobotById(aDestination))
	{
		ORWELL_LOG_WARN("This is an invalid destination: " << aDestination);
		return;
	}
	orwell::messages::ServerRobotState const & aRobotStateMsg = static_cast< orwell::messages::ServerRobotState const & >(*m_msg);

	//ORWELL_LOG_INFO("ProcessRobotState::execute : simple relay");

	std::set< std::shared_ptr< orwell::game::Item > > aVisitedItems;

	lookForRfid(aDestination, aRobotStateMsg, aVisitedItems);
	lookForColour(aDestination, aRobotStateMsg, aVisitedItems);

	//todo : what do we forward to the player ?
	// forward this message to each controler
	//RawMessage aForward(aDestination, "RobotState", aRobotStateMsg.SerializeAsString());
	//m_publisher->send( aForward );
	if (aVisitedItems.size() > 1)
	{
		ORWELL_LOG_WARN(
				"RobotState contained information about "
				<< aVisitedItems.size() << " sensors for robot with routing id "
				<< aDestination << ".");
	}
	bool aFirst = true;
	for (std::shared_ptr< orwell::game::Item > aItem: aVisitedItems)
	{
		PlayerState aPlayerState;
		if (aFirst)
		{
			aFirst = false;
			copy_battery(aRobotStateMsg, aPlayerState);
			copy_ultrasound(aRobotStateMsg, aPlayerState);
		}
		Item * aPbItem = aPlayerState.mutable_item();
		assert(nullptr != aPbItem);
		aItem->getEncoder()->encode(*aPbItem);
		RawMessage aMessage(
				aDestination,
				"PlayerState",
				aPlayerState.SerializeAsString());
		m_publisher->send(aMessage);
	}
	if (aVisitedItems.empty())
	{
		PlayerState aPlayerState;
		copy_battery(aRobotStateMsg, aPlayerState);
		copy_ultrasound(aRobotStateMsg, aPlayerState);
		RawMessage aMessage(
				aDestination,
				"PlayerState",
				aPlayerState.SerializeAsString());
		m_publisher->send(aMessage);
	}
}

void ProcessRobotState::lookForRfid(
		std::string const & iDestination,
		orwell::messages::ServerRobotState const & iRobotStateMsg,
		std::set< std::shared_ptr< orwell::game::Item > > & ioVisitedItems)
{
	for (int i = 0; i < iRobotStateMsg.rfid_size() ; ++i)
	{
		std::shared_ptr< orwell::game::Item > aItem = game::Item::GetItemByRfid(iRobotStateMsg.rfid(i).rfid());
		if (not aItem)
		{
			continue;
		}
		ioVisitedItems.insert(aItem);
		switch (iRobotStateMsg.rfid(i).status())
		{
		case messages::Status::ON :
			ORWELL_LOG_INFO("Robot " << iDestination <<
					" records contact with RFID " << iRobotStateMsg.rfid(i).rfid() <<
					" at " << iRobotStateMsg.rfid(i).timestamp());
			m_game->robotIsInContactWith(iDestination, aItem);
			break;
		case messages::Status::OFF :
			ORWELL_LOG_INFO("Robot " << iDestination <<
					" stops contact with RFID " << iRobotStateMsg.rfid(i).rfid() <<
					" at " << iRobotStateMsg.rfid(i).timestamp());
			m_game->robotDropsContactWith(iDestination, aItem);
			break;
		}
	}
}

void ProcessRobotState::lookForColour(
		std::string const & iDestination,
		orwell::messages::ServerRobotState const & iRobotStateMsg,
		std::set< std::shared_ptr< orwell::game::Item > > & ioVisitedItems)
{
	boost::posix_time::ptime aEpoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
	for (int i = 0; i < iRobotStateMsg.colour_size() ; ++i)
	{
		std::weak_ptr< orwell::game::item::FlagDetector > aPossibleFlagDetector = m_game->getFlagDetector(iDestination);
		if (auto aFlagDetector = aPossibleFlagDetector.lock())
		{
			orwell::messages::Colour aMessageColour = iRobotStateMsg.colour(i);
			ORWELL_LOG_INFO("visiting colour " << i << " sent by the robot for '"
					<< iDestination << "' and colour '" << aMessageColour.colour() << "'");
			std::weak_ptr< orwell::game::Item > aPossibleItem = aFlagDetector->setColour(
					aMessageColour.colour(),
					aEpoch + boost::posix_time::milliseconds(aMessageColour.timestamp()));
			if (auto aItem = aPossibleItem.lock())
			{
				ioVisitedItems.insert(aItem);
			}
		}
		else
		{
			ORWELL_LOG_WARN("No flag detector found for this robot: '" << iDestination << "'");
		}
	}
}

}
}
