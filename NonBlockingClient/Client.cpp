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

	auto result = ((m_service.m_socketFD.revents & POLLWRNORM) && 
				  (m_service.m_recieveMessage.m_buffer == nullptr)) ?
													CustomSocket::Result::Success :
													CustomSocket::Result::Fail;


	if (result == CustomSocket::Result::Success)
	{
		//std::lock_guard<std::mutex> operationLock(m_operationMutex);

		m_service.m_recieveMessage.m_buffer = static_cast<char*>(data);
		m_service.m_recieveMessage.m_bytesToProcess = numberOfBytes;
		m_service.m_recieveMessage.m_bufferTotalSize = numberOfBytes;
		m_service.m_recieveMessage.m_onProcessFlag = true;
	}

	return result;
}

CustomSocket::Result Client::Send(const void* data, int numberOfBytes)
{
	std::lock_guard<std::mutex> operationLock(m_operationMutex);

	auto result = ((m_service.m_socketFD.revents & POLLWRNORM) && 
				  (m_service.m_sendMessage.m_buffer == nullptr)) 
														? CustomSocket::Result::Success
														: CustomSocket::Result::Fail;


	if (result == CustomSocket::Result::Success)
	{
		m_service.m_sendMessage.m_buffer = static_cast<const char*>(data);
		m_service.m_sendMessage.m_bytesToProcess = numberOfBytes;
		m_service.m_sendMessage.m_bufferTotalSize = numberOfBytes;
		m_service.m_sendMessage.m_onProcessFlag = true;
	}

	return result;
}

CustomSocket::IPEndpoint Client::GetClientIPConfig()
{
	return m_service.m_socketInfo.second;
}

CustomSocket::Result Client::WaitOnRecieveEvent()
{
	WaitForSingleObject(m_service.m_onRecieveEvent, INFINITE);
	auto result = (ResetEvent(m_service.m_onRecieveEvent) == TRUE) ?
															CustomSocket::Result::Success :
															CustomSocket::Result::Fail;

	return result;
}

CustomSocket::Result Client::WaitOnSendEvent()
{
	WaitForSingleObject(m_service.m_onSendEvent, INFINITE);
	auto result = (ResetEvent(m_service.m_onSendEvent) == TRUE) ? 
															CustomSocket::Result::Success :
															CustomSocket::Result::Fail;

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
		(m_service.m_recieveMessage.m_onProcessFlag) == true)
	{
		int bytesRecieved = 0;
		int offset = (m_service.m_recieveMessage.m_bufferTotalSize -
					  m_service.m_recieveMessage.m_bytesToProcess);

		//SMTH SHOULD BE READ
		if (m_service.m_socketInfo.first.Recieve(
			m_service.m_recieveMessage.m_buffer + offset,
			m_service.m_recieveMessage.m_bytesToProcess,
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
				m_service.m_recieveMessage.m_bytesToProcess -= bytesRecieved;

				if (m_service.m_recieveMessage.m_bytesToProcess <= 0)
				{
					int tmp_buffer_size = m_service.m_recieveMessage.m_bufferTotalSize;
					char* tmp_buffer = new char[tmp_buffer_size];
					memcpy_s(tmp_buffer,
							 tmp_buffer_size,
							 m_service.m_recieveMessage.m_buffer,
							 m_service.m_recieveMessage.m_bufferTotalSize);

					m_service.m_recieveMessage.m_buffer = nullptr;
					m_service.m_recieveMessage.m_bytesToProcess = 0;
					m_service.m_recieveMessage.m_bufferTotalSize = 0;
					m_service.m_recieveMessage.m_onProcessFlag = false;

					OnRecieve(tmp_buffer, tmp_buffer_size);

					delete[] tmp_buffer;
				}
			}
		}
	}
}

void Client::ProcessSending()
{
	std::lock_guard<std::mutex> operationLock(m_operationMutex);

	if ((m_service.m_socketFD.revents & POLLWRNORM) &&
		(m_service.m_sendMessage.m_onProcessFlag) == true)
	{
		int bytesSent = 0;
		int offset = (m_service.m_sendMessage.m_bufferTotalSize -
					  m_service.m_sendMessage.m_bytesToProcess);

		//SMTH SHOULD BE READ
		if (m_service.m_socketInfo.first.Send(
			m_service.m_sendMessage.m_buffer + offset,
			m_service.m_sendMessage.m_bytesToProcess,
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
				m_service.m_sendMessage.m_bytesToProcess -= bytesSent;

				if (m_service.m_sendMessage.m_bytesToProcess <= 0)
				{
					int tmp_buffer_size = m_service.m_sendMessage.m_bufferTotalSize;
					char* tmp_buffer = new char[tmp_buffer_size];
					memcpy_s(tmp_buffer,
						tmp_buffer_size,
						m_service.m_sendMessage.m_buffer,
						m_service.m_sendMessage.m_bufferTotalSize);

					m_service.m_sendMessage.m_buffer = nullptr;
					m_service.m_sendMessage.m_bytesToProcess = 0;
					m_service.m_sendMessage.m_bufferTotalSize = 0;
					m_service.m_sendMessage.m_onProcessFlag = false;

					OnSend(tmp_buffer, tmp_buffer_size);

					delete[] tmp_buffer;
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

	SetEvent(m_service.m_onRecieveEvent);
}

void Client::OnSend(const char* data, int& bytesSent)
{
	//std::lock_guard<std::mutex> operationLock(m_operationMutex);
	std::lock_guard<std::mutex> coutLock(m_coutMutex);

	std::cout << "[SERVICE INFO]: " << "{ " << bytesSent;
	std::cout << " bytes sent } { MESSAGE = \"" << data << "\" }" << std::endl;

	SetEvent(m_service.m_onSendEvent);
}
