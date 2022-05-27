#include <NonBlockingSocket/IncludeMe.h>
#include <iostream>
#include <thread>

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

			if (socket.Connect(CustomSocket::IPEndpoint("127.0.0.1", 4790))
				== CustomSocket::Result::Success)
			{
				std::cout << "Succesfully connected to server." << std::endl;

				char buffer[256];
				strcpy_s(buffer, "Hello world from client)))\0");

				//int bytesSent = 0;
				CustomSocket::Result sendFlag = CustomSocket::Result::Fail;

				while (sendFlag == CustomSocket::Result::Fail)
				{
					std::cout << "[SERVICE INFO]: ";
					std::cout << "Attempting to send data to server..." << std::endl;
					//sendFlag = socket.Send(buffer, 256, bytesSent);
					sendFlag = socket.SendAll(buffer, 256);

					if (sendFlag != CustomSocket::Result::Success)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
					}
					else
					{
						std::cout << "[SERVICE INFO]: ";
						std::cout << "Data were successfully sent." << std::endl;
					}
				}
			}
			else
			{
				std::cerr << "Failed to connect to server." << std::endl;
			}

			socket.close();
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
		CustomSocket::Socket client_socket;

		if (client_socket.Create() == CustomSocket::Result::Success)
		{
			std::cout << "[SERVICE INFO]: ";
			std::cout << "Socket was successfully created." << std::endl;

			if (client_socket.SetSocketOption(CustomSocket::Option::IO_NonBlocking, 
											  FALSE) == CustomSocket::Result::Success)
			{
				std::cout << "[SERVICE INFO]: ";
				std::cout << "Socket was successfully switched to Blocking state." << std::endl;

				CustomSocket::IPEndpoint IPToListen("127.0.0.1", 4792);

				if (client_socket.Bind(IPToListen) == CustomSocket::Result::Success)
				{
					CustomSocket::IPEndpoint IPToConnect("127.0.0.1", 4790);

					if (client_socket.Connect(IPToConnect) == CustomSocket::Result::Success)
					{
						std::cout << "[SERVICE INFO]: ";
						std::cout << "Socket was successfully connected." << std::endl;
						std::cout << IPToConnect;

						const int bufSize = 256;
						char buffer[bufSize] = "Hello world from client)))\0";

						CustomSocket::Result sendFlag = CustomSocket::Result::Fail;

						while (sendFlag == CustomSocket::Result::Fail)
						{
							std::cout << "[SERVICE INFO]: ";
							std::cout << "Attempting to send data to server..." << std::endl;

							sendFlag = client_socket.SendAll(buffer, const_cast<int&>(bufSize));

							if (sendFlag != CustomSocket::Result::Success)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(500));
							}
							else
							{
								std::cout << "[SERVICE INFO]: ";
								std::cout << "Data were successfully sent." << std::endl;
							}
						}

						CustomSocket::Result result = CustomSocket::Result::Fail;
						while (result != CustomSocket::Result::Success)
						{
							//result = newConnection.Recieve(buffer, 256, bytesRecieved);
							int numberOfBytes = 256;
							result = client_socket.RecieveAll(buffer, numberOfBytes);
							if (result == CustomSocket::Result::Success)
							{
								std::cout << "[SERVER]: " << buffer << std::endl;
								break;
							}
						}
					}
					else
					{
						std::cerr << "[SERVICE INFO]: ";
						std::cerr << "Failed to connect to IP = " << IPToConnect.GetIPString();
						std::cerr << " port " << IPToConnect.GetPort() << "." << std::endl;
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
				std::cerr << "Failed to set the socket to Blocking state." << std::endl;
			}

			if (client_socket.Close() == CustomSocket::Result::Success)
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