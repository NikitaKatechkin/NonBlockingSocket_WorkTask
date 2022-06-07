#include "SubscribingClient.h"

SubscribingClient::SubscribingClient(const CustomSocket::IPEndpoint& clientConfig) :
	CallbackClient(clientConfig),
	Subscriber()
{
}

SubscribingClient::SubscribingClient(const std::string& IP, const uint16_t port):
	CallbackClient(IP, port),
	Subscriber()
{
}
