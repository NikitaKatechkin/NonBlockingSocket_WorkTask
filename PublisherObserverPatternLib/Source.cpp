#include <iostream>

#include "Publisher.h"

int main()
{
	Publisher pub;
	Subscriber sub;

	std::cout << pub.Subscribe(sub) << std::endl;
	
	pub.Notify();
	std::cout << pub.Notify(sub) << std::endl;

	std::cout << pub.Unsubscribe(sub) << std::endl;

	return 0;
}