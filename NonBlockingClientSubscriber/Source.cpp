#include <NonBlockingClientSubscriber/SubscribingClient.h>
#include <iostream>

int main()
{
	SubscribingClient client;

	std::cout << client.GetClientIPConfig() << std::endl;

	system("pause");
	return 0;
}