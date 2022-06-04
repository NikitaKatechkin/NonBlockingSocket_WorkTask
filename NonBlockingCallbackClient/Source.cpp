#include <NonBlockingCallbackClient/CallbackClient.h>

int main()
{
	const int bufSize = 256;

	CallbackClient client("127.0.0.1", 4791);
	client.Connect(CustomSocket::IPEndpoint("127.0.0.1", 4790));

	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	const char writeBuffer[bufSize] = { "Hello world from client)))\0" };
	client.Send(writeBuffer, bufSize);

	char readBuffer[bufSize] = {};
	client.Recieve(readBuffer, bufSize);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	client.Disconnect();

	system("pause");
	return 0;
}