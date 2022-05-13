#include <NonBlockingSocket/IncludeMe.h>
#include <iostream>
#include <thread>

int main()
{
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

	system("pause");
	return 0;
}