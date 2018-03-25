#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

#include "QueueIndices.hpp"
#include "util/Util.hpp"
#include "Vertex.hpp"

#include "builder/SwapchainBuilder.hpp"
#include "builder/ImageViewBuilder.hpp"
#include "builder/FramebufferBuilder.hpp"
#include "builder/RenderPassBuilder.hpp"
#include "builder/PipelineBuilder.hpp"
#include "builder/PipelineLayoutBuilder.hpp"


using namespace std;

namespace com::gelunox::vulcanUtils
{
	class Swapchain
	{
	private:
		const int width;
		const int height;

		VkDevice device;

		VkSwapchainKHR swapchain;

		VkExtent2D extent;
		VkFormat imageFormat;

		vector<VkImage> images;
		vector<VkImageView> imageViews;

		VkRenderPass renderPass;
		VkPipeline graphics;
		VkPipelineLayout layout;

		vector<VkFramebuffer> frameBuffers;

	public:
		Swapchain( int width, int height, VkPhysicalDevice physicalDevice, VkDevice device,
			VkSurfaceKHR surface, QueueIndices queueIndices, VkDescriptorSetLayout descriptorLayout );
		Swapchain( int width, int height, VkPhysicalDevice physicalDevice, VkDevice device,
			VkSurfaceKHR surface, QueueIndices queueIndices, VkDescriptorSetLayout descriptorLayout, Swapchain * oldSwapchain );
		~Swapchain();

		VkSwapchainKHR getSwapchain() { return swapchain; }

		VkExtent2D getExtent() { return extent; }
		VkFormat getImageFormat() { return imageFormat; }
		vector<VkImage> getImages() { return images; }
		vector<VkImageView> getImageViews() { return imageViews; }
		VkRenderPass getRenderPass() { return renderPass; }
		VkPipeline getPipeline() { return graphics; }
		VkPipelineLayout getPipelineLayout() { return layout; }
		vector<VkFramebuffer> getFrameBuffers() { return frameBuffers; }

	private:
		void createSwapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, QueueIndices queueIndices, VkSwapchainKHR oldSwapchain );
		void createImages();
		void createRenderpass( VkFormat imageFormat );
		void createPipeline( VkDescriptorSetLayout descriptorLayout );
		void createFrameBuffers();
	};
}