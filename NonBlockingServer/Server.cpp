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


	if (m_listeningSocketService.m_socketInfo.first.Bind(IPconfig) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_listeningSocketService.m_socketInfo.first.GetSocketInfo(
		m_listeningSocketService.m_socketInfo.second) != CustomSocket::Result::Success)
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

	if (m_listeningSocketService.m_socketInfo.first.Bind(CustomSocket::IPEndpoint(ip, port))
		!= CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_listeningSocketService.m_socketInfo.first.GetSocketInfo(
		m_listeningSocketService.m_socketInfo.second) != CustomSocket::Result::Success)
	{
		throw std::exception();
	}
}

Server::~Server()
{
	if (m_isRunning == true)
	{
		Stop();
	}

	CustomSocket::NetworkAPIInitializer::Shutdown();
}

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

CustomSocket::Result Server::Recieve(const std::string& ip, const uint16_t port,
									 void* data, int numberOfBytes)
{
	std::lock_guard<std::mutex> operationLock(m_operationMutex);

	auto find_iter = m_connections.find(CustomSocket::IPEndpoint(ip, port));

	auto result = (find_iter != m_connections.end()) ? CustomSocket::Result::Success :
		CustomSocket::Result::Fail;


	if (result == CustomSocket::Result::Success)
	{
		//std::lock_guard<std::mutex> operationLock(m_operationMutex);
		result = (find_iter->second.m_recieveMessage.m_buffer == nullptr) ? CustomSocket::Result::Success :
															   CustomSocket::Result::Fail;

		if (result == CustomSocket::Result::Success)
		{
			find_iter->second.m_recieveMessage.m_buffer = static_cast<char*>(data);
			find_iter->second.m_recieveMessage.m_bytesToProcess = numberOfBytes;
			find_iter->second.m_recieveMessage.m_bufferTotalSize = numberOfBytes;
			find_iter->second.m_recieveMessage.m_onProcessFlag = true;
		}
	}

	return result;
}

CustomSocket::Result Server::Send(const std::string& ip, const uint16_t port,
								  const void* data, int numberOfBytes)
{
	std::lock_guard<std::mutex> operationLock(m_operationMutex);

	auto find_iter = m_connections.find(CustomSocket::IPEndpoint(ip, port));

	auto result = (find_iter != m_connections.end()) ? CustomSocket::Result::Success :
													   CustomSocket::Result::Fail;

	if (result == CustomSocket::Result::Success)
	{
		//std::lock_guard<std::mutex> operationLock(m_operationMutex);

		result = (find_iter->second.m_sendMessage.m_buffer == nullptr) ? CustomSocket::Result::Success :
																CustomSocket::Result::Fail;

		if (result == CustomSocket::Result::Success)
		{
			find_iter->second.m_sendMessage.m_buffer = static_cast<const char*>(data);
			find_iter->second.m_sendMessage.m_bytesToProcess = numberOfBytes;
			find_iter->second.m_sendMessage.m_bufferTotalSize = numberOfBytes;
			find_iter->second.m_sendMessage.m_onProcessFlag = true;
		}
	}

	return result;
}

void Server::WaitForConnection()
{
	WaitForSingleObject(m_getInfoEvent, INFINITE);
}
 
Server::CONNECTIONS_LIST Server::GetConnectionList()
{
	auto key_selector = [](auto& pair) { return pair.first; };
	CONNECTIONS_LIST result;

	for (auto& item : m_connections)
	{
		result.push_back(key_selector(item));
	}

	return result;
}

CustomSocket::Result Server::WaitClientOnSendEvent(const std::string& ip, 
												   const uint16_t port)
{
	auto result = (m_connections.find(CustomSocket::IPEndpoint(ip, port))
				   == m_connections.end()) ? CustomSocket::Result::Fail : 
											 CustomSocket::Result::Success;
	
	if (result == CustomSocket::Result::Success)
	{
		auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;
		WaitForSingleObject(connection.m_onSendEvent, INFINITE);
		result = (ResetEvent(connection.m_onSendEvent) == TRUE) ? 
															CustomSocket::Result::Success : 
															CustomSocket::Result::Fail;
	}

	return result;
}

CustomSocket::Result Server::WaitClientOnRecieveEvent(const std::string& ip, 
												   const uint16_t port)
{
	auto result = (m_connections.find(CustomSocket::IPEndpoint(ip, port))
		== m_connections.end()) ? CustomSocket::Result::Fail :
		CustomSocket::Result::Success;

	if (result == CustomSocket::Result::Success)
	{
		auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;
		WaitForSingleObject(connection.m_onRecieveEvent, INFINITE);
		result = (ResetEvent(connection.m_onRecieveEvent) == TRUE) ?
			CustomSocket::Result::Success :
			CustomSocket::Result::Fail;
	}

	return result;
}

