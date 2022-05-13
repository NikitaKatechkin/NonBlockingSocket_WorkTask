#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>

namespace CustomSocket
{
	namespace NetworkAPIInitializer
	{
		bool Initialize();
		void Shutdown();
	}
}