#include <NonBlockingSocket/IncludeMe.h>
#include <iostream>

int main()
{
	/** 
	* 
	* BLOCKING VERSION
	* 
	if (CustomSocket::NetworkAPIInitializer::Initialize())
	{
		std::cout << "The winsock API was successfully initialized." << std::endl;

		CustomSocket::Socket socket;
		if (socket.create() == CustomSocket::Result::Success)
		{
			std::cout << "The socket was successfully created." << std::endl;

			if (socket.Listen(CustomSocket::IPEndpoint("0.0.0.0", 4790))
				== CustomSocket::Result::Success)
			{
				std::cout << "Socket successfully listening on port 4790." << std::endl;

				CustomSocket::Socket newConnection;
				if (socket.Accept(newConnection) == CustomSocket::Result::Success)
				{
					char buffer[256];
					//int bytesRecieved = 0;

					CustomSocket::Result result = CustomSocket::Result::Fail;
					while (result != CustomSocket::Result::Success)
					{
						//result = newConnection.Recieve(buffer, 256, bytesRecieved);
						result = newConnection.RecieveAll(buffer, 256);

						if (result == CustomSocket::Result::Success)
						{
							std::cout << "[CLIENT]: " << buffer << std::endl;
							break;
						}
					}

					newConnection.close();
				}
				else
				{
					std::cerr << "Failed to accept connection on a socket." << std::endl;
				}
			}
			else
			{
				std::cerr << "Failed to listen a socket on port 4790." << std::endl;
			}

			if (socket.close() == CustomSocket::Result::Success)
			{
				std::cout << "The socket was successfully closed." << std::endl;
			}
			else
			{
				std::cout << "Failed to close a socket." << std::endl;
			}
		}
		else
		{
			std::cout << "Failed to create a socket." << std::endl;
		}
	}

	CustomSocket::NetworkAPIInitializer::Shutdown();
	**/

	if (CustomSocket::NetworkAPIInitializer::Initialize() == true)
	{
		CustomSocket::Socket server_socket;

		if (server_socket.Create() == CustomSocket::Result::Success)
		{
			std::cout << "[SERVICE INFO]: ";
			std::cout << "Socket was successfully created." << std::endl;

			if (server_socket.SetSocketOption(CustomSocket::Option::IO_NonBlocking, 
											  FALSE) == CustomSocket::Result::Success)
			{
				std::cout << "[SERVICE INFO]: ";
				std::cout << "Socket was successfully switched to Non Blocking state." << std::endl;

				CustomSocket::IPEndpoint IPToListen("127.0.0.1", 4790);

				if (server_socket.Listen(IPToListen) == CustomSocket::Result::Success)
				{
					CustomSocket::Socket newClientSocket;
					while (server_socket.Accept(newClientSocket) == CustomSocket::Result::Fail)
					{
						//std::cerr << "[SERVICE INFO]: ";
						//std::cerr << "Failed to accept new connection on the socket." << std::endl;
					}

					std::cout << "[SERVICE INFO]: ";
					std::cout << "Client was successfully connected." << std::endl;

					if (newClientSocket.Close() == CustomSocket::Result::Success)
					{
						std::cout << "[CLIENT]: " << "STATUS = Disconnected" << std::endl;
					}
					else
					{
						std::cerr << "[SERVICE INFO]: ";
						std::cerr << "Failed to disconnect the client." << std::endl;
					}
				}
				else
				{
					std::cerr << "[SERVICE INFO]: ";
					std::cerr << "Failed to set the socket to listen ip = "; 
					std::cerr << IPToListen.GetIPString();
					std::cerr << " on port " << IPToListen.GetPort() << "." << std::endl;
				}
			}
			else
			{
				std::cerr << "[SERVICE INFO]: ";
				std::cerr << "Failed to set the socket to Non Blocking state." << std::endl;
			}

			if (server_socket.Close() == CustomSocket::Result::Success)
			{
				std::cout << "[SERVICE INFO]: ";
				std::cout << "Socket was successfully closed." << std::endl;
			}
			else
			{
				std::cerr << "[SERVICE INFO]: ";
				std::cerr << "Failed to close the socket." << std::endl;
			}
		}
		else
		{
			std::cerr << "[SERVICE INFO]: ";
			std::cerr << "Failed to create the WinSock API socket." << std::endl;
		}
	}
	else
	{
		std::cerr << "[SERVICE INFO]: " << "Failed to strat up the WinSock API." << std::endl;
	}

	system("pause");
	return 0;
}