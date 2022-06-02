#include "Server.h"

#include <iostream>
#include <memory>
#include <string>

Server::Server(const CustomSocket::IPEndpoint& IPconfig) :
	m_isRunning(false),
	m_getInfoEvent(CreateEvent(nullptr, TRUE, FALSE, L"ServiceEvent"))
{
	if (m_getInfoEvent == NULL)
	{
		throw std::exception();
	}

	if (CustomSocket::NetworkAPIInitializer::Initialize() != true)
	{
		throw std::exception();
	}

	if (m_listeningSocketService.m_socketInfo.first.Create() != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_listeningSocketService.m_socketInfo.first.SetSocketOption(
		CustomSocket::Option::IO_NonBlocking, TRUE) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	m_listeningSocketService.m_socketFD = WSAPOLLFD({
											  m_listeningSocketService.m_socketInfo.first.GetHandle(),
											  (POLLRDNORM),
											  0 });


	if (m_listeningSocketService.m_socketInfo.first.Bind(&IPconfig) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_listeningSocketService.m_socketInfo.first.GetSocketInfo(
		&m_listeningSocketService.m_socketInfo.second) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}
}

Server::Server(const std::string& ip, const uint16_t port) :
	m_isRunning(false),
	m_getInfoEvent(CreateEvent(nullptr, TRUE, FALSE, L"ServiceEvent"))
{
	if (m_getInfoEvent == NULL)
	{
		throw std::exception();
	}

	if (CustomSocket::NetworkAPIInitializer::Initialize() != true)
	{
		throw std::exception();
	}

	//m_listeningSocketService.m_socketInfo.second = CustomSocket::IPEndpoint(ip, port);

	if (m_listeningSocketService.m_socketInfo.first.Create() != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_listeningSocketService.m_socketInfo.first.SetSocketOption(
		CustomSocket::Option::IO_NonBlocking, TRUE) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	m_listeningSocketService.m_socketFD = WSAPOLLFD({
											  m_listeningSocketService.m_socketInfo.first.GetHandle(),
											  (POLLRDNORM),
											  0 });

	CustomSocket::IPEndpoint config(ip, port);
	if (m_listeningSocketService.m_socketInfo.first.Bind(&config) 
		!= CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_listeningSocketService.m_socketInfo.first.GetSocketInfo(
		&m_listeningSocketService.m_socketInfo.second) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}
}

Server::~Server()
{
	CustomSocket::NetworkAPIInitializer::Shutdown();
}

// MUTEX SAFE
CustomSocket::Result Server::Run()
{
	auto result = CustomSocket::Result::Fail;

	{
		std::lock_guard<std::mutex> operationLock(m_operationMutex);

		result = m_listeningSocketService.m_socketInfo.first.Listen();
	}

	if (result == CustomSocket::Result::Success)
	{
		{
			std::lock_guard<std::mutex> operationLock(m_operationMutex);

			m_isRunning = true;
			m_listenThread = std::thread(&Server::ProcessLoop, this);
		}
		

		std::lock_guard<std::mutex> coutLock(m_printLogMutex);

		std::cout << "[SERVICE INFO]: " << "Server successfully started." << std::endl;
	}
	else
	{
		std::lock_guard<std::mutex> coutLock(m_printLogMutex);

		std::cout << "[SERVICE INFO]: " << "Failed to start a server." << std::endl;
	}

	return result;
}

// MUTEX SAFE
CustomSocket::Result Server::Stop()
{
	{
		std::lock_guard<std::mutex> operationLock(m_operationMutex);

		m_isRunning = false;
		m_listenThread.join();
	}

	int index = 0;
	for (auto& connection : m_connections)
	{
		if (index >= m_connections.size())
		{
			break;
		}

		index += 1;
		Disconnect(connection.first.GetIPString(), connection.first.GetPort());
	}

	auto result = CustomSocket::Result::Fail;

	{
		std::lock_guard<std::mutex> operationLock(m_operationMutex);

		result = m_listeningSocketService.m_socketInfo.first.Close();
	}

	if (result == CustomSocket::Result::Success)
	{
		std::lock_guard<std::mutex> coutLock(m_printLogMutex);

		std::cout << "[SERVICE INFO]: " << "Server successfully stopped." << std::endl;
	}

	return result;
}

// MUTEX SAFE
CustomSocket::Result Server::Recieve(const std::string& ip, const uint16_t port,
									 void* data, int numberOfBytes)
{
	auto find_iter = m_connections.end();

	{
		std::lock_guard<std::mutex> operationLock(m_operationMutex);

		find_iter = m_connections.find(CustomSocket::IPEndpoint(ip, port));
	}

	auto result = (find_iter != m_connections.end()) ? CustomSocket::Result::Success :
		CustomSocket::Result::Fail;


	if (result == CustomSocket::Result::Success)
	{
		std::lock_guard<std::mutex> operationLock(m_operationMutex);
		
		find_iter->second.m_readBuffer = static_cast<char*>(data);
		find_iter->second.m_bytesToRecieve = numberOfBytes;
		find_iter->second.m_onRecieveFlag = true;
	}

	return result;
}

// MUTEX SAFE
CustomSocket::Result Server::Send(const std::string& ip, const uint16_t port,
								  const void* data, int numberOfBytes)
{
	auto find_iter = m_connections.end();

	{
		std::lock_guard<std::mutex> operationLock(m_operationMutex);

		find_iter = m_connections.find(CustomSocket::IPEndpoint(ip, port));
	}

	auto result = (find_iter != m_connections.end()) ? CustomSocket::Result::Success :
													   CustomSocket::Result::Fail;

	if (result == CustomSocket::Result::Success)
	{
		std::lock_guard<std::mutex> operationLock(m_operationMutex);

		find_iter->second.m_writeBuffer = static_cast<const char*>(data);
		find_iter->second.m_bytesToSend = numberOfBytes;
		find_iter->second.m_onSendFlag = true;
	}

	return result;
}

void Server::WaitForConnection()
{
	WaitForSingleObject(m_getInfoEvent, INFINITE);
}

std::vector<CustomSocket::IPEndpoint> Server::GetConnectionList()
{
	auto key_selector = [](auto& pair) { return pair.first; };
	std::vector<CustomSocket::IPEndpoint> result;

	for (auto& item : m_connections)
	{
		result.push_back(key_selector(item));
	}

	return result;
}

CustomSocket::IPEndpoint Server::GetServerIPConfig()
{
	return m_listeningSocketService.m_socketInfo.second;
}

// MUTEX SAFE
CustomSocket::Result Server::Disconnect(const std::string& ip, const uint16_t port)
{
	auto portToDisconnect = m_connections.end();
	{
		std::lock_guard<std::mutex> operationLock(m_operationMutex);

		portToDisconnect = m_connections.find(CustomSocket::IPEndpoint(ip, port));
	}

	auto result = (portToDisconnect != m_connections.end()) ? CustomSocket::Result::Success :
		CustomSocket::Result::Fail;

	if (result == CustomSocket::Result::Success)
	{
		OnDisconnect(portToDisconnect->second.m_socketInfo.second.GetIPString(),
					 portToDisconnect->second.m_socketInfo.second.GetPort());
		
		{
			std::lock_guard<std::mutex> operationLock(m_operationMutex);

			m_connections.erase(portToDisconnect);

			if (m_connections.empty())
			{
				ResetEvent(m_getInfoEvent);
			}
		}
	}

	return result;
}

// MUTEX SAFE
CustomSocket::Result Server::Connect() 
{
	CustomSocket::Socket newConnection;
	CustomSocket::IPEndpoint newConnectionEndpoint;

	CustomSocket::Result result = CustomSocket::Result::Fail;

	{
		std::lock_guard<std::mutex> operationLock(m_operationMutex);

		result = m_listeningSocketService.m_socketInfo.first.Accept(newConnection,
																	&newConnectionEndpoint);//MEMBER -> LOCK
	}

	if (result == CustomSocket::Result::Success)
	{
		result = newConnection.SetSocketOption(CustomSocket::Option::IO_NonBlocking, TRUE);

		if (result == CustomSocket::Result::Success)
		{
			ConnectionService newConnectionService({
				CONNECTION_INFO(std::move(newConnection), newConnectionEndpoint),
				{ INVALID_SOCKET, (POLLRDNORM | POLLWRNORM), 0},
				nullptr, 0, false,
				nullptr, 0, false
				});


			newConnectionService.m_socketFD.fd = newConnectionService.m_socketInfo.first.GetHandle();

			{
				std::lock_guard<std::mutex> coutLock(m_operationMutex);

				m_connections.insert({ newConnectionEndpoint, std::move(newConnectionService) });
				SetEvent(m_getInfoEvent);
			}

			OnConnect(newConnectionEndpoint.GetIPString(), newConnectionEndpoint.GetPort());
		}
	}
	else
	{
		std::lock_guard<std::mutex> coutLock(m_printLogMutex);

		std::cout << "[SERVICE INFO]: ";
		std::cout << "Failed to accept new connection." << std::endl;
	}

	return result;
}

void Server::ProcessLoop()
{
	while (m_isRunning == true)
	{
		size_t numOfAccuredEvents = WSAPoll(&m_listeningSocketService.m_socketFD, 1, 1);

		{
			//std::lock_guard<std::mutex> coutLock(m_operationMutex);

			for (auto& item : m_connections)
			{
				numOfAccuredEvents += WSAPoll(&item.second.m_socketFD, 1, 1);
			}
		}
		

		if (numOfAccuredEvents > 0)
		{
			if (m_listeningSocketService.m_socketFD.revents & POLLRDNORM)
			{
				if (Connect() != CustomSocket::Result::Success)
				{
					break;
				}

				if (numOfAccuredEvents > 1)
				{
					InspectAllConnections();
				}
			}
			else
			{
				InspectAllConnections();
			}
		}
	}
}

void Server::InspectAllConnections()
{
	for (auto& item : m_connections)
	{
		if (item.second.m_socketFD.revents & POLLRDNORM)
		{
			RecieveProcessing(item.second.m_socketInfo.second.GetIPString(),
				item.second.m_socketInfo.second.GetPort());
		}

		if (item.second.m_socketFD.revents & POLLWRNORM)
		{
			SendProcessing(item.second.m_socketInfo.second.GetIPString(),
				item.second.m_socketInfo.second.GetPort());
		}
	}
}

void Server::RecieveProcessing(const std::string& ip, const uint16_t port)
{
	auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;

	if (connection.m_onRecieveFlag == true)
	{
		int bytesRecieved = 0;

		if (connection.m_socketInfo.first.Recieve(connection.m_readBuffer,
			connection.m_bytesToRecieve,
			bytesRecieved)
			== CustomSocket::Result::Success)
		{
			if (bytesRecieved == 0)
			{
				//CLIENT WAS DISCONNECTED

				Disconnect(connection.m_socketInfo.second.GetIPString(),
					connection.m_socketInfo.second.GetPort());
			}
			else
			{
				connection.m_bytesToRecieve -= bytesRecieved;

				if (connection.m_bytesToRecieve <= 0)
				{
					OnRecieve(ip, port, connection.m_readBuffer, bytesRecieved);

					connection.m_bytesToRecieve = 0;
					connection.m_onRecieveFlag = false;
				}
			}
		}
	}
}

void Server::SendProcessing(const std::string& ip, const uint16_t port)
{
	auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;

	if (connection.m_onSendFlag == true)
	{
		int bytesSent = 0;

		if (connection.m_socketInfo.first.Send(connection.m_writeBuffer, 256, bytesSent)
			== CustomSocket::Result::Success)
		{
			if (bytesSent == 0)
			{
				//CLIENT WAS DISCONNECTED

				Disconnect(connection.m_socketInfo.second.GetIPString(),
					connection.m_socketInfo.second.GetPort());
			}
			else
			{
				connection.m_bytesToSend -= bytesSent;

				if (connection.m_bytesToSend <= 0)
				{
					OnSend(ip, port, connection.m_writeBuffer, bytesSent);

					connection.m_bytesToSend = 0;
					connection.m_onSendFlag = false;
				}
			}
		}
	}
}

void Server::OnSend(const std::string& ip, const uint16_t port, const char* data, int& bytesSent)
{
	//auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;

	std::lock_guard<std::mutex> coutLock(m_printLogMutex);

	std::cout << "[SERVICE INFO]: " << "{ " << bytesSent;
	std::cout << " bytes sent to port ";
	std::cout << static_cast<int>(port);
	std::cout << "} { MESSAGE = \"" << data << "\" }" << std::endl;
}

void Server::OnRecieve(const std::string& ip, const uint16_t port, char* data, int& bytesRecieved)
{
	//auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;

	std::lock_guard<std::mutex> coutLock(m_printLogMutex);

	std::cout << "[CLIENT]: ";
	std::cout << "{ " << bytesRecieved << " bytes recieved from port ";
	std::cout << static_cast<int>(port) << "} ";
	std::cout << "{ MESSAGE = \"" << data << "\" } " << std::endl;

}

void Server::OnConnect(const std::string& ip, const uint16_t port)
{
	std::lock_guard<std::mutex> coutLock(m_printLogMutex);

	std::cout << "[CLIENT]: " << "{IP = " << ip;
	std::cout << "} {PORT = " << port << "} ";
	std::cout << "{STATUS = CONNECTED}" << std::endl;

	std::cout << "[SERVICE INFO]: " << "Pool size = ";
	std::cout << m_connections.size() << "." << std::endl;
}

void Server::OnDisconnect(const std::string& ip, const uint16_t port)
{
	std::lock_guard<std::mutex> coutLock(m_printLogMutex);

	std::cout << "[CLIENT]: " << "{IP = ";
	std::cout << ip;
	std::cout << "} {PORT = " << port << "} ";
	std::cout << "{STATUS = DISCONNECTED}" << std::endl;
}