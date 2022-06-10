#pragma once

#include <NonBlockingCallbackClient/CallbackClient.h>
#include <PublisherObserverPatternLib/Subscriber.h>

class SubscribingLogger final : public Subscriber
{
public:
	SubscribingLogger() = default;
	~SubscribingLogger() = default;

	virtual void Update() override;
};