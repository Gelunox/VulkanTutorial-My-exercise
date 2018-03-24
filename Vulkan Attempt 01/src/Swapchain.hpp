#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

#include "QueueIndices.hpp"
#include "GraphicsPipeline.hpp"

#include "builder/SwapchainBuilder.hpp"
#include "builder/ImageViewBuilder.hpp"
#include "builder/FramebufferBuilder.hpp"

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class Swapchain
	{
	private:
		const int width;
		const int height;

		GraphicsPipeline * pipeline;

		const VkDevice device;

		VkSwapchainKHR swapchain;

		VkExtent2D extent;
		VkFormat imageFormat;

		vector<VkImage> images;
		vector<VkImageView> imageViews;
		vector<VkFramebuffer> frameBuffers;

	public:
		Swapchain( int width, int height, VkPhysicalDevice physicalDevice, VkDevice device,
			VkSurfaceKHR surface, QueueIndices queueIndices, VkDescriptorSetLayout descriptorLayout );
		Swapchain( int width, int height, VkPhysicalDevice physicalDevice, VkDevice device,
			VkSurfaceKHR surface, QueueIndices queueIndices, VkDescriptorSetLayout descriptorLayout, Swapchain * oldSwapchain );
		~Swapchain();

		VkSwapchainKHR getSwapchain() { return swapchain; }
		GraphicsPipeline * getPipeline() { return pipeline; }

		VkExtent2D getExtent() { return extent; }
		VkFormat getImageFormat() { return imageFormat; }
		vector<VkImage> getImages() { return images; }
		vector<VkImageView> getImageViews() { return imageViews; }
		vector<VkFramebuffer> getFrameBuffers() { return frameBuffers; }

	private:
		void createSwapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, QueueIndices queueIndices, VkSwapchainKHR oldSwapchain );
		void createImages( VkDevice device );
		void createFrameBuffers();

		static VkSurfaceCapabilitiesKHR getSurfaceCapabilities( VkPhysicalDevice device, VkSurfaceKHR surface );
		static VkExtent2D getExtent( uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR& capabilities );
		static VkSurfaceFormatKHR getSurfaceFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );
		static VkPresentModeKHR getPresentMode( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );
	};
}