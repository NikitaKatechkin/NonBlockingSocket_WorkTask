#pragma once

#include "Subscriber.h"

#include <vector>

class Publisher
{
public:
	bool Subscribe(Subscriber& newSubscriber);
	bool Unsubscribe(Subscriber& subscriber);

	bool Notify(const Subscriber& subscriber);
	void Notify();
protected:
	std::vector<Subscriber*> m_subscriberList;
};