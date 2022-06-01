#include "pch.h"

#include <NonBlockingCallbackServer/CalbackServer.h>

TEST(NonBlockingCallbackServer, ConstructorTest)
{
	EXPECT_NO_THROW(CallbackServer(CustomSocket::IPEndpoint("127.0.0.1", 4790)));
	EXPECT_ANY_THROW(CallbackServer(CustomSocket::IPEndpoint("327.0.0.1", 4790)));
}

TEST(NonBlockingCallbackServer, AdditionalConstructorTest)
{
	EXPECT_NO_THROW(CallbackServer("127.0.0.1", 4790));
	EXPECT_ANY_THROW(CallbackServer("327.0.0.1", 4790));
}

TEST(NonBlockingCallbackServer, PositiveRunTest)
{
	CallbackServer server("127.0.0.1", 4790);
	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	server.Stop();
}

TEST(NonBlockingCallbackServer, NegativeRunTest)
{
	CallbackServer server("127.0.0.1", 4790);
	EXPECT_ANY_THROW(CallbackServer("127.0.0.1", 4790));

	server.Run();
	//EXPECT_EQ(failing_server.Run(), CustomSocket::Result::Fail);

	server.Stop();
}

TEST(NonBlockingCallbackServer, PositiveStopTest)
{
	CallbackServer server("127.0.0.1", 4790);
	server.Run();

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
}

TEST(NonBlockingCallbackServer, NegativeStopTest)
{
	CallbackServer server("127.0.0.1", 4790);
	server.Run();

	CustomSocket::NetworkAPIInitializer::Shutdown();

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
}

TEST(NonBlockingCallbackServer, PositiveEmptyConnectionListTest)
{
	CallbackServer server("127.0.0.1", 4790);
	server.Run();

	std::vector<CustomSocket::IPEndpoint> empty_list;
	EXPECT_EQ(server.GetConnectionList(), empty_list);

	server.Stop();
}

TEST(NonBlockingCallbackServer, PositiveNonEmptyConnectionListTest)
{
	CallbackServer server("127.0.0.1", 4790);
	server.Run();

	CustomSocket::Socket client;

	//std::string clientIP = "127.0.0.1";
	//clientIP.resize(16);
	//const uint16_t port = 55687;

	const CustomSocket::IPEndpoint clientConfig("127.0.0.1", 55687);

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(&clientConfig), CustomSocket::Result::Success);
	
	client.Connect(CustomSocket::IPEndpoint("127.0.0.1", 4790));

	std::vector<CustomSocket::IPEndpoint> list;
	list.push_back(clientConfig);

	server.WaitForConnection();
	EXPECT_EQ(server.GetConnectionList(), list);

	server.Stop();

	list.clear();
	EXPECT_EQ(server.GetConnectionList(), list);
}

TEST(NonBlockingCallbackServer, PositiveRecieveTest)
{
	const CustomSocket::IPEndpoint clientConfig("127.0.0.1", 55699);
	const CustomSocket::IPEndpoint serverConfig("127.0.0.1", 4790);

	CallbackServer server(serverConfig);
	server.Run();

	CustomSocket::Socket client;

	client.Create();
	client.Bind(&clientConfig);

	client.Connect(serverConfig);
	server.WaitForConnection();

	while (server.GetConnectionList().size() != 1) {}

	EXPECT_EQ(server.Recieve("127.0.0.1", 55699, nullptr, 0), CustomSocket::Result::Success);

	server.Stop();
}

TEST(NonBlockingCallbackServer, NegativeRecieveTest)
{
	const CustomSocket::IPEndpoint serverConfig("127.0.0.1", 4790);

	CallbackServer server(serverConfig);
	server.Run();

	EXPECT_EQ(server.Recieve("127.0.0.1", 55699, nullptr, 0), CustomSocket::Result::Fail);

	server.Stop();
}

TEST(NonBlockingCallbackServer, PositiveSendTest)
{
	const CustomSocket::IPEndpoint clientConfig("127.0.0.1", 55699);
	const CustomSocket::IPEndpoint serverConfig("127.0.0.1", 4790);

	CallbackServer server(serverConfig);
	server.Run();

	CustomSocket::Socket client;

	client.Create();
	client.Bind(&clientConfig);

	client.Connect(serverConfig);
	server.WaitForConnection();

	//while (server.GetConnectionList().size() != 1) {}

	EXPECT_EQ(server.Send("127.0.0.1", 55699, nullptr, 0), CustomSocket::Result::Success);

	server.Stop();
}

TEST(NonBlockingCallbackServer, NegativeSendTest)
{
	const CustomSocket::IPEndpoint serverConfig("127.0.0.1", 4790);

	CallbackServer server(serverConfig);
	server.Run();

	EXPECT_EQ(server.Send("127.0.0.1", 55699, nullptr, 0), CustomSocket::Result::Fail);

	server.Stop();
}

//---------------------------------------------------------

TEST(NonBlockingSocket, PositiveBindAndListenCompitabilityTest)
{
	EXPECT_TRUE(CustomSocket::NetworkAPIInitializer::Initialize());

	CustomSocket::Socket socket;
	
	EXPECT_EQ(socket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Bind(&CustomSocket::IPEndpoint("127.0.0.1", 4790)),
			  CustomSocket::Result::Success);
	EXPECT_EQ(socket.Listen(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Close(), CustomSocket::Result::Success);

	EXPECT_EQ(socket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint config;
	EXPECT_EQ(socket.GetSocketInfo(&config), 
			  CustomSocket::Result::Success);

	std::cout << config << std::endl;

	EXPECT_EQ(socket.Listen(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Close(), CustomSocket::Result::Success);

	EXPECT_EQ(socket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Bind(&CustomSocket::IPEndpoint("127.0.0.1", 4790)),
		CustomSocket::Result::Success);
	EXPECT_EQ(socket.Listen(&CustomSocket::IPEndpoint("127.0.0.1", 4790)), 
							CustomSocket::Result::Success);
	EXPECT_EQ(socket.Close(), CustomSocket::Result::Success);

	EXPECT_EQ(socket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Bind(), CustomSocket::Result::Success);

	CustomSocket::NetworkAPIInitializer::Shutdown();
}

int main(int argc, char* argv[])
{
	///**
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
	//**/

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
	return 0;
	**/
}