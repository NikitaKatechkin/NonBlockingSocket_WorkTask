#pragma once

#include <NonBlockingSocket/IncludeMe.h>
#include <thread>

class Client
{
protected:
	using CONNECTION_INFO = std::pair<CustomSocket::Socket, CustomSocket::IPEndpoint>;

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
	Client(const CustomSocket::IPEndpoint& clientConfig = 
										CustomSocket::IPEndpoint("127.0.0.1", 0));
	Client(const std::string& IP = "127.0.0.1", const uint16_t port = 0);
	~Client();

	CustomSocket::Result Connect(const CustomSocket::IPEndpoint& serverEndpoint);
protected:
	void Run();
	void Stop();

	void ProcessLoop();
	void ProcessRecieving();
	void ProcessSending();

	//void OnConnect(const std::string& IP, const uint16_t port);
	void OnConnect();
protected:
	ConnectionService m_service;

	bool m_isRunning = false;
	std::thread m_processThread;
};