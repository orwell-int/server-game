#pragma once

namespace orwell
{
namespace com
{

enum class ConnectionMode
{
	/// Ask zmq to bind a socket so that other can connect to it.
	BIND,
	/// Ask zmq to connect to a socket already bound.
	CONNECT,
};

}
}
