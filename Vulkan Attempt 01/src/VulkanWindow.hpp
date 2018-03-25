#pragma once

#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <chrono>

#include <GLFW/glfw3.h>
#include <vector>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util/Util.hpp"
#include "Vertex.hpp"
#include "UniformBufferObject.hpp"
#include "QueueIndices.hpp"
#include "Swapchain.hpp"

#include "builder/InstanceBuilder.hpp"
#include "builder/LogicalDeviceBuilder.hpp"

using namespace std;


namespace com::gelunox::vulcanUtils
{
	typedef chrono::time_point<chrono::steady_clock> timepoint;

	class VulkanWindow
	{
	private:
		int width = 500;
		int height = 500;
		timepoint startTime = chrono::high_resolution_clock::now();

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

		Swapchain * swapchain;

		//it would better to have a single buffer with offsets
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexMemory;
		VkBuffer uniformBuffer;
		VkDeviceMemory uniformMemory;

		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;

		VkCommandPool commandpool;
		vector<VkCommandBuffer> commandBuffers;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;

		QueueIndices queueIndices;

		const vector<Vertex> vertices =
		{
			{ { -0.8f, -0.8f },{ 1.0f, 0.0f, 0.0f } },
			{ {  0.8f, -0.8f },{ 0.0f, 1.0f, 0.0f } },
			{ {  0.8f,  0.8f },{ 0.0f, 0.0f, 1.0f } },
			{ { -0.8f,  0.8f },{ 0.5f, 0.0f, 0.5f } }
		};
		const vector<uint16_t> indices =
		{
			0, 1, 2,
			2, 3, 0
		};

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

		void recreateSwapchain();

		void createCommandpool();

		void createBuffers();
		void createMemory( VkDeviceSize size, const void * srcData, VkBuffer& dstBuffer, VkDeviceMemory& dstMemory, VkBufferUsageFlagBits flags );
		void createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags property, VkBuffer &buffer, VkDeviceMemory &memory );
		void copyBuffer( VkBuffer src, VkBuffer dst, VkDeviceSize size );

		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSet();

		void createCommandbuffers();
		void createSemaphores();

		void update();
		void drawFrame();
	};
}