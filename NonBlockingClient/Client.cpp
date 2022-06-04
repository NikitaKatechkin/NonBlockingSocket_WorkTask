#include "Client.h"
#include <iostream>

Client::Client(const CustomSocket::IPEndpoint& clientConfig)
{
	if (CustomSocket::NetworkAPIInitializer::Initialize() == false)
	{
		throw std::exception();
	}

	if (m_service.m_socketInfo.first.Create() != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_service.m_socketInfo.first.SetSocketOption(
		CustomSocket::Option::IO_NonBlocking, TRUE)
		!= CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_service.m_socketInfo.first.Bind(clientConfig)
		!= CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	m_service.m_socketFD = WSAPOLLFD({m_service.m_socketInfo.first.GetHandle(),
									  (POLLRDNORM | POLLWRNORM),
									  0 });

	if (m_service.m_socketInfo.first.GetSocketInfo(
		m_service.m_socketInfo.second) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	Run();
}

Client::Client(const std::string& IP, const uint16_t port)
{
	if (CustomSocket::NetworkAPIInitializer::Initialize() == false)
	{
		throw std::exception();
	}

	if (m_service.m_socketInfo.first.Create() != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_service.m_socketInfo.first.SetSocketOption(
		CustomSocket::Option::IO_NonBlocking, TRUE)
		!= CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_service.m_socketInfo.first.Bind(CustomSocket::IPEndpoint(IP, port))
		!= CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	m_service.m_socketFD = WSAPOLLFD({ m_service.m_socketInfo.first.GetHandle(),
									  (POLLRDNORM | POLLWRNORM),
									  0 });

	if (m_service.m_socketInfo.first.GetSocketInfo(
		m_service.m_socketInfo.second) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	Run();
}

Client::~Client()
{
	if (m_isRunning == true)
	{
		Stop();
	}

	CustomSocket::NetworkAPIInitializer::Shutdown();
}

CustomSocket::Result Client::Connect(const CustomSocket::IPEndpoint& serverEndpoint)
{
	auto result = (m_service.m_socketFD.revents & POLLWRNORM) ? 
													CustomSocket::Result::Fail :
													CustomSocket::Result::Success;

	if (result == CustomSocket::Result::Success)
	{
		if ((m_service.m_socketInfo.first.Connect(serverEndpoint) 
			== CustomSocket::Result::Fail) && 
			(WSAGetLastError() == WSAEWOULDBLOCK))
		{
			result = CustomSocket::Result::Success;
		}
		else
		{
			result = CustomSocket::Result::Fail;
		}
	}

	return result;
}

CustomSocket::Result Client::Disconnect()
{
	auto result = (m_service.m_socketFD.revents & POLLWRNORM) ? CustomSocket::Result::Success :
																CustomSocket::Result::Fail;
	if (result == CustomSocket::Result::Success)
	{
		result = m_service.m_socketInfo.first.Close();
	}

	return result;
}

CustomSocket::Result Client::Recieve(void* data, int numberOfBytes)
{
	std::lock_guard<std::mutex> operationLock(m_operationMutex);

	auto result = (m_service.m_socketFD.revents & POLLWRNORM) && (m_service.m_readBuffer == nullptr) ? 
																	CustomSocket::Result::Success :
																	CustomSocket::Result::Fail;


	if (result == CustomSocket::Result::Success)
	{
		//std::lock_guard<std::mutex> operationLock(m_operationMutex);

		m_service.m_readBuffer = static_cast<char*>(data);
		m_service.m_bytesToRecieve = numberOfBytes;
		m_service.m_onRecieveFlag = true;
	}

	return result;
}

CustomSocket::Result Client::Send(const void* data, int numberOfBytes)
{
	std::lock_guard<std::mutex> operationLock(m_operationMutex);

	auto result = (m_service.m_socketFD.revents & POLLWRNORM) && (m_service.m_writeBuffer == nullptr) 
																	? CustomSocket::Result::Success 
																	: CustomSocket::Result::Fail;


	if (result == CustomSocket::Result::Success)
	{
		m_service.m_writeBuffer = static_cast<const char*>(data);
		m_service.m_bytesToSend = numberOfBytes;
		m_service.m_onSendFlag = true;
	}

	return result;
}

void Client::Run()
{
	m_isRunning = true;
	m_processThread = std::thread(&Client::ProcessLoop, this);
}

void Client::Stop()
{
	m_isRunning = false;
	m_processThread.join();
}

void Client::ProcessLoop()
{
	bool isConnected = false;

	while (m_isRunning == true)
	{
		size_t numOfAccuredEvents = WSAPoll(&m_service.m_socketFD, 1, 1);

		if (numOfAccuredEvents > 0)
		{
			/**
			if (m_service.m_socketFD.revents & POLLWRNORM)
			{
				if (isConnected == false)
				{
					OnConnect();
				}
				isConnected = true;
			}
			else
			{
				if (isConnected == true)
				{
					OnDisconnect();
				}
				isConnected = false;
			}
			**/

			bool currentConnectionStatus = m_service.m_socketFD.revents & POLLWRNORM;
			if (currentConnectionStatus != isConnected)
			{
				(isConnected == true) ? OnDisconnect() : OnConnect();
			}
			isConnected = currentConnectionStatus;

			if (isConnected == true)
			{
				ProcessRecieving();
				ProcessSending();
			}
		}
	}
}

void Client::ProcessRecieving()
{
	std::lock_guard<std::mutex> operationLock(m_operationMutex);

	if ((m_service.m_socketFD.revents & POLLRDNORM) && 
		(m_service.m_onRecieveFlag) == true)
	{
		int bytesRecieved = 0;
		//SMTH SHOULD BE READ
		if (m_service.m_socketInfo.first.Recieve(m_service.m_readBuffer,
												 m_service.m_bytesToRecieve,
												 bytesRecieved)
			== CustomSocket::Result::Success)
		{
			if (bytesRecieved == 0)
			{
				//SERVER WAS DISCONNECTED

				Disconnect();
			}
			else
			{
				m_service.m_bytesToRecieve -= bytesRecieved;

				if (m_service.m_bytesToRecieve <= 0)
				{
					OnRecieve(m_service.m_readBuffer, bytesRecieved);

					m_service.m_readBuffer = nullptr;
					m_service.m_bytesToRecieve = 0;
					m_service.m_onRecieveFlag = false;
				}
			}
		}
	}
}

void Client::ProcessSending()
{
	std::lock_guard<std::mutex> operationLock(m_operationMutex);

	if ((m_service.m_socketFD.revents & POLLWRNORM) &&
		(m_service.m_onSendFlag) == true)
	{
		int bytesSent = 0;
		//SMTH SHOULD BE READ
		if (m_service.m_socketInfo.first.Send(m_service.m_writeBuffer,
											  m_service.m_bytesToSend,
											  bytesSent)
			== CustomSocket::Result::Success)
		{
			if (bytesSent == 0)
			{
				//SERVER WAS DISCONNECTED

				Disconnect();
			}
			else
			{
				m_service.m_bytesToSend -= bytesSent;

				if (m_service.m_bytesToSend <= 0)
				{
					OnSend(m_service.m_writeBuffer, bytesSent);

					m_service.m_writeBuffer = nullptr;
					m_service.m_bytesToSend = 0;
					m_service.m_onSendFlag = false;
				}
			}
		}
	}
}

void Client::OnConnect()
{
	//std::lock_guard<std::mutex> operationLock(m_operationMutex);
	std::lock_guard<std::mutex> coutLock(m_coutMutex);

	std::cout << "[SERVICE INFO]: ";// << "{IP = " << IP;
	//std::cout << "} {PORT = " << port << "} ";
	std::cout << "{STATUS = CONNECTED}" << std::endl;
}

void Client::OnDisconnect()
{
	//std::lock_guard<std::mutex> operationLock(m_operationMutex);
	std::lock_guard<std::mutex> coutLock(m_coutMutex);

	std::cout << "[SERVICE INFO]: ";// << "{IP = " << IP;
	//std::cout << "} {PORT = " << port << "} ";
	std::cout << "{STATUS = DISCONNECTED}" << std::endl;
}

void Client::OnRecieve(char* data, int& bytesRecieved)
{
	//std::lock_guard<std::mutex> operationLock(m_operationMutex);
	std::lock_guard<std::mutex> coutLock(m_coutMutex);

	std::cout << "[SERVER]: " << "{ " << bytesRecieved;
	std::cout << " bytes recieved } { MESSAGE = \"" << data << "\" }" << std::endl;
}

void Client::OnSend(const char* data, int& bytesSent)
{
	//std::lock_guard<std::mutex> operationLock(m_operationMutex);
	std::lock_guard<std::mutex> coutLock(m_coutMutex);

	std::cout << "[SERVICE INFO]: " << "{ " << bytesSent;
	std::cout << " bytes sent } { MESSAGE = \"" << data << "\" }" << std::endl;
}
