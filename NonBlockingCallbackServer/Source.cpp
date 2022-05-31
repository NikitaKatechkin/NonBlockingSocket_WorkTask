#include <NonBlockingSocket/IncludeMe.h>
#include <NonBlockingCallbackServer/CalbackServer.h>
#include <iostream>
#include <thread>

int main()
{
	/**
	const int bufSize = 256;

	CallbackServer server("127.0.0.1", 4790);

	server.Run();

	server.WaitForConnection();

	CustomSocket::IPEndpoint client_connection = server.GetConnectionList()[0];

	char readBuffer[bufSize] = {};
	server.Recieve(client_connection.GetIPString(), static_cast<int>(client_connection.GetPort()), 
				   readBuffer, bufSize);

	const char writeBuffer[bufSize] = { "Hello world from server)))\0" };
	server.Send(client_connection.GetIPString(), static_cast<int>(client_connection.GetPort()), 
				writeBuffer, bufSize);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	server.Stop();

	system("pause");
	**/

	return 0;
}