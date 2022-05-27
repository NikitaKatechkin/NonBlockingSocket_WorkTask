#include "Server.h"

#include <iostream>
#include <memory>
#include <string>

//static const size_t LISTENING_FD_INDEX = 0;

/**
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
	m_socketService.push_back(ConnectionService());
	m_socketService[LISTENING_FD_INDEX].m_socketInfo.second = IPconfig;
	if (m_socketService[LISTENING_FD_INDEX].m_socketInfo.first.Create()
		!= CustomSocket::Result::Success)
	{
		throw std::exception();
	}
	if (m_socketService[LISTENING_FD_INDEX].m_socketInfo.first.SetSocketOption(
					CustomSocket::Option::IO_NonBlocking, TRUE) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}
	m_socketService[LISTENING_FD_INDEX].m_socketFD = WSAPOLLFD(
								{ m_socketService[LISTENING_FD_INDEX].m_socketInfo.first.GetHandle(),
								  (POLLRDNORM),
								  0 });
	m_socketService[LISTENING_FD_INDEX].m_readBuffer = nullptr;
	m_socketService[LISTENING_FD_INDEX].m_bytesToRecieve = 0;
	m_socketService[LISTENING_FD_INDEX].m_onRecieveFlag = false;
	m_socketService[LISTENING_FD_INDEX].m_writeBuffer = nullptr;
	m_socketService[LISTENING_FD_INDEX].m_bytesToSend = 0;
	m_socketService[LISTENING_FD_INDEX].m_onSendFlag = false;
}
**/

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

	m_listeningSocketService.m_socketInfo.second = IPconfig;

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
}

/**
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
	m_socketService.push_back(ConnectionService());
	m_socketService[LISTENING_FD_INDEX].m_socketInfo.second = CustomSocket::IPEndpoint(ip, port);
	if (m_socketService[LISTENING_FD_INDEX].m_socketInfo.first.Create()
		!= CustomSocket::Result::Success)
	{
		throw std::exception();
	}
	if (m_socketService[LISTENING_FD_INDEX].m_socketInfo.first.SetSocketOption(
		CustomSocket::Option::IO_NonBlocking, TRUE) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}
	m_socketService[LISTENING_FD_INDEX].m_socketFD = WSAPOLLFD(
		{ m_socketService[LISTENING_FD_INDEX].m_socketInfo.first.GetHandle(),
		  (POLLRDNORM),
		  0 });
	m_socketService[LISTENING_FD_INDEX].m_readBuffer = nullptr;
	m_socketService[LISTENING_FD_INDEX].m_bytesToRecieve = 0;
	m_socketService[LISTENING_FD_INDEX].m_onRecieveFlag = false;
	m_socketService[LISTENING_FD_INDEX].m_writeBuffer = nullptr;
	m_socketService[LISTENING_FD_INDEX].m_bytesToSend = 0;
	m_socketService[LISTENING_FD_INDEX].m_onSendFlag = false;
}
**/

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

	m_listeningSocketService.m_socketInfo.second = CustomSocket::IPEndpoint(ip, port);

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
}


Server::~Server()
{
	CustomSocket::NetworkAPIInitializer::Shutdown();
}

/**
CustomSocket::Result Server::run()
{
	auto result = m_socketService[LISTENING_FD_INDEX].m_socketInfo.first.Listen(
											m_socketService[LISTENING_FD_INDEX].m_socketInfo.second);
	if (result == CustomSocket::Result::Success)
	{
		m_isRunning = true;
		m_listenThread = std::thread(&Server::processLoop, this);
		std::cout << "[SERVICE INFO]: " << "Server successfully started." << std::endl;
	}
	else
	{
		std::cout << "[SERVICE INFO]: " << "Failed to start a server." << std::endl;
	}
	return result;
}
**/

CustomSocket::Result Server::run()
{
	auto result = m_listeningSocketService.m_socketInfo.first.Listen(
		m_listeningSocketService.m_socketInfo.second);

	if (result == CustomSocket::Result::Success)
	{
		m_isRunning = true;
		m_listenThread = std::thread(&Server::processLoop, this);

		std::cout << "[SERVICE INFO]: " << "Server successfully started." << std::endl;
	}
	else
	{
		std::cout << "[SERVICE INFO]: " << "Failed to start a server." << std::endl;
	}

	return result;
}

