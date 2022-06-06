#pragma once

#include <NonBlockingServer/Server.h>

class CallbackServer final : public Server
{
public:
	CallbackServer(const CustomSocket::IPEndpoint& IPconfig);
	CallbackServer(const std::string& ip = "127.0.0.1", const uint16_t port = 0);

protected:
	//virtual void RecieveProcessing(const std::string& ip, const uint16_t port) override;
	//virtual void SendProcessing(const std::string& ip, const uint16_t port) override;

	virtual void OnSend(const std::string& ip, const uint16_t port,
		const char* data, int& bytesSent) override;
	virtual void OnRecieve(const std::string& ip, const uint16_t port,
		char* data, int& bytesRecieved) override;
	virtual void OnConnect(const std::string& ip, const uint16_t port) override;
	virtual void OnDisconnect(const std::string& ip, const uint16_t port) override;
};