CustomSocket::IPEndpoint Server::GetServerIPConfig()
{
	return m_listeningSocketService.m_socketInfo.second;
}

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
				{nullptr, 0, 0, false},
				CreateEvent(nullptr, TRUE, FALSE, L"OnSendEvent"),
				{nullptr, 0, 0, false},
				CreateEvent(nullptr, TRUE, FALSE, L"OnRecieveEvent")
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

	if (connection.m_recieveMessage.m_onProcessFlag == true)
	{
		int bytesRecieved = 0;
		auto offset = (connection.m_recieveMessage.m_bufferTotalSize -
					   connection.m_recieveMessage.m_bytesToProcess);

		if (connection.m_socketInfo.first.Recieve(
			connection.m_recieveMessage.m_buffer + offset,
			connection.m_recieveMessage.m_bytesToProcess,
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
				connection.m_recieveMessage.m_bytesToProcess -= bytesRecieved;

				if (connection.m_recieveMessage.m_bytesToProcess <= 0)
				{
					int tmp_buffer_size = connection.m_recieveMessage.m_bufferTotalSize;
					char* tmp_buffer = new char[tmp_buffer_size];
					memcpy_s(tmp_buffer,
							 tmp_buffer_size,
							 connection.m_recieveMessage.m_buffer,
							 connection.m_recieveMessage.m_bufferTotalSize);

					connection.m_recieveMessage.m_buffer = nullptr;
					connection.m_recieveMessage.m_bytesToProcess = 0;
					connection.m_recieveMessage.m_bufferTotalSize = 0;
					connection.m_recieveMessage.m_onProcessFlag = false;

					OnRecieve(ip, 
							  port, 
							  tmp_buffer,
							  tmp_buffer_size);

					delete[] tmp_buffer;
				}
			}
		}
	}
}

void Server::SendProcessing(const std::string& ip, const uint16_t port)
{
	auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;

	if (connection.m_sendMessage.m_onProcessFlag == true)
	{
		int bytesSent = 0;
		auto offset = (connection.m_sendMessage.m_bufferTotalSize -
					   connection.m_sendMessage.m_bytesToProcess);

		if (connection.m_socketInfo.first.Send(
			connection.m_sendMessage.m_buffer + offset,
			connection.m_sendMessage.m_bytesToProcess,
			bytesSent)
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
				connection.m_sendMessage.m_bytesToProcess -= bytesSent;

				if (connection.m_sendMessage.m_bytesToProcess <= 0)
				{
					int tmp_buffer_size = connection.m_sendMessage.m_bufferTotalSize;
					char* tmp_buffer = new char[tmp_buffer_size];
					memcpy_s(tmp_buffer,
							 tmp_buffer_size,
							 connection.m_sendMessage.m_buffer,
							 connection.m_sendMessage.m_bufferTotalSize);

					connection.m_sendMessage.m_buffer = nullptr;
					connection.m_sendMessage.m_bytesToProcess = 0;
					connection.m_sendMessage.m_bufferTotalSize = 0;
					connection.m_sendMessage.m_onProcessFlag = false;

					OnSend(ip, 
						   port, 
						   tmp_buffer,
						   tmp_buffer_size);

					delete[] tmp_buffer;
					//SetEvent(connection.m_onSendEvent);
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

	auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;
	/**
	if (SetEvent(connection.m_onSendEvent))
	{
		std::cout << "ejrgnaeingrijnaeirgnhiquhgpiuqhiughq3p4or" << std::endl;
	}
	else
	{
		std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << std::endl;
	}
	**/
	SetEvent(connection.m_onSendEvent);
}

void Server::OnRecieve(const std::string& ip, const uint16_t port, char* data, int& bytesRecieved)
{
	//auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;

	std::lock_guard<std::mutex> coutLock(m_printLogMutex);

	std::cout << "[CLIENT]: ";
	std::cout << "{ " << bytesRecieved << " bytes recieved from port ";
	std::cout << static_cast<int>(port) << "} ";
	std::cout << "{ MESSAGE = \"" << data << "\" } " << std::endl;

	auto& connection = m_connections.find(CustomSocket::IPEndpoint(ip, port))->second;
	SetEvent(connection.m_onRecieveEvent);
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