/**
CustomSocket::Result Server::stop()
{
	m_isRunning = false;
	m_listenThread.join();
	auto result = m_socketService[LISTENING_FD_INDEX].m_socketInfo.first.Close();
	if (result == CustomSocket::Result::Success)
	{
		std::cout << "[SERVICE INFO]: " << "Server successfully stopped." << std::endl;
	}
	return result;
}
**/

CustomSocket::Result Server::stop()
{
	m_isRunning = false;
	m_listenThread.join();

	auto result = m_listeningSocketService.m_socketInfo.first.Close();
	if (result == CustomSocket::Result::Success)
	{
		std::cout << "[SERVICE INFO]: " << "Server successfully stopped." << std::endl;
	}

	//m_connections.clear();

	return result;
}

/**
CustomSocket::Result Server::recieve(const std::string& ip, const uint16_t port,
									 void* data, int numberOfBytes)
{
	std::string correct_ip;
	correct_ip = ip;
	correct_ip.resize(16);
	auto find_iter = std::find_if(
		m_socketService.begin(),
		m_socketService.end(),
		[&port, &correct_ip](ConnectionService info) {
			return ((info.m_socketInfo.second.GetPort() == port) &&
					(info.m_socketInfo.second.GetIPString() == correct_ip)); });
	auto result = (find_iter != m_socketService.end()) ? CustomSocket::Result::Success :
														 CustomSocket::Result::Fail;
	if (result == CustomSocket::Result::Success)
	{
		find_iter->m_readBuffer = static_cast<char*>(data);
		find_iter->m_bytesToRecieve = numberOfBytes;
		find_iter->m_onRecieveFlag = true;
	}
	return result;
}
**/

CustomSocket::Result Server::recieve(const std::string& ip, const uint16_t port,
	void* data, int numberOfBytes)
{
	std::string correct_ip = ip;
	correct_ip.resize(16);

	auto find_iter = m_connections.find(CustomSocket::IPEndpoint(correct_ip, port));

	auto result = (find_iter != m_connections.end()) ? CustomSocket::Result::Success :
		CustomSocket::Result::Fail;


	if (result == CustomSocket::Result::Success)
	{
		find_iter->second.m_readBuffer = static_cast<char*>(data);
		find_iter->second.m_bytesToRecieve = numberOfBytes;
		find_iter->second.m_onRecieveFlag = true;
	}

	return result;
}

/**
CustomSocket::Result Server::send(const std::string& ip, const uint16_t port,
								  const void* data, int numberOfBytes)
{
	std::string correct_ip;
	correct_ip = ip;
	correct_ip.resize(16);
	auto find_iter = std::find_if(
		m_socketService.begin(),
		m_socketService.end(),
		[&port, &correct_ip](ConnectionService info) {
			return ((info.m_socketInfo.second.GetPort() == port) &&
					(info.m_socketInfo.second.GetIPString() == correct_ip)); });
	auto result = (find_iter != m_socketService.end()) ? CustomSocket::Result::Success :
														 CustomSocket::Result::Fail;
	if (result == CustomSocket::Result::Success)
	{
		find_iter->m_writeBuffer = static_cast<const char*>(data);
		find_iter->m_bytesToSend = numberOfBytes;
		find_iter->m_onSendFlag = true;
	}
	return result;
}
**/

CustomSocket::Result Server::send(const std::string& ip, const uint16_t port,
	const void* data, int numberOfBytes)
{
	std::string correct_ip = ip;
	correct_ip.resize(16);

	auto find_iter = m_connections.find(CustomSocket::IPEndpoint(correct_ip, port));

	auto result = (find_iter != m_connections.end()) ? CustomSocket::Result::Success :
		CustomSocket::Result::Fail;

	if (result == CustomSocket::Result::Success)
	{
		find_iter->second.m_writeBuffer = static_cast<const char*>(data);
		find_iter->second.m_bytesToSend = numberOfBytes;
		find_iter->second.m_onSendFlag = true;
	}

	return result;
}

