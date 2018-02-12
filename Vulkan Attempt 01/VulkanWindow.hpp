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
		int width = 500;
		int height = 500;

		const vector<const char*> validationLayers =
		{
			"VK_LAYER_LUNARG_standard_validation"
		};
		const bool enableValidationLayers = true;
		VkDebugReportCallbackEXT callback;

		const vector<const char*> deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		GLFWwindow * window;

		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		VkSurfaceKHR surface;

		VkQueue graphicsQ;
		VkQueue presentQ;

		VkSwapchainKHR swapchain;
		vector<VkImage> swapchainImages;
		vector<VkImageView> swapchainImageViews;
		VkFormat swapchainImageFormat;
		VkExtent2D swapchainExtent;

		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		vector<VkFramebuffer> swapchainFramebuffers;
		VkCommandPool commandpool;
		vector<VkCommandBuffer> commandBuffers;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;

		int graphicsQIndex = -1;
		int presentationQIndex = -1;

	public:
		static bool isSuitableGpu( VkPhysicalDevice device );

		VulkanWindow();
		~VulkanWindow();

		void run();

		void onWindowResized( int width, int height );
		static void onWindowResized( GLFWwindow * window, int width, int height );

	private:
		void selectPhysicalDevice();
		void createLogicalDevice();
		void findQFamilyIndexes();

		void cleanupSwapchain();
		void recreateSwapchain();
		void createSwapchain();
		void createImages();

		void createRenderPass();
		void createGraphicsPipeline();

		void createFramebuffers();
		void createCommandpool();
		void createCommandbuffers();
		void createSemaphores();

		void drawFrame();

		VkSurfaceCapabilitiesKHR getSurfaceCapabilities();
		VkExtent2D getSwapExtent( VkSurfaceCapabilitiesKHR& capabilities );
		VkSurfaceFormatKHR getSurfaceFormat();
		VkPresentModeKHR getPresentMode();

		VkShaderModule createShaderModule( const vector<char>& code );
	};
}