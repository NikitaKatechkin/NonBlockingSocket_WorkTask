#pragma once

#include "ServiceStructs.h"
#include "IPEndpoint.h"

namespace CustomSocket
{
	class Socket
	{
	public:
		Socket(SocketHandle handle = INVALID_SOCKET, 
			   IPVersion IPVersion = IPVersion::IPv4);
		~Socket() = default;

		Result create();
		Result close();

		Result Bind(IPEndpoint endpooint);
		Result Listen(IPEndpoint endpoint, int backlog = 5);

		Result Accept(Socket& outSocket);
		Result Connect(IPEndpoint endpoint);

		Result Send(void* data, int numberOfBytes, int& bytesSent);
		Result Recieve(void* destination, int numberOfBytes, int& bytesRecieved);

		Result SendAll(void* data, int numberOfBytes);
		Result RecieveAll(void* data, int numberOfBytes);

		SocketHandle getHandle();
		IPVersion getIPVersion();

		void setHandle(SocketHandle handle);
		void setIPVersion(IPVersion ipVersion);
	private:
		Result setSocketOption(Option option, BOOL value);

	private:
		SocketHandle m_handle = INVALID_SOCKET;
		IPVersion m_IPVersion = IPVersion::IPv4;
	};
}