#pragma once

#include <NonBlockingServer/Server.h>

class CallbackServer final : public Server
{
public:
	CallbackServer(const CustomSocket::IPEndpoint& IPconfig);
	CallbackServer(const std::string& ip, const uint16_t port);

protected:
	virtual void RecieveProcessing(const std::string& ip, const uint16_t port) override;
	virtual void SendProcessing(const std::string& ip, const uint16_t port) override;
};