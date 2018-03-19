#include "VulkanWindow.hpp"

#include <iostream>

using namespace com::gelunox::vulcanUtils;

int main()
{
	{
		VulkanWindow window;

		window.run();
	}

	std::cout << "Press enter to continue...";
	getchar();

	return 0;
}
