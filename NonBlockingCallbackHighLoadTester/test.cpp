#include "pch.h"

#include <NonBlockingCallbackServer/CalbackServer.h>
#include <NonBlockingCallbackClient/CallbackClient.h>
#include <NonBlockingServerPublisher/PublishingServer.h>
#include <NonBlockingClientSubscriber/SubscribingClient.h>

#include <boost/crc.hpp>
#include <cstdlib>
#include <ctime>

namespace TestToolkit
{
	void GetRandomString(char* string, unsigned int length);
}

TEST(HighLoadTestCase, AsynchServerSend) 
{
	//---------------------RUNNING SERVER---------------------------------//

	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//----------------------RUNNING CLIENT--------------------------------//

	CustomSocket::Socket client;

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint clientIPConfig;
	EXPECT_EQ(client.GetSocketInfo(clientIPConfig), CustomSocket::Result::Success);

	//----------------------CONNECTING AGENTS-----------------------------//

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	server.WaitForConnection();

	EXPECT_EQ(server.GetConnectionList(), std::vector<CustomSocket::IPEndpoint> { clientIPConfig });

	//----------------------SENDING MESSAGES------------------------------//

	const int bufSize = 4096;
	char* serverMessage = new char[bufSize];

	int bytesToRecieve = bufSize;
	char* clientBuffer = new char[bytesToRecieve] {};

	for (size_t index = 0; index < 1024; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//

		TestToolkit::GetRandomString(serverMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(server.Send(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
					serverMessage, bufSize), CustomSocket::Result::Success);
		EXPECT_EQ(server.WaitClientOnSendEvent(clientIPConfig.GetIPString(),
			clientIPConfig.GetPort()),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.RecieveAll(clientBuffer, bytesToRecieve), CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverMessageCheckSum;
		serverMessageCheckSum.process_bytes(serverMessage, bufSize);

		boost::crc_32_type clientBufferCheckSum;
		clientBufferCheckSum.process_bytes(clientBuffer, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientBufferCheckSum.checksum(), serverMessageCheckSum.checksum());

	}

	//----------------------CLOSING AGENTS--------------------------------//

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Close(), CustomSocket::Result::Success);

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverMessage;
	delete[] clientBuffer;
}

TEST(HighLoadTestCase, AsynchServerRecieve)
{
	//---------------------RUNNING SERVER---------------------------------//

	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//----------------------RUNNING CLIENT--------------------------------//

	CustomSocket::Socket client;

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint clientIPConfig;
	EXPECT_EQ(client.GetSocketInfo(clientIPConfig), CustomSocket::Result::Success);

	//----------------------CONNECTING AGENTS-----------------------------//

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	server.WaitForConnection();

	EXPECT_EQ(server.GetConnectionList(), std::vector<CustomSocket::IPEndpoint> { clientIPConfig });

	//----------------------RECIEVING MESSAGES----------------------------//

	const int bufSize = 4096;
	char* serverBuffer = new char[bufSize] {};

	int bytesToSend = bufSize;
	char* clientMessage = new char[bufSize];

	for (size_t index = 0; index < 1024; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//

		TestToolkit::GetRandomString(clientMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(client.SendAll(clientMessage, bytesToSend),
			CustomSocket::Result::Success);

		EXPECT_EQ(server.Recieve(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
								 serverBuffer, bufSize), 
				  CustomSocket::Result::Success);
		EXPECT_EQ(server.WaitClientOnRecieveEvent(clientIPConfig.GetIPString(),
												  clientIPConfig.GetPort()),
				  CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverBufferCheckSum;
		serverBufferCheckSum.process_bytes(serverBuffer, bufSize);

		boost::crc_32_type clientMessageCheckSum;
		clientMessageCheckSum.process_bytes(clientMessage, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientMessageCheckSum.checksum(), serverBufferCheckSum.checksum());
	}

	//----------------------CLOSING AGENTS--------------------------------//

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Close(), CustomSocket::Result::Success);

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverBuffer;
	delete[] clientMessage;
}

TEST(HighLoadTestCase, AsynchClientSend)
{
	//----------------------RUNNING CLIENT--------------------------------//

	CallbackClient client;
	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//---------------------RUNNING SERVER---------------------------------//

	CustomSocket::Socket server;

	EXPECT_EQ(server.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Bind(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig;
	EXPECT_EQ(server.GetSocketInfo(serverIPConfig), CustomSocket::Result::Success);

	//----------------------CONNECTING AGENTS-----------------------------//

	CustomSocket::Socket newConnectionSocket;

	std::thread serverThread(&CustomSocket::Socket::Accept,
							std::ref(server),
							std::ref(newConnectionSocket),
							nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	serverThread.join();

	//----------------------SENDING MESSAGES------------------------------//

	const int bufSize = 4096;
	char* clientMessage = new char[bufSize];

	int bytesToRecv = bufSize;
	char* serverBuffer = new char[bufSize] {};

	for (size_t index = 0; index < 1024; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//	

		TestToolkit::GetRandomString(clientMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(client.Send(clientMessage, bufSize),
				  CustomSocket::Result::Success);
		EXPECT_EQ(client.WaitOnSendEvent(), CustomSocket::Result::Success);

		EXPECT_EQ(newConnectionSocket.RecieveAll(serverBuffer, bytesToRecv),
				  CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverBufferCheckSum;
		serverBufferCheckSum.process_bytes(serverBuffer, bufSize);

		boost::crc_32_type clientMessageCheckSum;
		clientMessageCheckSum.process_bytes(clientMessage, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientMessageCheckSum.checksum(), serverBufferCheckSum.checksum());
	}

	//----------------------CLOSING AGENTS--------------------------------//
	
	EXPECT_EQ(newConnectionSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Close(), CustomSocket::Result::Success);

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverBuffer;
	delete[] clientMessage;
}

TEST(HighLoadTestCase, AsynchClientRecieve)
{
	//----------------------RUNNING CLIENT--------------------------------//

	CallbackClient client;
	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//---------------------RUNNING SERVER---------------------------------//

	CustomSocket::Socket server;

	EXPECT_EQ(server.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Bind(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig;
	EXPECT_EQ(server.GetSocketInfo(serverIPConfig), CustomSocket::Result::Success);

	//----------------------CONNECTING AGENTS-----------------------------//

	CustomSocket::Socket newConnectionSocket;

	std::thread serverThread(&CustomSocket::Socket::Accept,
		std::ref(server),
		std::ref(newConnectionSocket),
		nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	serverThread.join();


	//----------------------RECIEVING MESSAGES----------------------------//

	const int bufSize = 4096;

	int bytesToSend = bufSize;
	char* serverMessage = new char[bufSize];

	int bytesToRecv = bufSize;
	char* clientBuffer = new char[bytesToRecv] {};

	for (size_t index = 0; index < 1024; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//	

		TestToolkit::GetRandomString(serverMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(newConnectionSocket.SendAll(serverMessage, bytesToSend),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.Recieve(clientBuffer, bytesToRecv),
				  CustomSocket::Result::Success);
		EXPECT_EQ(client.WaitOnRecieveEvent(), CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverMessageCheckSum;
		serverMessageCheckSum.process_bytes(serverMessage, bufSize);

		boost::crc_32_type clientBufferCheckSum;
		clientBufferCheckSum.process_bytes(clientBuffer, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientBufferCheckSum.checksum(), serverMessageCheckSum.checksum());
	}

	//----------------------CLOSING AGENTS--------------------------------//

	EXPECT_EQ(newConnectionSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Close(), CustomSocket::Result::Success);

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverMessage;
	delete[] clientBuffer;
}

TEST(HighLoadTestCase, AsynchServerSendBigOne)
{
	//---------------------RUNNING SERVER---------------------------------//

	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//----------------------RUNNING CLIENT--------------------------------//

	CustomSocket::Socket client;

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint clientIPConfig;
	EXPECT_EQ(client.GetSocketInfo(clientIPConfig), CustomSocket::Result::Success);

	//----------------------CONNECTING AGENTS-----------------------------//

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	server.WaitForConnection();

	EXPECT_EQ(server.GetConnectionList(), std::vector<CustomSocket::IPEndpoint> { clientIPConfig });

	//----------------------SENDING MESSAGES------------------------------//

	const int bufSize = 4096 * 1024;
	char* serverMessage = new char[bufSize];

	int bytesToRecieve = bufSize;
	char* clientBuffer = new char[bytesToRecieve] {};

	for (size_t index = 0; index < 1; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//

		TestToolkit::GetRandomString(serverMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(server.Send(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
			serverMessage, bufSize), CustomSocket::Result::Success);
		EXPECT_EQ(server.WaitClientOnSendEvent(clientIPConfig.GetIPString(),
			clientIPConfig.GetPort()),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.RecieveAll(clientBuffer, bytesToRecieve), CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverMessageCheckSum;
		serverMessageCheckSum.process_bytes(serverMessage, bufSize);

		boost::crc_32_type clientBufferCheckSum;
		clientBufferCheckSum.process_bytes(clientBuffer, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientBufferCheckSum.checksum(), serverMessageCheckSum.checksum());
	}

	//----------------------CLOSING AGENTS--------------------------------//

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Close(), CustomSocket::Result::Success);

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverMessage;
	delete[] clientBuffer;
}

TEST(HighLoadTestCase, AsynchServerRecieveBigOne)
{
	//---------------------RUNNING SERVER---------------------------------//

	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//----------------------RUNNING CLIENT--------------------------------//

	CustomSocket::Socket client;

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint clientIPConfig;
	EXPECT_EQ(client.GetSocketInfo(clientIPConfig), CustomSocket::Result::Success);

	//----------------------CONNECTING AGENTS-----------------------------//

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	server.WaitForConnection();

	EXPECT_EQ(server.GetConnectionList(), std::vector<CustomSocket::IPEndpoint> { clientIPConfig });

	//----------------------RECIEVING MESSAGES----------------------------//

	const int bufSize = 4096 * 1024;
	char* serverBuffer = new char[bufSize] {};

	int bytesToSend = bufSize;
	char* clientMessage = new char[bufSize];


	for (size_t index = 0; index < 1; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//

		TestToolkit::GetRandomString(clientMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(client.SendAll(clientMessage, bytesToSend),
			CustomSocket::Result::Success);

		EXPECT_EQ(server.Recieve(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
			serverBuffer, bufSize),
			CustomSocket::Result::Success);
		EXPECT_EQ(server.WaitClientOnRecieveEvent(clientIPConfig.GetIPString(),
			clientIPConfig.GetPort()),
			CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverBufferCheckSum;
		serverBufferCheckSum.process_bytes(serverBuffer, bufSize);

		boost::crc_32_type clientMessageCheckSum;
		clientMessageCheckSum.process_bytes(clientMessage, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientMessageCheckSum.checksum(), serverBufferCheckSum.checksum());
	}

	//----------------------CLOSING AGENTS--------------------------------//

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Close(), CustomSocket::Result::Success);

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverBuffer;
	delete[] clientMessage;
}

TEST(HighLoadTestCase, AsynchClientSendBigOne)
{
	//----------------------RUNNING CLIENT--------------------------------//

	CallbackClient client;
	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//---------------------RUNNING SERVER---------------------------------//

	CustomSocket::Socket server;

	EXPECT_EQ(server.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Bind(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig;
	EXPECT_EQ(server.GetSocketInfo(serverIPConfig), CustomSocket::Result::Success);

	//----------------------CONNECTING AGENTS-----------------------------//

	CustomSocket::Socket newConnectionSocket;

	std::thread serverThread(&CustomSocket::Socket::Accept,
		std::ref(server),
		std::ref(newConnectionSocket),
		nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	serverThread.join();

	//----------------------SENDING MESSAGES------------------------------//

	const int bufSize = 4096 * 1024;
	char* clientMessage = new char[bufSize];

	int bytesToRecv = bufSize;
	char* serverBuffer = new char[bufSize] {};

	for (size_t index = 0; index < 1; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//	

		TestToolkit::GetRandomString(clientMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(client.Send(clientMessage, bufSize),
			CustomSocket::Result::Success);
		EXPECT_EQ(client.WaitOnSendEvent(), CustomSocket::Result::Success);

		EXPECT_EQ(newConnectionSocket.RecieveAll(serverBuffer, bytesToRecv),
			CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverBufferCheckSum;
		serverBufferCheckSum.process_bytes(serverBuffer, bufSize);

		boost::crc_32_type clientMessageCheckSum;
		clientMessageCheckSum.process_bytes(clientMessage, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientMessageCheckSum.checksum(), serverBufferCheckSum.checksum());
	}

	//----------------------CLOSING AGENTS--------------------------------//

	EXPECT_EQ(newConnectionSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Close(), CustomSocket::Result::Success);

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverBuffer;
	delete[] clientMessage;
}

TEST(HighLoadTestCase, AsynchClientRecieveBigOne)
{
	//----------------------RUNNING CLIENT--------------------------------//

	CallbackClient client;
	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//---------------------RUNNING SERVER---------------------------------//

	CustomSocket::Socket server;

	EXPECT_EQ(server.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Bind(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Listen(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig;
	EXPECT_EQ(server.GetSocketInfo(serverIPConfig), CustomSocket::Result::Success);

	//----------------------CONNECTING AGENTS-----------------------------//

	CustomSocket::Socket newConnectionSocket;

	std::thread serverThread(&CustomSocket::Socket::Accept,
		std::ref(server),
		std::ref(newConnectionSocket),
		nullptr);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	serverThread.join();


	//----------------------RECIEVING MESSAGES----------------------------//

	const int bufSize = 4096 * 1024;

	int bytesToSend = bufSize;
	char* serverMessage = new char[bufSize];

	int bytesToRecv = bufSize;
	char* clientBuffer = new char[bytesToRecv] {};

	for (size_t index = 0; index < 1; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//	

		TestToolkit::GetRandomString(serverMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(newConnectionSocket.SendAll(serverMessage, bytesToSend),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.Recieve(clientBuffer, bytesToRecv),
			CustomSocket::Result::Success);
		EXPECT_EQ(client.WaitOnRecieveEvent(), CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverMessageCheckSum;
		serverMessageCheckSum.process_bytes(serverMessage, bufSize);

		boost::crc_32_type clientBufferCheckSum;
		clientBufferCheckSum.process_bytes(clientBuffer, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientBufferCheckSum.checksum(), serverMessageCheckSum.checksum());
	}

	//----------------------CLOSING AGENTS--------------------------------//

	EXPECT_EQ(newConnectionSocket.Close(), CustomSocket::Result::Success);
	EXPECT_EQ(server.Close(), CustomSocket::Result::Success);

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverMessage;
	delete[] clientBuffer;
}

TEST(HighLoadTestCase, FullyAsynchServerToClientSend)
{
	//---------------------RUNNING SERVER---------------------------------//

	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//----------------------RUNNING CLIENT--------------------------------//

	CallbackClient client;
	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//----------------------CONNECTING AGENTS-----------------------------//


	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	//----------------------SENDING MESSAGES------------------------------//

	const int bufSize = 4096;
	char* serverMessage = new char[bufSize];

	int bytesToRecieve = bufSize;
	char* clientBuffer = new char[bytesToRecieve] {};

	for (size_t index = 0; index < 1024; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//

		TestToolkit::GetRandomString(serverMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(server.Send(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
							  serverMessage, bufSize), 
				  CustomSocket::Result::Success);
		EXPECT_EQ(server.WaitClientOnSendEvent(clientIPConfig.GetIPString(),
											   clientIPConfig.GetPort()),
				  CustomSocket::Result::Success);

		EXPECT_EQ(client.Recieve(clientBuffer, bytesToRecieve), CustomSocket::Result::Success);
		EXPECT_EQ(client.WaitOnRecieveEvent(), CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverMessageCheckSum;
		serverMessageCheckSum.process_bytes(serverMessage, bufSize);

		boost::crc_32_type clientBufferCheckSum;
		clientBufferCheckSum.process_bytes(clientBuffer, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientBufferCheckSum.checksum(), serverMessageCheckSum.checksum());
	}

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverMessage;
	delete[] clientBuffer;
}

TEST(HighLoadTestCase, FullyAsynchServerFromClientRecieve)
{
	//---------------------RUNNING SERVER---------------------------------//

	CallbackServer server;

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//----------------------RUNNING CLIENT--------------------------------//

	CallbackClient client;
	CustomSocket::IPEndpoint clientIPConfig = client.GetClientIPConfig();

	//----------------------CONNECTING AGENTS-----------------------------//


	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	//----------------------RECIEVING MESSAGES----------------------------//

	const int bufSize = 4096;
	char* serverBuffer = new char[bufSize] {};

	int bytesToSend = bufSize;
	char* clientMessage = new char[bufSize];

	for (size_t index = 0; index < 1024; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//

		TestToolkit::GetRandomString(clientMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(client.Send(clientMessage, bufSize),
			CustomSocket::Result::Success);
		EXPECT_EQ(client.WaitOnSendEvent(), CustomSocket::Result::Success);

		EXPECT_EQ(server.Recieve(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
			serverBuffer, bufSize), CustomSocket::Result::Success);
		EXPECT_EQ(server.WaitClientOnRecieveEvent(clientIPConfig.GetIPString(),
			clientIPConfig.GetPort()),
			CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverBufferCheckSum;
		serverBufferCheckSum.process_bytes(serverBuffer, bufSize);

		boost::crc_32_type clientMessageCheckSum;
		clientMessageCheckSum.process_bytes(clientMessage, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientMessageCheckSum.checksum(), serverBufferCheckSum.checksum());
	}

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverBuffer;
	delete[] clientMessage;

}

TEST(HighLoadTestCase, PublisherSubscriberTest)
{
	//---------------------CREATING LOGGER--------------------------------//

	SubscribingLogger logger;

	//---------------------RUNNING SERVER---------------------------------//

	PublishingServer server(CustomSocket::IPEndpoint("127.0.0.1", 0));
	server.Subscribe(logger);

	EXPECT_EQ(server.Run(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint serverIPConfig = server.GetServerIPConfig();

	//----------------------RUNNING CLIENT--------------------------------//

	CustomSocket::Socket client;

	EXPECT_EQ(client.Create(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Bind(), CustomSocket::Result::Success);

	CustomSocket::IPEndpoint clientIPConfig;
	EXPECT_EQ(client.GetSocketInfo(clientIPConfig), CustomSocket::Result::Success);

	//----------------------CONNECTING AGENTS-----------------------------//

	EXPECT_EQ(client.Connect(serverIPConfig), CustomSocket::Result::Success);
	server.WaitForConnection();

	EXPECT_EQ(server.GetConnectionList(), std::vector<CustomSocket::IPEndpoint> { clientIPConfig });

	//----------------------SENDING MESSAGES------------------------------//

	const int bufSize = 4096;
	char* serverMessage = new char[bufSize];

	int bytesToRecieve = bufSize;
	char* clientBuffer = new char[bytesToRecieve] {};

	for (size_t index = 0; index < 1024; index++)
	{
		//--------------------PREPARE SENDING MESSAGE---------------------//

		TestToolkit::GetRandomString(serverMessage, bufSize);

		//--------------------PERFORMING TESTED OPERATION-----------------//

		EXPECT_EQ(server.Send(clientIPConfig.GetIPString(), clientIPConfig.GetPort(),
			serverMessage, bufSize), CustomSocket::Result::Success);
		EXPECT_EQ(server.WaitClientOnSendEvent(clientIPConfig.GetIPString(),
			clientIPConfig.GetPort()),
			CustomSocket::Result::Success);

		EXPECT_EQ(client.RecieveAll(clientBuffer, bytesToRecieve), CustomSocket::Result::Success);

		//--------------------PREPARING CHECKSUM--------------------------//

		boost::crc_32_type serverMessageCheckSum;
		serverMessageCheckSum.process_bytes(serverMessage, bufSize);

		boost::crc_32_type clientBufferCheckSum;
		clientBufferCheckSum.process_bytes(clientBuffer, bufSize);

		//--------------------CHECKING CHECKSUM---------------------------//

		EXPECT_EQ(clientBufferCheckSum.checksum(), serverMessageCheckSum.checksum());

	}

	//----------------------CLOSING AGENTS--------------------------------//

	EXPECT_EQ(server.Stop(), CustomSocket::Result::Success);
	EXPECT_EQ(client.Close(), CustomSocket::Result::Success);

	//----------------------CLEARING HEAP---------------------------------//

	delete[] serverMessage;
	delete[] clientBuffer;
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

namespace TestToolkit
{
	void GetRandomString(char* string, unsigned int length)
	{
		std::srand(static_cast<unsigned int>(std::time(nullptr)));

		const std::string alphabet = (std::string("ABCDEFGHIJKLMNOPQRSTUVWXYZ") + 
									  std::string("abcdefghijklmnopqrstuvwxyz") + 
									  std::string("0123456789") + std::string("!@#$%^&*()_+=-"));

		for (size_t index = 0; index < length; index++)
		{
			auto randomCharIndex = rand() % alphabet.size();
			string[index] = alphabet[randomCharIndex];
		}
	}
}