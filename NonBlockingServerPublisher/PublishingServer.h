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
};