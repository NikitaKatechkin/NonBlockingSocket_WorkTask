#pragma once

#include "ServiceStructs.h"

#include <string>
#include <vector>
#include <WS2tcpip.h>

namespace CustomSocket
{
	class IPEndpoint
	{
	public:
		IPEndpoint() = default;
		IPEndpoint(const std::string& ip, uint16_t port);
		IPEndpoint(sockaddr* addr);
		~IPEndpoint() = default;

		IPVersion GetIPVersion();
		std::string GetHostname();
		std::string GetIPString();
		uint8_t* GetIPBytes();
		uint16_t GetPort() const;

		sockaddr_in GetSockaddrIPv4();
	public:
		friend std::ostream& operator << (std::ostream& outputStream,
										  const IPEndpoint& obj);
	private:
		std::string m_hostname;

		IPVersion m_ipVersion = IPVersion::Unknown;
		std::string m_ipString;
		uint8_t* m_ipBytes = nullptr;

		uint16_t m_port = 0;
	};
}
