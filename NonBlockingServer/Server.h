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

		const char* m_writeBuffer;
		bool m_onWriteFlag;

		char* m_readBuffer;
		bool m_onReadFlag;
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
	CustomSocket::IPEndpoint m_IPConfig;
	CustomSocket::Socket m_listeningSocket;

	//TODO: elegant vector of struct instead
	CONNECTIONS m_connection;
	std::vector<WSAPOLLFD> m_socketFDs;

	std::vector<const char*> m_writeBuffer;
	std::vector<bool> m_onWriteFlag;

	std::vector<char*> m_readBuffer;
	std::vector<bool> m_onReadFlag;

	bool m_isRunning;
	HANDLE m_getInfoEvent;

	std::thread m_listenThread;
};