#include <NonBlockingSocket/IncludeMe.h>
#include <iostream>

int main()
{
	if (CustomSocket::NetworkAPIInitializer::Initialize())
	{
		std::cout << "The winsock API was successfully initialized." << std::endl;

		/**
		CustomSocket::IPEndpoint test_ipe("www.vk.com", 8080);
		if (test_ipe.GetIPVersion() == CustomSocket::IPVersion::IPv4)
		{
			std::cout << "--------------------------------------------------" << std::endl;
			std::cout << "~~~~~~~~~~~~~~~IPEndpoint class log~~~~~~~~~~~~~~~" << std::endl;
			std::cout << "Hostname: " << test_ipe.GetHostname() << std::endl;
			std::cout << "IP: " << test_ipe.GetIPString() << std::endl;
			std::cout << "Port: " << test_ipe.GetPort() << std::endl;
			std::cout << "IP Bytes... " << std::endl;
			for (auto& digit : test_ipe.GetIPBytes())
			{
				std::cout << "\t" << static_cast<int>(digit) << std::endl;
			}
			std::cout << "~~~~~~~~~~~~~~~IPEndpoint class log~~~~~~~~~~~~~~~" << std::endl;
			std::cout << "--------------------------------------------------" << std::endl;
		}
		else
		{
			std::cerr << "Provided string is not an IPv4 address" << std::endl;
		}
		**/

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

	system("pause");
	return 0;
}