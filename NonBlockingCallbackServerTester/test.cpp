#include "pch.h"

#include <NonBlockingCallbackServer/CalbackServer.h>

const std::string serverIP = "127.0.0.1";
const uint32_t serverPort = 0;
const CustomSocket::IPEndpoint serverConfig(serverIP, serverPort);

const std::string invalidServerIP = "327.0.0.1";

const std::string clientIP = "127.0.0.1";
const uint32_t clientPort = 0;
const CustomSocket::IPEndpoint clientConfig(clientIP, clientPort);


TEST(NonBlockingCallbackServer, ConstructorTest)
{
	EXPECT_NO_THROW(CallbackServer test(serverConfig));
	EXPECT_ANY_THROW(CallbackServer test(
		CustomSocket::IPEndpoint(invalidServerIP, serverPort)));
}

TEST(NonBlockingCallbackServer, AdditionalConstructorTest)
{
	EXPECT_NO_THROW(CallbackServer test(serverIP, serverPort));
	EXPECT_ANY_THROW(CallbackServer test(invalidServerIP, serverPort));
}

TEST(NonBlockingCallbackServer, PositiveRunTest)
{
	CallbackServer server(serverConfig);

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
}

TEST(NonBlockingCallbackServer, PositiveStopTest)
{
	CallbackServer server(serverConfig);

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
}

TEST(NonBlockingCallbackServer, PositiveEmptyConnectionListTest)
{
	CallbackServer server(serverConfig);
	server.Run();

	std::vector<CustomSocket::IPEndpoint> empty_list;
	EXPECT_EQ(server.GetConnectionList(), empty_list);

	server.Stop();
}

TEST(NonBlockingCallbackServer, PositiveNonEmptyConnectionListTest)
{
	CallbackServer server(serverConfig);
	server.Run();

	CustomSocket::Socket client;
	//const CustomSocket::IPEndpoint clientConfig("127.0.0.1", 55687);

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(clientConfig), CustomSocket::Result::Success);
	
	client.Connect(server.GetServerIPConfig());

	CustomSocket::IPEndpoint clientIPInfo;
	EXPECT_EQ(client.GetSocketInfo(clientIPInfo), 
			  CustomSocket::Result::Success);

	std::vector<CustomSocket::IPEndpoint> list;
	list.push_back(clientIPInfo);

	server.WaitForConnection();
	EXPECT_EQ(server.GetConnectionList(), list);

	server.Stop();

	list.clear();
	EXPECT_EQ(server.GetConnectionList(), list);
}

TEST(NonBlockingCallbackServer, PositiveRecieveTest)
{
	//const CustomSocket::IPEndpoint clientConfig("127.0.0.1", 55699);
	//const CustomSocket::IPEndpoint serverConfig("127.0.0.1", 0);

	CallbackServer server(serverConfig);
	server.Run();

	CustomSocket::Socket client;

	client.Create();
	client.Bind(clientConfig);

	client.Connect(server.GetServerIPConfig());
	server.WaitForConnection();

	CustomSocket::IPEndpoint clientIPInfo;
	EXPECT_EQ(client.GetSocketInfo(clientIPInfo),
		CustomSocket::Result::Success);

	EXPECT_EQ(server.Recieve(clientIPInfo.GetIPString(),
							 clientIPInfo.GetPort(),
							 nullptr, 
							 0), CustomSocket::Result::Success);

	server.Stop();
}

TEST(NonBlockingCallbackServer, NegativeRecieveTest)
{
	//const CustomSocket::IPEndpoint serverConfig("127.0.0.1", 0);

	CallbackServer server(serverConfig);
	server.Run();

	EXPECT_EQ(server.Recieve(clientIP, clientPort, nullptr, 0), 
			  CustomSocket::Result::Fail);

	server.Stop();
}

TEST(NonBlockingCallbackServer, PositiveSendTest)
{
	//const CustomSocket::IPEndpoint clientConfig("127.0.0.1", 55699);
	//const CustomSocket::IPEndpoint serverConfig("127.0.0.1", 0);

	CallbackServer server(serverConfig);
	server.Run();

	CustomSocket::Socket client;

	client.Create();
	client.Bind(clientConfig);

	client.Connect(server.GetServerIPConfig());
	server.WaitForConnection();

	CustomSocket::IPEndpoint clientIPInfo;
	EXPECT_EQ(client.GetSocketInfo(clientIPInfo),
			  CustomSocket::Result::Success);

	EXPECT_EQ(server.Send(clientIPInfo.GetIPString(),
						  clientIPInfo.GetPort(),
						  nullptr, 
						  0),
			  CustomSocket::Result::Success);

	server.Stop();
}

TEST(NonBlockingCallbackServer, NegativeSendTest)
{
	//const CustomSocket::IPEndpoint serverConfig("127.0.0.1", 0);

	CallbackServer server(serverConfig);
	server.Run();

	EXPECT_EQ(server.Send(clientIP, clientPort, nullptr, 0), CustomSocket::Result::Fail);

	server.Stop();
}

//---------------------------------------------------------

TEST(NonBlockingSocket, PositiveBindAndListenCompitabilityTest)
{
	EXPECT_TRUE(CustomSocket::NetworkAPIInitializer::Initialize());

	CustomSocket::Socket socket;
	
	EXPECT_EQ(socket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Bind(CustomSocket::IPEndpoint("127.0.0.1", 4790)),
			  CustomSocket::Result::Success);
	EXPECT_EQ(socket.Listen(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Close(), CustomSocket::Result::Success);

	EXPECT_EQ(socket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint config;
	EXPECT_EQ(socket.GetSocketInfo(config), 
			  CustomSocket::Result::Success);

	std::cout << config << std::endl;

	EXPECT_EQ(socket.Listen(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Close(), CustomSocket::Result::Success);

	EXPECT_EQ(socket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Bind(CustomSocket::IPEndpoint("127.0.0.1", 4790)),
		CustomSocket::Result::Success);
	EXPECT_EQ(socket.Listen(CustomSocket::IPEndpoint("127.0.0.1", 4790)), 
							CustomSocket::Result::Success);
	EXPECT_EQ(socket.Close(), CustomSocket::Result::Success);

	EXPECT_EQ(socket.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(socket.Bind(), CustomSocket::Result::Success);

	CustomSocket::NetworkAPIInitializer::Shutdown();
}

int main(int argc, char* argv[])
{
	/**
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
	**/

	///**
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
	//**/
}