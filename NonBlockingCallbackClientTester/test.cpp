#include "pch.h"

#include <NonBlockingCallbackClient/CallbackClient.h>

const std::string clientIP = "127.0.0.1";
const uint32_t clientPort = 0;
const CustomSocket::IPEndpoint clientConfig(clientIP, clientPort);

const std::string invalidServerIP = "327.0.0.1";

const std::string serverIP = "127.0.0.1";
const uint32_t serverPort = 0;
const CustomSocket::IPEndpoint serverConfig(serverIP, serverPort);

TEST(NonBlockingCallbackClient, ConstructorTest)
{
	EXPECT_NO_THROW(CallbackClient test(clientConfig));
	EXPECT_ANY_THROW(CallbackClient test(CustomSocket::IPEndpoint(invalidServerIP, clientPort)));
}

TEST(NonBlockingCallbackClient, AdditionalConstructorTest)
{
	EXPECT_NO_THROW(CallbackClient test(clientIP, clientPort));
	EXPECT_ANY_THROW(CallbackClient test(invalidServerIP, clientPort));
}

TEST(NonBlockingCallbackClient, PositiveConnectDisconnectTest)
{
	CallbackClient client(clientConfig);

	CustomSocket::Socket serverSocket;
	EXPECT_EQ(serverSocket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(serverSocket.Bind(serverConfig), CustomSocket::Result::Success);
	EXPECT_EQ(serverSocket.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverEndpoint;
	EXPECT_EQ(serverSocket.GetSocketInfo(serverEndpoint), CustomSocket::Result::Success);

	CustomSocket::Socket newConnectionSocket;
	std::thread serverListenThread(&CustomSocket::Socket::Accept,
								   std::ref(serverSocket),
								   std::ref(newConnectionSocket),
								   nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	EXPECT_EQ(client.Connect(serverEndpoint), CustomSocket::Result::Success);

	serverListenThread.join();
	EXPECT_EQ(serverSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Disconnect(), CustomSocket::Result::Success);
}

TEST(NonBlockingCallbackClient, PositiveSendTest)
{
	CallbackClient client(clientConfig);

	CustomSocket::Socket serverSocket;
	EXPECT_EQ(serverSocket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(serverSocket.Bind(serverConfig), CustomSocket::Result::Success);
	EXPECT_EQ(serverSocket.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverEndpoint;
	EXPECT_EQ(serverSocket.GetSocketInfo(serverEndpoint), CustomSocket::Result::Success);

	CustomSocket::Socket newConnectionSocket;
	std::thread serverThread(&CustomSocket::Socket::Accept,
								   std::ref(serverSocket),
								   std::ref(newConnectionSocket),
								   nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	EXPECT_EQ(client.Connect(serverEndpoint), CustomSocket::Result::Success);

	serverThread.join();

	const int bufSize = 256;
	const char serverMessage[bufSize] = "Hello world from server)))\0";

	int bytesSent = 0;
	serverThread = std::thread(&CustomSocket::Socket::Send,
								std::ref(newConnectionSocket),
								std::ref(serverMessage),
								bufSize,
								std::ref(bytesSent));

	char recieveBuffer[bufSize] = {};
	int bytesRecieved = 0;

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	
	EXPECT_EQ(client.Recieve(recieveBuffer, bytesRecieved), CustomSocket::Result::Success);

	EXPECT_EQ(bytesRecieved, 0);
	EXPECT_EQ(bytesSent, bufSize);

	serverThread.join();

	EXPECT_EQ(serverSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Disconnect(), CustomSocket::Result::Success);
}

int main(int argc, char* argv[])
{
	///**
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
	//**/

	/**
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
	**/
}