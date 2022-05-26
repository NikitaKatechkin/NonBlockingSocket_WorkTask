#pragma once

#include <NonBlockingSocket/IncludeMe.h>
#include <vector>
#include <thread>

//TODO:
//map using (unordered maybe)
//socket should not have copy opersstion alowed


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
		int m_bytesToSend = 0;

		bool m_onSendFlag = false;

		char* m_readBuffer = nullptr;
		int m_bytesToRecieve = 0;

		bool m_onRecieveFlag = false;
	};

public:
	Server(const CustomSocket::IPEndpoint& IPconfig);
	Server(const std::string& ip, const uint16_t port);

	~Server() = default;

public:
	CustomSocket::Result run();
	CustomSocket::Result stop();

	CustomSocket::Result recieve(const std::string& ip, const uint16_t port, 
								 void* data, int numberOfBytes);
	CustomSocket::Result send(const std::string& ip, const uint16_t port, 
							  const void* data, int numberOfBytes);

	void waitForConnection();
protected:
	CustomSocket::Result disconnect(const uint16_t port);
	CustomSocket::Result connect();

	void processLoop();
	void inspectAllConnections();

	void OnRecieve(const std::string& ip, const uint16_t port);
	void OnSend(const std::string& ip, const uint16_t port);

protected:
	std::vector<ConnectionService> m_socketService;

	bool m_isRunning;
	HANDLE m_getInfoEvent;

	std::thread m_listenThread;
};