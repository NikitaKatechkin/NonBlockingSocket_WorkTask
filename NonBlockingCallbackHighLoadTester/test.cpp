#include "pch.h"

#include <NonBlockingCallbackServer/CalbackServer.h>
#include <NonBlockingCallbackClient/CallbackClient.h>

/**
TEST(HighLoadTestCase, AsynchServerSend) 
{
	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//-------------------------------------------------------------------

	CustomSocket::Socket client;

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint clientIPConfig;
	EXPECT_EQ(client.GetSocketInfo(clientIPConfig), CustomSocket::Result::Success);

	//--------------------------------------------------------------------

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	server.WaitForConnection();

	EXPECT_EQ(server.GetConnectionList(), std::vector<CustomSocket::IPEndpoint> { clientIPConfig });

	//--------------------------------------------------------------------

	const int bufSize = 4096;
	const char* serverMessage = new char[bufSize] 
										{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};

	int bytesToRecieve = bufSize;
	char* clientBuffer = new char[bytesToRecieve] {};

	for (size_t index = 0; index < 1024; index++)
	{
		EXPECT_EQ(server.Send(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
					serverMessage, bufSize), CustomSocket::Result::Success);

		EXPECT_EQ(client.RecieveAll(clientBuffer, bytesToRecieve), CustomSocket::Result::Success);

		EXPECT_EQ(server.WaitClientOnSendEvent(clientIPConfig.GetIPString(), 
											   clientIPConfig.GetPort()), 
				  CustomSocket::Result::Success);

	}

	//--------------------------------------------------------------------

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Close(), CustomSocket::Result::Success);
}

TEST(HighLoadTestCase, AsynchServerRecieve)
{
	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//-------------------------------------------------------------------

	CustomSocket::Socket client;

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint clientIPConfig;
	EXPECT_EQ(client.GetSocketInfo(clientIPConfig), CustomSocket::Result::Success);

	//--------------------------------------------------------------------

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	server.WaitForConnection();

	EXPECT_EQ(server.GetConnectionList(), std::vector<CustomSocket::IPEndpoint> { clientIPConfig });

	//--------------------------------------------------------------------

	const int bufSize = 4096;
	char* serverBuffer = new char[bufSize] {};

	int bytesToSend = bufSize;
	const char* clientMessage = new char[bufSize]
	{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};


	for (size_t index = 0; index < 1024; index++)
	{
		EXPECT_EQ(client.SendAll(clientMessage, bytesToSend),
			CustomSocket::Result::Success);

		EXPECT_EQ(server.Recieve(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
								 serverBuffer, bufSize), CustomSocket::Result::Success);

		EXPECT_EQ(server.WaitClientOnRecieveEvent(clientIPConfig.GetIPString(),
												  clientIPConfig.GetPort()),
				  CustomSocket::Result::Success);
	}

	//--------------------------------------------------------------------

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Close(), CustomSocket::Result::Success);
}

TEST(HighLoadTestCase, AsynchClientSend)
{
	CallbackClient client;

	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//-------------------------------------------------------------------

	CustomSocket::Socket server;

	EXPECT_EQ(server.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Bind(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig;
	EXPECT_EQ(server.GetSocketInfo(serverIPConfig), CustomSocket::Result::Success);

	//--------------------------------------------------------------------

	CustomSocket::Socket newConnectionSocket;

	std::thread serverThread(&CustomSocket::Socket::Accept,
							std::ref(server),
							std::ref(newConnectionSocket),
							nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	serverThread.join();

	
	//--------------------------------------------------------------------

	const int bufSize = 4096;
	const char* clientMessage = new char[bufSize]
	{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};

	int bytesToRecv = bufSize;
	char* serverBuffer = new char[bufSize] {};

	for (size_t index = 0; index < 1024; index++)
	{
		EXPECT_EQ(client.Send(clientMessage, bufSize),
				  CustomSocket::Result::Success);

		EXPECT_EQ(newConnectionSocket.RecieveAll(serverBuffer, bytesToRecv),
				  CustomSocket::Result::Success);

		EXPECT_EQ(client.WaitOnSendEvent(), CustomSocket::Result::Success);
	}

	//--------------------------------------------------------------------
	
	EXPECT_EQ(newConnectionSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Close(), CustomSocket::Result::Success);
}

TEST(HighLoadTestCase, AsynchClientRecieve)
{
	CallbackClient client;

	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//-------------------------------------------------------------------

	CustomSocket::Socket server;

	EXPECT_EQ(server.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Bind(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig;
	EXPECT_EQ(server.GetSocketInfo(serverIPConfig), CustomSocket::Result::Success);

	//--------------------------------------------------------------------

	CustomSocket::Socket newConnectionSocket;

	std::thread serverThread(&CustomSocket::Socket::Accept,
		std::ref(server),
		std::ref(newConnectionSocket),
		nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	serverThread.join();


	//--------------------------------------------------------------------

	const int bufSize = 4096;

	int bytesToSend = bufSize;
	const char* serverMessage = new char[bufSize]
	{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};

	int bytesToRecv = bufSize;
	char* clientBuffer = new char[bytesToRecv] {};

	for (size_t index = 0; index < 1024; index++)
	{
		EXPECT_EQ(newConnectionSocket.SendAll(serverMessage, bytesToSend),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.Recieve(clientBuffer, bytesToRecv),
				  CustomSocket::Result::Success);

		EXPECT_EQ(client.WaitOnRecieveEvent(), CustomSocket::Result::Success);
	}

	//--------------------------------------------------------------------

	EXPECT_EQ(newConnectionSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Close(), CustomSocket::Result::Success);
}

TEST(HighLoadTestCase, AsynchServerSendBigOne)
{
	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//-------------------------------------------------------------------

	CustomSocket::Socket client;

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint clientIPConfig;
	EXPECT_EQ(client.GetSocketInfo(clientIPConfig), CustomSocket::Result::Success);

	//--------------------------------------------------------------------

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	server.WaitForConnection();

	EXPECT_EQ(server.GetConnectionList(), std::vector<CustomSocket::IPEndpoint> { clientIPConfig });

	//--------------------------------------------------------------------

	const int bufSize = 4096 * 1024;
	const char* serverMessage = new char[bufSize]
	{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};

	int bytesToRecieve = bufSize;
	char* clientBuffer = new char[bytesToRecieve] {};

	for (size_t index = 0; index < 1; index++)
	{
		EXPECT_EQ(server.Send(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
			serverMessage, bufSize), CustomSocket::Result::Success);

		EXPECT_EQ(client.RecieveAll(clientBuffer, bytesToRecieve), CustomSocket::Result::Success);

		EXPECT_EQ(server.WaitClientOnSendEvent(clientIPConfig.GetIPString(),
			clientIPConfig.GetPort()),
			CustomSocket::Result::Success);
	}

	//--------------------------------------------------------------------

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Close(), CustomSocket::Result::Success);

	delete[] serverMessage;
	delete[] clientBuffer;
}

TEST(HighLoadTestCase, AsynchServerRecieveBigOne)
{
	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//-------------------------------------------------------------------

	CustomSocket::Socket client;

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint clientIPConfig;
	EXPECT_EQ(client.GetSocketInfo(clientIPConfig), CustomSocket::Result::Success);

	//--------------------------------------------------------------------

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	server.WaitForConnection();

	EXPECT_EQ(server.GetConnectionList(), std::vector<CustomSocket::IPEndpoint> { clientIPConfig });

	//--------------------------------------------------------------------

	const int bufSize = 4096 * 1024;
	char* serverBuffer = new char[bufSize] {};

	int bytesToSend = bufSize;
	const char* clientMessage = new char[bufSize]
	{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};


	for (size_t index = 0; index < 1; index++)
	{
		EXPECT_EQ(client.SendAll(clientMessage, bytesToSend),
			CustomSocket::Result::Success);

		EXPECT_EQ(server.Recieve(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
			serverBuffer, bufSize), CustomSocket::Result::Success);

		EXPECT_EQ(server.WaitClientOnRecieveEvent(clientIPConfig.GetIPString(),
			clientIPConfig.GetPort()),
			CustomSocket::Result::Success);
	}

	//--------------------------------------------------------------------

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Close(), CustomSocket::Result::Success);

	delete[] serverBuffer;
	delete[] clientMessage;
}

TEST(HighLoadTestCase, AsynchClientSendBigOne)
{
	CallbackClient client;

	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//-------------------------------------------------------------------

	CustomSocket::Socket server;

	EXPECT_EQ(server.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Bind(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig;
	EXPECT_EQ(server.GetSocketInfo(serverIPConfig), CustomSocket::Result::Success);

	//--------------------------------------------------------------------

	CustomSocket::Socket newConnectionSocket;

	std::thread serverThread(&CustomSocket::Socket::Accept,
		std::ref(server),
		std::ref(newConnectionSocket),
		nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	serverThread.join();


	//--------------------------------------------------------------------

	const int bufSize = 4096 * 1024;
	const char* clientMessage = new char[bufSize]
	{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};

	int bytesToRecv = bufSize;
	char* serverBuffer = new char[bufSize] {};

	for (size_t index = 0; index < 1; index++)
	{
		EXPECT_EQ(client.Send(clientMessage, bufSize),
			CustomSocket::Result::Success);

		EXPECT_EQ(newConnectionSocket.RecieveAll(serverBuffer, bytesToRecv),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.WaitOnSendEvent(), CustomSocket::Result::Success);
	}

	//--------------------------------------------------------------------

	EXPECT_EQ(newConnectionSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Close(), CustomSocket::Result::Success);

	delete[] serverBuffer;
	delete[] clientMessage;
}

TEST(HighLoadTestCase, AsynchClientRecieveBigOne)
{
	CallbackClient client;

	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//-------------------------------------------------------------------

	CustomSocket::Socket server;

	EXPECT_EQ(server.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Bind(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig;
	EXPECT_EQ(server.GetSocketInfo(serverIPConfig), CustomSocket::Result::Success);

	//--------------------------------------------------------------------

	CustomSocket::Socket newConnectionSocket;

	std::thread serverThread(&CustomSocket::Socket::Accept,
		std::ref(server),
		std::ref(newConnectionSocket),
		nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	serverThread.join();


	//--------------------------------------------------------------------

	const int bufSize = 4096 * 1024;

	int bytesToSend = bufSize;
	const char* serverMessage = new char[bufSize]
	{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};

	int bytesToRecv = bufSize;
	char* clientBuffer = new char[bytesToRecv] {};

	for (size_t index = 0; index < 1; index++)
	{
		EXPECT_EQ(newConnectionSocket.SendAll(serverMessage, bytesToSend),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.Recieve(clientBuffer, bytesToRecv),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.WaitOnRecieveEvent(), CustomSocket::Result::Success);
	}

	//--------------------------------------------------------------------

	EXPECT_EQ(newConnectionSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Close(), CustomSocket::Result::Success);

	delete[] serverMessage;
	delete[] clientBuffer;
}
**/

