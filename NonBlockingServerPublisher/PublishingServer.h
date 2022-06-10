#pragma once

#include <NonBlockingCallbackServer/CalbackServer.h>
#include <PublisherObserverPatternLib/Publisher.h>

class PublishingServer final : public Publisher, public CallbackServer
{
public:
	PublishingServer(const CustomSocket::IPEndpoint& IPconfig);
	PublishingServer(const std::string& ip = "127.0.0.1", const uint16_t port = 0);
	~PublishingServer() = default;

protected:
	virtual void OnConnect(const std::string& ip, const uint16_t port) override;
	virtual void OnDisconnect(const std::string& ip, const uint16_t port) override;
	virtual void OnSend(const std::string& ip, const uint16_t port, 
						const char* data, int& bytesSent) override;
	virtual void OnRecieve(const std::string& ip, const uint16_t port,
						   char* data, int& bytesRecieved) override;
};