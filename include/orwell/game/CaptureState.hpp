#pragma once

namespace orwell
{
namespace game
{
enum class CaptureState
{
	PENDING, ///< there is no capture yet
	STARTED,
	SUCCEEDED,
	FAILED
};
}
}
