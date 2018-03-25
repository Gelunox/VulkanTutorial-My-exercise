#include "VulkanWindow.hpp"

#include <iostream>

using namespace com::gelunox::vulcanUtils;

int main()
{
	try {
		{
			VulkanWindow window;

			window.run();
		}
	}
	catch (runtime_error e)
	{
		std::cerr << e.what() << std::endl;
	}

	std::cout << "Press enter to continue...";
	getchar();

	return 0;
}
