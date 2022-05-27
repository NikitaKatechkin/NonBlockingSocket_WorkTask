#pragma once

#include <NonBlockingSocket/IncludeMe.h>
#include <vector>
#include <thread>
#include <unordered_map>

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

	struct IPEndpointHasher
	{
		std::size_t operator()(const CustomSocket::IPEndpoint& key) const
		{
			using std::size_t;
			using std::hash;
			using std::string;

			return ((hash<string>()(key.GetIPString())
				^ (hash<uint16_t>()(key.GetPort()) << 1)));
		}
	};

	struct ListeningService
	{
		CONNECTION_INFO m_socketInfo;
		WSAPOLLFD m_socketFD;
	};

public:
	Server(const CustomSocket::IPEndpoint& IPconfig);
	Server(const std::string& ip, const uint16_t port);

	~Server();

public:
	CustomSocket::Result run();
	CustomSocket::Result stop();

	CustomSocket::Result recieve(const std::string& ip, const uint16_t port, 
								 void* data, int numberOfBytes);
	CustomSocket::Result send(const std::string& ip, const uint16_t port, 
							  const void* data, int numberOfBytes);

	void waitForConnection();

protected:
	CustomSocket::Result disconnect(const std::string& ip, const uint16_t port);
	CustomSocket::Result connect();

	void processLoop();
	void inspectAllConnections();

	void RecieveProcessing(const std::string& ip, const uint16_t port);
	void SendProcessing(const std::string& ip, const uint16_t port);

protected:
	virtual void OnSend(const std::string& ip, const uint16_t port, 
						const char* data, int& bytesSent);
	virtual void OnRecieve(const std::string& ip, const uint16_t port, 
						   char* data, int& bytesRecieved);
	virtual void OnConnect(const std::string& ip, const uint16_t port);
	virtual void OnDisconnect(const std::string& ip, const uint16_t port);

protected:
	ListeningService m_listeningSocketService;

	std::unordered_map<CustomSocket::IPEndpoint, ConnectionService, IPEndpointHasher> m_connections;

	bool m_isRunning;
	HANDLE m_getInfoEvent;

	std::thread m_listenThread;
};