TEST(HighLoadTestCase, FullyAsynchServerToClientSend)
{
	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//-------------------------------------------------------------------

	CallbackClient client;

	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//-------------------------------------------------------------------

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	//--------------------------------------------------------------------

	///**
	const int bufSize = 4096;
	const char* serverMessage = new char[bufSize]
	{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};

	int bytesToRecieve = bufSize;
	char* clientBuffer = new char[bytesToRecieve] {};

	for (size_t index = 0; index < 1024; index++)
	{
		EXPECT_EQ(server.Send(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
			serverMessage, bufSize), CustomSocket::Result::Success);

		EXPECT_EQ(server.WaitClientOnSendEvent(clientIPConfig.GetIPString(),
			clientIPConfig.GetPort()),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.Recieve(clientBuffer, bytesToRecieve), CustomSocket::Result::Success);

		EXPECT_EQ(client.WaitOnRecieveEvent(), CustomSocket::Result::Success);
	}
	//**/

	//--------------------------------------------------------------------
}

TEST(HighLoadTestCase, FullyAsynchServerFromClientRecieve)
{
	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//-------------------------------------------------------------------

	CallbackClient client;

	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//-------------------------------------------------------------------

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	//--------------------------------------------------------------------

	///**
	const int bufSize = 4096;
	char* serverBuffer = new char[bufSize] {};

	int bytesToSend = bufSize;
	const char* clientMessage = new char[bufSize]
	{"NIKITA, HONEY. TAKE THIS SOCKET STREAM MESSAGE)\0"};

	for (size_t index = 0; index < 1024; index++)
	{
		EXPECT_EQ(client.Send(clientMessage, bufSize),
			CustomSocket::Result::Success);
		EXPECT_EQ(client.WaitOnSendEvent(), CustomSocket::Result::Success);

		EXPECT_EQ(server.Recieve(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
			serverBuffer, bufSize), CustomSocket::Result::Success);
		EXPECT_EQ(server.WaitClientOnRecieveEvent(clientIPConfig.GetIPString(),
			clientIPConfig.GetPort()),
			CustomSocket::Result::Success);
	}
	//**/

	//--------------------------------------------------------------------
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}