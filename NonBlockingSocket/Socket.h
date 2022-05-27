#pragma once

#include "ServiceStructs.h"
#include "IPEndpoint.h"

namespace CustomSocket
{
	class Socket
	{
	public:
		//Socket& operator=(const Socket& other);
		Socket& operator=(Socket&& other) noexcept;

	public:
		Socket(SocketHandle handle = INVALID_SOCKET, 
			   IPVersion IPVersion = IPVersion::IPv4);

		//Socket(const Socket& socket);
		Socket(Socket&& socket) noexcept;

		~Socket();

		Result Create();
		Result Close();

		Result Bind(IPEndpoint endpooint);
		Result Listen(IPEndpoint endpoint, int backlog = 5);

		Result Accept(Socket& outSocket, IPEndpoint* outEndpoint = nullptr);
		Result Connect(IPEndpoint endpoint);

		Result Send(const void* data, int numberOfBytes, int& bytesSent);
		Result Recieve(void* destination, int numberOfBytes, int& bytesRecieved);

		Result SendAll(void* data, int& numberOfBytes);
		Result RecieveAll(void* data, int& numberOfBytes);

		SocketHandle GetHandle();
		IPVersion GetIPVersion();

		void SetHandle(SocketHandle handle);
		void SetIPVersion(IPVersion ipVersion);

		Result SetSocketOption(Option option, BOOL value);
	protected:

	protected:
		SocketHandle m_handle = INVALID_SOCKET;
		IPVersion m_IPVersion = IPVersion::IPv4;
	};
}