#pragma once

#include "ServiceStructs.h"

#include <string>
#include <vector>
#include <WS2tcpip.h>
#include <memory>

//TODO: define move construct

namespace CustomSocket
{
	class IPEndpoint
	{
	public:
		IPEndpoint() = default;
		IPEndpoint(const std::string& ip, const uint16_t port);
		IPEndpoint(sockaddr* addr);
		IPEndpoint(const IPEndpoint& copy);
		~IPEndpoint();

	public:
		IPVersion GetIPVersion() const;
		std::string GetHostname() const;
		std::string GetIPString() const;
		uint8_t* GetIPBytes() const;
		uint16_t GetPort() const;
		sockaddr_in GetSockaddrIPv4() const;

	public:
		IPEndpoint& operator=(const IPEndpoint& copy);

	public:
		friend std::ostream& operator << (std::ostream& outputStream,
										  const IPEndpoint& obj);
		friend bool operator== (const IPEndpoint& c1, const IPEndpoint& c2);
		friend bool operator< (const IPEndpoint& c1, const IPEndpoint& c2);

	private:
		std::string m_hostname;

		IPVersion m_ipVersion = IPVersion::Unknown;
		std::string m_ipString;

		uint8_t* m_ipBytes = nullptr;

		uint16_t m_port = 0;
	};
}
