#pragma once

#include <NonBlockingSocket/IncludeMe.h>
#include <vector>
#include <thread>

class Server
{
public:
	Server(const CustomSocket::IPEndpoint& IPconfig);
	~Server() = default;

public:
	CustomSocket::Result run();
	CustomSocket::Result stop();

protected:
	CustomSocket::Result disconnect(const uint16_t port);
	CustomSocket::Result connect();

	void processLoop();
	void inspectAllConnections();
protected:
	using CONNECTION_INFO = std::pair<CustomSocket::Socket, CustomSocket::IPEndpoint>;
	using CONNECTIONS = std::vector<CONNECTION_INFO>;

protected:
	CustomSocket::IPEndpoint m_IPConfig;
	CustomSocket::Socket m_listeningSocket;

	CONNECTIONS m_connection;
	std::vector<WSAPOLLFD> m_socketFDs;

	bool m_isRunning;

	std::thread m_listenThread;
};