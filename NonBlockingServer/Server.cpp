#include "Server.h"

#include <iostream>

static const size_t LISTENING_FD_INDEX = 0;

Server::Server(const CustomSocket::IPEndpoint& IPconfig):
	m_IPConfig(IPconfig), m_isRunning(false)
{
	if (m_listeningSocket.Create() != CustomSocket::Result::Success)
	{
		if (CustomSocket::NetworkAPIInitializer::Initialize() != true)
		{
			throw std::exception();
		}
		else
		{
			if (m_listeningSocket.Create() != CustomSocket::Result::Success)
			{
				throw std::exception();
			}
		}
	}

	if (m_listeningSocket.SetSocketOption(CustomSocket::Option::IO_NonBlocking, TRUE) !=
		CustomSocket::Result::Success)
	{
		throw std::exception();
	}

	//if (m_listeningSocket.Listen(m_IPConfig) != CustomSocket::Result::Success)
	//{
	//	throw std::exception();
	//}

	WSAPOLLFD listeningSocketFD = { m_listeningSocket.GetHandle(),
									(POLLRDNORM),
									0 };
	//listeningSocketFD.fd = m_listeningSocket.GetHandle();
	//listeningSocketFD.events = POLLRDNORM; // I want
	//listeningSocketFD.revents = 0; // I get

	m_socketFDs.push_back(listeningSocketFD);
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

		m_connection.erase(portToDisconnect);
	}

	return result;
}

CustomSocket::Result Server::connect()
{
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
				if (connect() == CustomSocket::Result::Success)
				{
					//if (disconnect(m_connection.back().second.GetPort())
					//	== CustomSocket::Result::Fail)
					//{
					//	break;
					//}
				}
			}

			inspectAllConnections();
		}
	}
}

void Server::inspectAllConnections()
{
	for (size_t index = 0; index < m_connection.size(); index++)
	{
		if (m_socketFDs[index + 1].revents & POLLRDNORM)
		{
			//READ
			//std::cout << "Reading..." << std::endl;

			char buffer[256] = {};
			int bytesRecieved = 0;

			if (m_connection[index].first.Recieve(buffer, 256, bytesRecieved)
				== CustomSocket::Result::Success)
			{
				std::cout << "[CLIENT]: " << buffer << std::endl;
				m_connection[index].first.Close();
				//disconnect(m_connection[index].second.GetPort());
			}
		}
		if (m_socketFDs[index + 1].revents & POLLWRNORM)
		{
			//WRITE
			//std::cout << "Writing..." << std::endl;
		}
	}
}