/**
void Server::waitForConnection()
{
	WaitForSingleObject(m_getInfoEvent, INFINITE);
}
**/

void Server::waitForConnection()
{
	WaitForSingleObject(m_getInfoEvent, INFINITE);
}

/**
CustomSocket::Result Server::disconnect(const uint16_t port)
{
	auto portToDisconnect = std::find_if(
		m_socketService.begin(),
		m_socketService.end(),
		[&port](ConnectionService info) { return info.m_socketInfo.second.GetPort() == port; });
	auto result = (portToDisconnect != m_socketService.end()) ? CustomSocket::Result::Success :
																CustomSocket::Result::Fail;
	if (result == CustomSocket::Result::Success)
	{
		{
			//std::lock_guard<std::mutex> print_lock(m_printLogMutex);
			std::cout << "[CLIENT]: " << "{IP = ";
			std::cout << portToDisconnect->m_socketInfo.second.GetIPString();
			std::cout << "} {PORT = " << portToDisconnect->m_socketInfo.second.GetPort() << "} ";
			std::cout << "{STATUS = DISCONNECTED}" << std::endl;
		}
		m_socketService.erase(portToDisconnect);
	}
	return result;
}
**/

CustomSocket::Result Server::disconnect(const std::string& ip, const uint16_t port)
{
	auto portToDisconnect = m_connections.find(CustomSocket::IPEndpoint(ip, port));

	auto result = (portToDisconnect != m_connections.end()) ? CustomSocket::Result::Success :
		CustomSocket::Result::Fail;

	if (result == CustomSocket::Result::Success)
	{
		{
			//std::lock_guard<std::mutex> print_lock(m_printLogMutex);

			std::cout << "[CLIENT]: " << "{IP = ";
			std::cout << portToDisconnect->second.m_socketInfo.second.GetIPString();
			std::cout << "} {PORT = " << portToDisconnect->second.m_socketInfo.second.GetPort() << "} ";
			std::cout << "{STATUS = DISCONNECTED}" << std::endl;
		}

		m_connections.erase(portToDisconnect);
	}

	return result;
}

/**
CustomSocket::Result Server::connect()
{
	//TODO:
	//MUST BE MOVE SEMANTICS
	//AND
	//REDUCE MULTIPLE BACK() CALLBACKS
	m_socketService.push_back(ConnectionService());
	auto result = m_socketService[LISTENING_FD_INDEX].m_socketInfo.first.Accept(
														m_socketService.back().m_socketInfo.first,
														&m_socketService.back().m_socketInfo.second);
	if (result == CustomSocket::Result::Success)
	{
		CustomSocket::Socket& newConnection = m_socketService.back().m_socketInfo.first;
		CustomSocket::IPEndpoint& newConnectionEndpoint = m_socketService.back().m_socketInfo.second;
		//ACCEPTING-REGISTRATING ROUTINE STARTS
		result = newConnection.SetSocketOption(CustomSocket::Option::IO_NonBlocking, TRUE);
		if (result == CustomSocket::Result::Success)
		{
			m_socketService.back().m_socketFD = { newConnection.GetHandle(),
												  (POLLRDNORM | POLLWRNORM),
												  0 };
			m_socketService.back().m_onRecieveFlag = false;
			m_socketService.back().m_readBuffer = nullptr;
			m_socketService.back().m_bytesToRecieve = 0;
			m_socketService.back().m_onSendFlag = false;
			m_socketService.back().m_writeBuffer = nullptr;
			m_socketService.back().m_bytesToSend = 0;
			SetEvent(m_getInfoEvent);
			std::cout << "[CLIENT]: " << "{IP = " << newConnectionEndpoint.GetIPString();
			std::cout << "} {PORT = " << newConnectionEndpoint.GetPort() << "} ";
			std::cout << "{STATUS = CONNECTED}" << std::endl;
			std::cout << "[SERVICE INFO]: " << "Pool size = ";
			std::cout << m_socketService.size() << "." << std::endl;
		}
		//ACCEPTING-REGISTRATING ROUTINE ENDS
	}
	else
	{
		m_socketService.pop_back();
		std::cout << "[SERVICE INFO]: ";
		std::cout << "Failed to accept new connection." << std::endl;
	}
	return result;
}
**/

