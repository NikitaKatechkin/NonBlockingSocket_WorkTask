#pragma once

#include <NonBlockingClient/Client.h>

class CallbackClient final : public Client
{
public:
	CallbackClient(const CustomSocket::IPEndpoint& clientConfig);
	CallbackClient(const std::string& IP = "127.0.0.1", const uint16_t port = 0);
	~CallbackClient() = default;
protected:
	virtual void OnConnect() override;
	virtual void OnDisconnect() override;

	virtual void OnRecieve(char* data, int& bytesRecieved);
	virtual void OnSend(const char* data, int& bytesSent);
};