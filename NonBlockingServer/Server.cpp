#include "Server.h"

#include <iostream>

static const size_t LISTENING_FD_INDEX = 0;

Server::Server(const CustomSocket::IPEndpoint& IPconfig):
	m_IPConfig(IPconfig), m_isRunning(false)
{
	if (CustomSocket::NetworkAPIInitializer::Initialize() != true)
	{
		throw std::exception();
	}

	if (m_listeningSocket.Create() != CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	if (m_listeningSocket.SetSocketOption(CustomSocket::Option::IO_NonBlocking, TRUE) !=
		CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	m_getInfoEvent = CreateEvent(nullptr, TRUE, FALSE, L"ServiceEvent");
	if (m_getInfoEvent == NULL)
	{
		throw std::exception();
	}

	WSAPOLLFD listeningSocketFD = { m_listeningSocket.GetHandle(),
									(POLLRDNORM),
									0 };

	m_socketFDs.push_back(listeningSocketFD);

	m_onWriteFlag.push_back(false);
	m_writeBuffer.push_back(nullptr);

	m_onReadFlag.push_back(false);
	m_readBuffer.push_back(nullptr);
}

CustomSocket::Result Server::run()
{
	CustomSocket::Result result = m_listeningSocket.Listen(m_IPConfig);

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

CustomSocket::Result Server::stop()
{
	m_isRunning = false;
	m_listenThread.join();

	CustomSocket::Result result = m_listeningSocket.Close();
	if (result == CustomSocket::Result::Success)
	{
		std::cout << "[SERVICE INFO]: " << "Server successfully stopped." << std::endl;
	}

	return result;
}

CustomSocket::Result Server::read(void* data, const uint16_t port)
{
	auto find_iter = std::find_if(
		m_connection.begin(),
		m_connection.end(),
		[&port](CONNECTION_INFO info) { return info.second.GetPort() == port; });

	CustomSocket::Result result = (find_iter != m_connection.end()) ? CustomSocket::Result::Success :
		CustomSocket::Result::Fail;


	if (result == CustomSocket::Result::Success)
	{
		size_t index = (find_iter - m_connection.begin());

		m_readBuffer[index + 1] = static_cast<char*>(data);
		m_onReadFlag[index + 1] = true;
	}

	return result;
}

CustomSocket::Result Server::write(const void* data, const uint16_t port)
{
	auto find_iter = std::find_if(
		m_connection.begin(),
		m_connection.end(),
		[&port](CONNECTION_INFO info) { return info.second.GetPort() == port; });

	CustomSocket::Result result = (find_iter != m_connection.end()) ? CustomSocket::Result::Success :
		CustomSocket::Result::Fail;


	if (result == CustomSocket::Result::Success)
	{
		size_t index = (find_iter - m_connection.begin());

		m_writeBuffer[index + 1] = static_cast<const char*>(data);
		m_onWriteFlag[index + 1] = true;
	}

	return result;
}

void Server::waitForConnection()
{
	WaitForSingleObject(m_getInfoEvent, INFINITE);
}

CustomSocket::Result Server::disconnect(const uint16_t port)
{
	auto portToDisconnect = std::find_if(
		m_connection.begin(),
		m_connection.end(),
		[&port](CONNECTION_INFO info) { return info.second.GetPort() == port; });

	CustomSocket::Result result = (portToDisconnect != m_connection.end()) ? CustomSocket::Result::Success :
		CustomSocket::Result::Fail;

	if (result == CustomSocket::Result::Success)
	{
		{
			//std::lock_guard<std::mutex> print_lock(m_printLogMutex);

			std::cout << "[CLIENT]: " << "{IP = " << portToDisconnect->second.GetIPString();
			std::cout << "} {PORT = " << portToDisconnect->second.GetPort() << "} ";
			std::cout << "{STATUS = DISCONNECTED}" << std::endl;
		}

		//portToDisconnect->first.close();

		size_t portToDisconnectIndex = portToDisconnect - m_connection.begin();
		m_socketFDs.erase(m_socketFDs.begin() + portToDisconnectIndex + 1);
		m_onWriteFlag.erase(m_onWriteFlag.begin() + portToDisconnectIndex + 1);
		m_writeBuffer.erase(m_writeBuffer.begin() + portToDisconnectIndex + 1);
		m_onReadFlag.erase(m_onReadFlag.begin() + portToDisconnectIndex + 1);
		m_readBuffer.erase(m_readBuffer.begin() + portToDisconnectIndex + 1);

		m_connection.erase(portToDisconnect);
	}

	return result;
}

CustomSocket::Result Server::connect()
{
	//TODO: FIX ~SOCKET::SOCKET()
	//AND MAKE SOLUTION WITHOUT FANCY DISTRUCT
	//OR
	//MOVE Semantics

	CustomSocket::Socket newConnection;
	CustomSocket::IPEndpoint newConnectionEndpoint;

	CustomSocket::Result result = m_listeningSocket.Accept(newConnection, &newConnectionEndpoint);

	if (result == CustomSocket::Result::Success)
	{
		std::cout << "[CLIENT]: " << "{IP = " << newConnectionEndpoint.GetIPString();
		std::cout << "} {PORT = " << newConnectionEndpoint.GetPort() << "} ";
		std::cout << "{STATUS = CONNECTED}" << std::endl;

		//ACCEPTING-REGISTRATING ROUTINE STARTS
		
		result = newConnection.SetSocketOption(CustomSocket::Option::IO_NonBlocking, TRUE);

		if (result == CustomSocket::Result::Success)
		{
			m_connection.push_back(CONNECTION_INFO(newConnection, newConnectionEndpoint));

			WSAPOLLFD listeningSocketFD = { newConnection.GetHandle(),
											(POLLRDNORM | POLLWRNORM),
											0 };

			m_socketFDs.push_back(listeningSocketFD);
			m_onWriteFlag.push_back(false);
			m_writeBuffer.push_back(nullptr);

			m_onReadFlag.push_back(false);
			m_readBuffer.push_back(nullptr);

			SetEvent(m_getInfoEvent);

			std::cout << "[SERVICE INFO]: " << "Pool size = ";
			std::cout << m_connection.size() << "." << std::endl;
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
	
void Server::processLoop()
{
	int numOfAccuredEvents = 0;

	while (m_isRunning == true)
	{
		numOfAccuredEvents = WSAPoll(&m_socketFDs[0], static_cast<ULONG>(m_socketFDs.size()), 1);

		if (numOfAccuredEvents > 0)
		{
			if (m_socketFDs[LISTENING_FD_INDEX].revents & POLLRDNORM)
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

		if (m_connection.empty() == true)
		{
			ResetEvent(m_getInfoEvent);
		}
	}
}

void Server::inspectAllConnections()
{
	for (size_t index = 0; index < m_connection.size(); index++)
	{
		if (m_socketFDs[index + 1].revents & POLLRDNORM)
		{
			//TODO:
			//READ -> CREATE OnRecieve() function
			//+ rename flags
			if (m_onReadFlag[index + 1] == true)
			{
				char buffer[256] = {};
				int bytesRecieved = 0;

				if (m_connection[index].first.Recieve(m_readBuffer[index + 1], 256, bytesRecieved)
					== CustomSocket::Result::Success)
				{
					if (bytesRecieved == 0)
					{
						//CLIENT WAS DISCONNECTED

						disconnect(m_connection[index].second.GetPort());
					}
					else
					{
						//std::cout << "[CLIENT]: " << buffer << std::endl;
						std::cout << "[CLIENT]: " << (m_readBuffer[index + 1]) << std::endl;
						m_onReadFlag[index + 1] = false;
					}
					

					//TODO: remove
					m_onWriteFlag[index + 1] = true;
				}
			}
		}

		if (m_socketFDs[index + 1].revents & POLLWRNORM)
		{
			//TODO:
			//WRITE -> CREATE OnSend() function
			//+ rename flags
			if (m_onWriteFlag[index + 1] == true)
			{
				const char buffer[256] = { "Hello, world)))\0" };
				int bytesSent = 0;

				if (m_connection[index].first.Send(m_writeBuffer[index + 1], 256, bytesSent)
					== CustomSocket::Result::Success)
				{
					if (bytesSent == 0)
					{
						//CLIENT WAS DISCONNECTED

						disconnect(m_connection[index].second.GetPort());
					}
					else
					{
						std::cout << "[SERVICE INFO]: " << "{ " << bytesSent;
						std::cout << " bytes sent }" << std::endl;
						m_onWriteFlag[index + 1] = false;

					}
				}
			}
		}
	}
}
