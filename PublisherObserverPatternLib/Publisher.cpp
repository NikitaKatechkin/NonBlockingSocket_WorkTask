#include "Publisher.h"

#include <algorithm>

bool Publisher::Subscribe(Subscriber& newSubscriber)
{
	bool result = 
		(std::find(m_subscriberList.begin(), m_subscriberList.end(), &newSubscriber)
		== m_subscriberList.end());

	if (result == true)
	{
		m_subscriberList.push_back(&newSubscriber);
	}

	return result;
}

bool Publisher::Unsubscribe(Subscriber& subscriber)
{
	auto unsubscribableElement = std::find(m_subscriberList.begin(), 
										   m_subscriberList.end(), 
										   &subscriber);
	bool result = (unsubscribableElement != m_subscriberList.end());

	if (result == true)
	{
		m_subscriberList.erase(unsubscribableElement);
	}

	return result;
}

bool Publisher::Notify(const Subscriber& subscriber)
{
	auto subscriberToNotify = std::find(m_subscriberList.begin(),
										m_subscriberList.end(),
										&subscriber);
	bool result = (subscriberToNotify != m_subscriberList.end());

	if (result == true)
	{
		//subscriberToNotify->Update();
		(*subscriberToNotify)->Update();
	}

	return result;
}

void Publisher::Notify()
{
	for (auto& subscriber : m_subscriberList)
	{
		subscriber->Update();
	}
}
