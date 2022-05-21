#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>

namespace CustomSocket
{
	typedef SOCKET SocketHandle;

	enum class Result
	{
		Success, 
		Fail,
		NotImplemented
	};

	enum class IPVersion
	{
		Unknown,
		IPv4
	};

	enum class Option
	{
		IO_NonBlocking,
		TCP_NoDelay
	};
}