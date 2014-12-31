#pragma once

namespace boost {
namespace posix_time {
class ptime;
}
}

namespace orwell
{
namespace game
{

enum class StepSignal
{
	AH_AH_AH_AH_STAYINGALIVE,
	SILENCEIKILLU
};

class TimeBound
{
public:
	virtual StepSignal step(boost::posix_time::ptime const & iCurrentTime) = 0;

protected :
	TimeBound();
	virtual ~TimeBound();
};

}
} //end namespace

