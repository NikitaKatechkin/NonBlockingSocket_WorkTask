#include "CalbackServer.h"

#include <iostream>

CallbackServer::CallbackServer(const CustomSocket::IPEndpoint& IPconfig):
	Server(IPconfig)
{
}

CallbackServer::CallbackServer(const std::string& ip, const uint16_t port):
	Server(ip, port)
{
}

void CallbackServer::RecieveProcessing(const std::string& ip, const uint16_t port)
{
	Server::RecieveProcessing(ip, port);
}

void CallbackServer::SendProcessing(const std::string& ip, const uint16_t port)
{
	Server::SendProcessing(ip, port);
}
