#pragma once

#include <NonBlockingSocket/IncludeMe.h>
#include <vector>
#include <thread>

class Server
{
protected:
	using CONNECTION_INFO = std::pair<CustomSocket::Socket, CustomSocket::IPEndpoint>;
	using CONNECTIONS = std::vector<CONNECTION_INFO>;

	struct ConnectionService
	{
		CONNECTION_INFO m_socketInfo;
		WSAPOLLFD m_socketFD;

		const char* m_writeBuffer = nullptr;
		bool m_onSendFlag = false;

		char* m_readBuffer = nullptr;
		bool m_onRecieveFlag = false;
	};

public:
	Server(const CustomSocket::IPEndpoint& IPconfig);
	Server(const std::string& ip, const uint16_t port);

	~Server() = default;

public:
	CustomSocket::Result run();
	CustomSocket::Result stop();

	CustomSocket::Result recieve(void* data, const uint16_t port);
	CustomSocket::Result send(const void* data, const uint16_t port);

	void waitForConnection();
protected:
	CustomSocket::Result disconnect(const uint16_t port);
	CustomSocket::Result connect();

	void processLoop();
	void inspectAllConnections();

	void OnRecieve(ConnectionService& connection);
	void OnSend(ConnectionService& connection);

protected:
	std::vector<ConnectionService> m_socketService;

	bool m_isRunning;
	HANDLE m_getInfoEvent;

	std::thread m_listenThread;
};