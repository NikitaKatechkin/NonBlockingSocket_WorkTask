#pragma once

#include <NonBlockingSocket/IncludeMe.h>
#include <thread>
#include <mutex>

class Client
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

		/**
		const char* m_writeBuffer = nullptr;
		int m_writeBufferTotalSize = 0;
		int m_bytesToSend = 0;

		bool m_onSendFlag = false;

		char* m_readBuffer = nullptr;
		int m_readBufferTotalSize = 0;
		int m_bytesToRecieve = 0;

		bool m_onRecieveFlag = false;
		**/

		ServerMessage<char*> m_recieveMessage;
		HANDLE m_onRecieveEvent = CreateEvent(nullptr, TRUE, FALSE, L"OnRecieveEvent");

		ServerMessage<const char*> m_sendMessage;
		HANDLE m_onSendEvent = CreateEvent(nullptr, TRUE, FALSE, L"OnSendEvent");

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

	CustomSocket::Result WaitOnRecieveEvent();
	CustomSocket::Result WaitOnSendEvent();

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