CustomSocket::Result Server::connect()
{
	//TODO:
	//MUST BE MOVE SEMANTICS
	//AND
	//REDUCE MULTIPLE BACK() CALLBACKS
	CustomSocket::Socket newConnection;
	CustomSocket::IPEndpoint newConnectionEndpoint;

	auto result = m_listeningSocketService.m_socketInfo.first.Accept(newConnection,
		&newConnectionEndpoint);

	if (result == CustomSocket::Result::Success)
	{
		//ACCEPTING-REGISTRATING ROUTINE STARTS

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
			m_connections.insert({ newConnectionEndpoint, std::move(newConnectionService) });

			SetEvent(m_getInfoEvent);

			std::cout << "[CLIENT]: " << "{IP = " << newConnectionEndpoint.GetIPString();
			std::cout << "} {PORT = " << newConnectionEndpoint.GetPort() << "} ";
			std::cout << "{STATUS = CONNECTED}" << std::endl;

			std::cout << "[SERVICE INFO]: " << "Pool size = ";
			std::cout << m_connections.size() << "." << std::endl;
		}


		//ACCEPTING-REGISTRATING ROUTINE ENDS
	}
	else
	{
		std::cout << "[SERVICE INFO]: ";
		std::cout << "Failed to accept new connection." << std::endl;
	}

	return result;
}

/**
void Server::processLoop()
{
	int numOfAccuredEvents = 0;
	while (m_isRunning == true)
	{
		size_t numOfAccuredEvents = 0;
		for (auto& item : m_socketService)
		{
			numOfAccuredEvents += WSAPoll(&item.m_socketFD, 1, 1);
		}
		if (numOfAccuredEvents > 0)
		{
			if (m_socketService[LISTENING_FD_INDEX].m_socketFD.revents & POLLRDNORM)
			{
				if (connect() != CustomSocket::Result::Success)
				{
					break;
				}
				if (numOfAccuredEvents > 1)
				{
					inspectAllConnections();
				}
			}
			else
			{
				inspectAllConnections();
			}
		}
		if (m_socketService.size() == 1)
		{
			ResetEvent(m_getInfoEvent);
		}
	}
}
**/

void Server::processLoop()
{
	//int numOfAccuredEvents = 0;

	while (m_isRunning == true)
	{
		size_t numOfAccuredEvents = WSAPoll(&m_listeningSocketService.m_socketFD, 1, 1);


		for (auto& item : m_connections)
		{
			numOfAccuredEvents += WSAPoll(&item.second.m_socketFD, 1, 1);
		}

		if (numOfAccuredEvents > 0)
		{
			if (m_listeningSocketService.m_socketFD.revents & POLLRDNORM)
			{
				if (connect() != CustomSocket::Result::Success)
				{
					break;
				}

				if (numOfAccuredEvents > 1)
				{
					inspectAllConnections();
				}
			}
			else
			{
				inspectAllConnections();
			}
		}

		if (m_connections.empty())
		{
			ResetEvent(m_getInfoEvent);
		}
	}
}

/**
void Server::inspectAllConnections()
{
	for (size_t index = LISTENING_FD_INDEX + 1; index < m_socketService.size(); index++)
	{
		ConnectionService& item = m_socketService[index];
		if (item.m_socketFD.revents & POLLRDNORM)
		{
			OnRecieve(item.m_socketInfo.second.GetIPString(),
					  item.m_socketInfo.second.GetPort());
		}
		if (item.m_socketFD.revents & POLLWRNORM)
		{
			OnSend(item.m_socketInfo.second.GetIPString(),
				   item.m_socketInfo.second.GetPort());
		}
	}
}
**/

void Server::inspectAllConnections()
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

