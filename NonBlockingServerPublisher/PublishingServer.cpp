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
	//m_connections.find(CustomSocket::IPEndpoint(ip, port));
	//Subscribe();
}
