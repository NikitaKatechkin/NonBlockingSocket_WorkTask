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

		//Socket(const Socket& socket); //TODO delete
		Socket(Socket&& socket) noexcept;

		~Socket();

		Result Create();
		Result Close();

		//Result Bind(const IPEndpoint* endpooint = nullptr);
		Result Bind(const IPEndpoint& endpoint = IPEndpoint("127.0.0.1", 0));
		Result Listen(const IPEndpoint& endpoint = IPEndpoint("127.0.0.1", 0), int backlog = 5);

		Result Accept(Socket& outSocket, IPEndpoint* outEndpoint = nullptr);
		Result Connect(const IPEndpoint& endpoint);

		Result GetSocketInfo(IPEndpoint& buffer);

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