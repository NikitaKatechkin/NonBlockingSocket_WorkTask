#include <NonBlockingSocket/IncludeMe.h>
#include <NonBlockingServer/Server.h>
#include <iostream>
#include <thread>

int main()
{
	/**
	if (CustomSocket::NetworkAPIInitializer::Initialize() == true)
	{
		CustomSocket::Socket server_socket;

		if (server_socket.Create() == CustomSocket::Result::Success)
		{
			std::cout << "[SERVICE INFO]: ";
			std::cout << "Socket was successfully created." << std::endl;

			if (server_socket.SetSocketOption(CustomSocket::Option::IO_NonBlocking, 
											  TRUE) == CustomSocket::Result::Success)
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

						std::this_thread::sleep_for(std::chrono::seconds(2));
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
	**/

	Server server(CustomSocket::IPEndpoint("127.0.0.1", 4790));
	
	server.run();

	server.waitForConnection();
	
	char readBuffer[256] = {};
	server.read(readBuffer, 4791);

	const char writeBuffer[256] = { "Hello world from server)))\0" };
	server.write(writeBuffer, 4791);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	server.stop();

	system("pause");
	return 0;
}