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
		bool m_onWriteFlag = false;

		char* m_readBuffer = nullptr;
		bool m_onReadFlag = false;
	};

public:
	Server(const CustomSocket::IPEndpoint& IPconfig); //TODO: classic interface with ip + port
	~Server() = default;

public:
	CustomSocket::Result run();
	CustomSocket::Result stop();

	CustomSocket::Result read(void* data, const uint16_t port);
	CustomSocket::Result write(const void* data, const uint16_t port);

	void waitForConnection();
protected:
	CustomSocket::Result disconnect(const uint16_t port);
	CustomSocket::Result connect();

	void processLoop();
	void inspectAllConnections();

protected:
	std::vector<ConnectionService> m_socketService;

	bool m_isRunning;
	HANDLE m_getInfoEvent;

	std::thread m_listenThread;
};