#pragma once

#include <NonBlockingSocket/IncludeMe.h>
#include <thread>
#include <mutex>

class Client
{
protected:
	using CONNECTION_INFO = std::pair<CustomSocket::Socket, CustomSocket::IPEndpoint>;

	struct ConnectionService
	{
		CONNECTION_INFO m_socketInfo;
		WSAPOLLFD m_socketFD;

		const char* m_writeBuffer = nullptr;
		int m_writeBufferTotalSize = 0;
		int m_bytesToSend = 0;

		bool m_onSendFlag = false;

		char* m_readBuffer = nullptr;
		int m_readBufferTotalSize = 0;
		int m_bytesToRecieve = 0;

		bool m_onRecieveFlag = false;
	};

public:
	Client(const CustomSocket::IPEndpoint& clientConfig = 
										CustomSocket::IPEndpoint("127.0.0.1", 0));
	Client(const std::string& IP = "127.0.0.1", const uint16_t port = 0);
	~Client();
public:
	CustomSocket::Result Connect(const CustomSocket::IPEndpoint& serverEndpoint);
	CustomSocket::Result Disconnect();

	CustomSocket::Result Recieve(void* data, int numberOfBytes);
	CustomSocket::Result Send(const void* data, int numberOfBytes);

	CustomSocket::IPEndpoint GetClientIPConfig();
protected:
	void Run();
	void Stop();

protected:
	void ProcessLoop();
	void ProcessRecieving();
	void ProcessSending();

protected:
	virtual void OnConnect();
	virtual void OnDisconnect();

	virtual void OnRecieve(char* data, int& bytesRecieved);
	virtual void OnSend(const char* data, int& bytesSent);
protected:
	ConnectionService m_service;

	bool m_isRunning = false;
	std::thread m_processThread;

	std::mutex m_coutMutex;
	std::mutex m_operationMutex;
};