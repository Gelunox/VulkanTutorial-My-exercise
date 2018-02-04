#pragma once

#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <GLFW/glfw3.h>
#include <vector>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class VulkanWindow
	{
	private:
		GLFWwindow * window;

		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkSurfaceKHR surface;

		VkSwapchainKHR swapchain;
		vector<VkImage> swapchainImages;
		vector<VkImageView> swapchainImageViews;
		VkFormat swapchainImageFormat;
		VkExtent2D swapchainExtent;

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		int graphicsQIndex = -1;
		int presentationQIndex = -1;

	public:
		static bool isSuitableGpu( VkPhysicalDevice device );

		VulkanWindow();
		~VulkanWindow();

		void run();

	private:
		void selectPhysicalDevice();
		void createLogicalDevice();
		void findQFamilyIndexes();

		void buildSwapchain();
		void buildImages();

		void buildGraphicsPipeline();

		VkSurfaceCapabilitiesKHR getSurfaceCapabilities();
		VkExtent2D getSwapExtent( VkSurfaceCapabilitiesKHR& capabilities );
		VkSurfaceFormatKHR getSurfaceFormat();
		VkPresentModeKHR getPresentMode();

		VkShaderModule createShaderModule( const vector<char>& code );
	};
}