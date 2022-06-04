#include "CallbackClient.h"

CallbackClient::CallbackClient(const CustomSocket::IPEndpoint& clientConfig):
	Client(clientConfig)
{
}

CallbackClient::CallbackClient(const std::string& IP, const uint16_t port):
	Client(IP, port)
{
}

void CallbackClient::OnConnect()
{
	Client::OnConnect();
}

void CallbackClient::OnDisconnect()
{
	Client::OnDisconnect();
}

void CallbackClient::OnRecieve(char* data, int& bytesRecieved)
{
	Client::OnRecieve(data, bytesRecieved);
}

void CallbackClient::OnSend(const char* data, int& bytesSent)
{
	Client::OnSend(data, bytesSent);
}
