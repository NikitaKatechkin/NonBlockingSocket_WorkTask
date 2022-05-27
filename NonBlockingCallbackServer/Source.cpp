#include <NonBlockingSocket/IncludeMe.h>
#include <NonBlockingCallbackServer/CalbackServer.h>
#include <iostream>
#include <thread>

int main()
{

	const int bufSize = 256;

	//Server server(CustomSocket::IPEndpoint("127.0.0.1", 4790));
	CallbackServer server("127.0.0.1", 4790);

	server.run();

	server.waitForConnection();

	char readBuffer[bufSize] = {};
	server.recieve("127.0.0.1", 4791, readBuffer, bufSize);

	const char writeBuffer[bufSize] = { "Hello world from server)))\0" };
	server.send("127.0.0.1", 4791, writeBuffer, bufSize);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	server.stop();

	system("pause");
	return 0;
}