/**
void Server::OnRecieve(const std::string& ip, const uint16_t port)
{
	auto connection = std::find_if(
		m_socketService.begin(),
		m_socketService.end(),
		[&port, &ip](ConnectionService info) {
			return ((info.m_socketInfo.second.GetPort() == port) &&
					(info.m_socketInfo.second.GetIPString() == ip)); });
	if (connection->m_onRecieveFlag == true)
	{
		int bytesRecieved = 0;
		if (connection->m_socketInfo.first.Recieve(connection->m_readBuffer,
												   connection->m_bytesToRecieve,
												   bytesRecieved)
			== CustomSocket::Result::Success)
		{
			if (bytesRecieved == 0)
			{
				//CLIENT WAS DISCONNECTED
				disconnect(connection->m_socketInfo.second.GetPort());
			}
			else
			{
				connection->m_bytesToRecieve -= bytesRecieved;
				if (connection->m_bytesToRecieve <= 0)
				{
					std::cout << "[CLIENT]: " << (connection->m_readBuffer) << std::endl;
					connection->m_bytesToRecieve = 0;
					connection->m_onRecieveFlag = false;
				}
			}
			//TODO: remove
			connection->m_onSendFlag = true;
		}
	}
}
**/

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

				disconnect(connection.m_socketInfo.second.GetIPString(),
					connection.m_socketInfo.second.GetPort());
			}
			else
			{
				connection.m_bytesToRecieve -= bytesRecieved;

				if (connection.m_bytesToRecieve <= 0)
				{
					//Here to be REAL OnRecieve();
					/**
					std::cout << "[CLIENT]: " << (connection.m_readBuffer) << std::endl;
					**/

					OnRecieve(ip, port, bytesRecieved);

					connection.m_bytesToRecieve = 0;
					connection.m_onRecieveFlag = false;
				}
			}


			//TODO: remove
			connection.m_onSendFlag = true;
		}
	}
}

/**
void Server::OnSend(const std::string& ip, const uint16_t port)
{
	auto connection = std::find_if(
		m_socketService.begin(),
		m_socketService.end(),
		[&port, &ip](ConnectionService info) {
			return ((info.m_socketInfo.second.GetPort() == port) &&
					(info.m_socketInfo.second.GetIPString() == ip)); });
	if (connection->m_onSendFlag == true)
	{
		int bytesSent = 0;
		if (connection->m_socketInfo.first.Send(connection->m_writeBuffer, 256, bytesSent)
			== CustomSocket::Result::Success)
		{
			if (bytesSent == 0)
			{
				//CLIENT WAS DISCONNECTED
				disconnect(connection->m_socketInfo.second.GetPort());
			}
			else
			{
				connection->m_bytesToSend -= bytesSent;
				if (connection->m_bytesToSend <= 0)
				{
					std::cout << "[SERVICE INFO]: " << "{ " << bytesSent;
					std::cout << " bytes sent from port ";
					std::cout << static_cast<int>(connection->m_socketInfo.second.GetPort());
					std::cout << "}" << std::endl;
					connection->m_bytesToSend = 0;
					connection->m_onSendFlag = false;
				}
			}
		}
	}
}
**/

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

				disconnect(connection.m_socketInfo.second.GetIPString(),
					connection.m_socketInfo.second.GetPort());
			}
			else
			{
				connection.m_bytesToSend -= bytesSent;

				if (connection.m_bytesToSend <= 0)
				{
					//Here to be REAL OnSent();

					/**
					std::cout << "[SERVICE INFO]: " << "{ " << bytesSent;
					std::cout << " bytes sent from port ";
					std::cout << static_cast<int>(connection.m_socketInfo.second.GetPort());
					std::cout << "}" << std::endl;
					**/
					OnSend(ip, port, bytesSent);

					connection.m_bytesToSend = 0;
					connection.m_onSendFlag = false;
				}
			}
		}
	}
}

void Server::OnSend(const std::string& ip, const uint16_t port, int& bytesSent)
{
	auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;

	std::cout << "[SERVICE INFO]: " << "{ " << bytesSent;
	std::cout << " bytes sent from port ";
	std::cout << static_cast<int>(connection.m_socketInfo.second.GetPort());
	std::cout << "}" << std::endl;
}

void Server::OnRecieve(const std::string& ip, const uint16_t port, int& bytesRecieved)
{
	auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;

	std::cout << "[CLIENT]: " << "{ MESSAGE = \"" << (connection.m_readBuffer);
	std::cout << "\" } ";
	std::cout << "{ " << bytesRecieved << " bytes of data recieved}";
	std::cout << std::endl;

}

void Server::OnConnect()
{

}

void Server::OnDisconnect()
{

}