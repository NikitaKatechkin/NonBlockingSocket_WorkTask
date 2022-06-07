#pragma once

#include <NonBlockingCallbackClient/CallbackClient.h>
#include <PublisherObserverPatternLib/Subscriber.h>

class SubscribingClient final : public CallbackClient, public Subscriber
{
public:
	SubscribingClient(const CustomSocket::IPEndpoint& clientConfig);
	SubscribingClient(const std::string& IP = "127.0.0.1", const uint16_t port = 0);
	~SubscribingClient() = default;
protected:
};