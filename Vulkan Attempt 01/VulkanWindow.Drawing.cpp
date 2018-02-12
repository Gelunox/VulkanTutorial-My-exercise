#include "VulkanWindow.hpp"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>


//https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Framebuffers
//https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffers

using namespace com::gelunox::vulcanUtils;
using namespace std;

void VulkanWindow::createFramebuffers()
{
	swapchainFramebuffers.resize( swapchainImageViews.size() );

	for (size_t i = 0; i < swapchainImageViews.size(); i++)
	{
		VkImageView attachments[] = { swapchainImageViews[i] };
		VkFramebufferCreateInfo framebuffInfo = {};
		framebuffInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffInfo.renderPass = renderPass;
		framebuffInfo.attachmentCount = 1;
		framebuffInfo.pAttachments = attachments;
		framebuffInfo.width = swapchainExtent.width;
		framebuffInfo.height = swapchainExtent.height;
		framebuffInfo.layers = 1;

		if (vkCreateFramebuffer( logicalDevice, &framebuffInfo, nullptr, &swapchainFramebuffers[i] ) != VK_SUCCESS)
		{
			throw runtime_error( "framebuffer could not be created" );
		}
	}
}

void VulkanWindow::createCommandpool()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = graphicsQIndex;
	poolInfo.flags = 0;

	if (vkCreateCommandPool( logicalDevice, &poolInfo, nullptr, &commandpool ) != VK_SUCCESS)
	{
		throw runtime_error( "commandpool creation failed" );
	}
}

void VulkanWindow::createCommandbuffers()
{
	commandBuffers.resize( swapchainFramebuffers.size() );

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandpool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers( logicalDevice, &allocInfo, commandBuffers.data() ) != VK_SUCCESS)
	{
		throw runtime_error( "command buffer allocation failed" );
	}

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer( commandBuffers[i], &beginInfo );

		VkClearValue clearColor = { .0f, .0f, 0.0f, 1.0f };
		VkRenderPassBeginInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.renderPass = renderPass;
		renderpassInfo.framebuffer = swapchainFramebuffers[i];
		renderpassInfo.renderArea.offset = { 0,0 };
		renderpassInfo.renderArea.extent = swapchainExtent;
		renderpassInfo.clearValueCount = 1;
		renderpassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass( commandBuffers[i], &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE );
		vkCmdBindPipeline( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );
		vkCmdDraw( commandBuffers[i], 3, 1, 0, 0 );
		vkCmdEndRenderPass( commandBuffers[i] );

		if (vkEndCommandBuffer( commandBuffers[i] ) != VK_SUCCESS)
		{
			throw runtime_error( "command buffer recording failed" );
		}
	}
}

void VulkanWindow::createSemaphores()
{
	VkSemaphoreCreateInfo spInfo = {};
	spInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore( logicalDevice, &spInfo, nullptr, &imageAvailableSemaphore ) != VK_SUCCESS
		|| vkCreateSemaphore( logicalDevice, &spInfo, nullptr, &renderFinishedSemaphore ) != VK_SUCCESS)
	{
		throw runtime_error( "semaphore creation failed" );
	}
}

void VulkanWindow::drawFrame()
{
	uint32_t imageIndex;
	vkAcquireNextImageKHR( logicalDevice, swapchain, numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex );
	
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	VkPipelineStageFlags waitstages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitstages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit( graphicsQ, 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS)
	{
		throw runtime_error( "draw submission failed" );
	}

	VkSwapchainKHR swapchains[] = { swapchain };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR( presentQ, &presentInfo );
}