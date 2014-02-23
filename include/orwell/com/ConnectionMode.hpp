#pragma once

namespace orwell
{
namespace com
{

namespace ConnectionMode
{
enum ConnectionMode
{
	/// Ask zmq to bind a socket so that other can connect to it.
	BIND,
	/// Ask zmq to connect to a socket already bound.
	CONNECT,
};
}

}
}
