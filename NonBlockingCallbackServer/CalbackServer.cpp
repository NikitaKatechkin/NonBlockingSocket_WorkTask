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

void CallbackServer::OnSend(const std::string& ip, const uint16_t port, 
							const char* data, int& bytesSent)
{
	Server::OnSend(ip, port, data, bytesSent);
}

void CallbackServer::OnRecieve(const std::string& ip, const uint16_t port, 
							   char* data, int& bytesRecieved)
{
	Server::OnRecieve(ip, port, data, bytesRecieved);
}

void CallbackServer::OnConnect(const std::string& ip, const uint16_t port)
{
	Server::OnConnect(ip, port);
}

void CallbackServer::OnDisconnect(const std::string& ip, const uint16_t port)
{
	Server::OnDisconnect(ip, port);
}

/**
void CallbackServer::RecieveProcessing(const std::string& ip, const uint16_t port)
{
	Server::RecieveProcessing(ip, port);
}

void CallbackServer::SendProcessing(const std::string& ip, const uint16_t port)
{
	Server::SendProcessing(ip, port);
}
**/
