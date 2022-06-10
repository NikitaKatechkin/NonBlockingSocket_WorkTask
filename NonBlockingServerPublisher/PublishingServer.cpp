#include "PublishingServer.h"

PublishingServer::PublishingServer(const CustomSocket::IPEndpoint& IPconfig):
	CallbackServer(IPconfig),
	Publisher()
{
}

PublishingServer::PublishingServer(const std::string& ip, const uint16_t port):
	CallbackServer(ip, port),
	Publisher()
{
}

void PublishingServer::OnConnect(const std::string& ip, const uint16_t port)
{
	CallbackServer::OnConnect(ip, port);
	Notify();
}

void PublishingServer::OnDisconnect(const std::string& ip, const uint16_t port)
{
	CallbackServer::OnDisconnect(ip, port);
	Notify();
}

void PublishingServer::OnSend(const std::string& ip, const uint16_t port, 
							  const char* data, int& bytesSent)
{
	CallbackServer::OnSend(ip, port, data, bytesSent);
	Notify();
}

void PublishingServer::OnRecieve(const std::string& ip, const uint16_t port, 
								 char* data, int& bytesRecieved)
{
	CallbackServer::OnRecieve(ip, port, data, bytesRecieved);
	Notify();
}
