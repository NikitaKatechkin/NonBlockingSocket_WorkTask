#pragma once

#include <NonBlockingSocket/IncludeMe.h>
#include <vector>
#include <thread>
#include <unordered_map>
#include <mutex>

class Server
{
protected:
	using CONNECTION_INFO = std::pair<CustomSocket::Socket, CustomSocket::IPEndpoint>;

	template <typename T>
	struct ServerMessage
	{
		T m_buffer = nullptr;
		int m_bufferTotalSize = 0;
		int m_bytesToProcess = 0;
		bool m_onProcessFlag = false;
	};

	struct ConnectionService
	{
		CONNECTION_INFO m_socketInfo;
		WSAPOLLFD m_socketFD;

		ServerMessage<const char*> m_sendMessage;
		HANDLE m_onSendEvent = CreateEvent(nullptr, TRUE, FALSE, L"OnSendEvent");

		ServerMessage<char*> m_recieveMessage;
		HANDLE m_onRecieveEvent = CreateEvent(nullptr, TRUE, FALSE, L"OnRecieveEvent");

	};

	struct IPEndpointHasher
	{
		std::size_t operator()(const CustomSocket::IPEndpoint& key) const
		{
			uint32_t ip = 0;

			switch (key.GetIPVersion())
			{
			case CustomSocket::IPVersion::IPv4:
			{
				memcpy_s(&ip, sizeof(uint32_t),
						 key.GetIPBytes(), sizeof(uint32_t));
			}
			default:
				break;
			}
			

			return ((std::hash<uint32_t>()(ip)
					^ (std::hash<uint16_t>()(key.GetPort()) << 1)));
		}
	};

	struct ListeningService
	{
		CONNECTION_INFO m_socketInfo;
		WSAPOLLFD m_socketFD;
	};

protected:
	using CONNECTIONS = std::unordered_map<CustomSocket::IPEndpoint, ConnectionService, IPEndpointHasher>;
	using CONNECTIONS_LIST = std::vector<CustomSocket::IPEndpoint>;

public:
	Server(const CustomSocket::IPEndpoint& IPconfig = CustomSocket::IPEndpoint("127.0.0.1", 0));
	Server(const std::string& ip = "127.0.0.1", const uint16_t port = 0);

	~Server();

public:
	CustomSocket::Result Run(); //+
	CustomSocket::Result Stop(); //+

	CustomSocket::Result Recieve(const std::string& ip, const uint16_t port, 
								 void* data, int numberOfBytes); //+
	CustomSocket::Result Send(const std::string& ip, const uint16_t port, 
							  const void* data, int numberOfBytes); //+

	void WaitForConnection(); //+

	CONNECTIONS_LIST GetConnectionList(); //+
	CustomSocket::IPEndpoint GetServerIPConfig();

	CustomSocket::Result WaitClientOnSendEvent(const std::string& ip, const uint16_t port);
	CustomSocket::Result WaitClientOnRecieveEvent(const std::string& ip, const uint16_t port);

protected:
	CustomSocket::Result Disconnect(const std::string& ip, const uint16_t port); //+
	CustomSocket::Result Connect(); //+

	void ProcessLoop(); //+
	void InspectAllConnections();

	void RecieveProcessing(const std::string& ip, const uint16_t port);
	void SendProcessing(const std::string& ip, const uint16_t port);

protected:
	virtual void OnSend(const std::string& ip, const uint16_t port, 
						const char* data, int& bytesSent); //+
	virtual void OnRecieve(const std::string& ip, const uint16_t port, 
						   char* data, int& bytesRecieved); //+
	virtual void OnConnect(const std::string& ip, const uint16_t port); //+
	virtual void OnDisconnect(const std::string& ip, const uint16_t port);  //+

protected:
	ListeningService m_listeningSocketService;

	//std::unordered_map<CustomSocket::IPEndpoint, ConnectionService, IPEndpointHasher> m_connections;
	CONNECTIONS m_connections;

	bool m_isRunning;
	HANDLE m_getInfoEvent;

	std::thread m_listenThread;

	std::mutex m_printLogMutex;
	std::mutex m_operationMutex;
};