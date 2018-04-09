#include "VulkanWindow.hpp"


//https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Framebuffers
//https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffers

using namespace com::gelunox::vulcanUtils;
using namespace std;

void VulkanWindow::createCommandpool()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueIndices.graphics;
	poolInfo.flags = 0;

	if (vkCreateCommandPool( logicalDevice, &poolInfo, nullptr, &commandpool ) != VK_SUCCESS)
	{
		throw runtime_error( "commandpool creation failed" );
	}

	memFac.setCommandPool( commandpool );
}

void VulkanWindow::createCommandbuffers()
{
	auto framebuffers = swapchain->getFrameBuffers();
	commandBuffers.resize( framebuffers.size() );

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
		vkCmdBindDescriptorSets( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain->getPipelineLayout(),
			0, 1, &descriptorSet, 0, nullptr);

		VkClearValue clearColor = { .0f, .0f, 0.0f, 1.0f };
		VkRenderPassBeginInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.renderPass = swapchain->getRenderPass();
		renderpassInfo.framebuffer = framebuffers[i];
		renderpassInfo.renderArea.offset = { 0,0 };
		renderpassInfo.renderArea.extent = swapchain->getExtent();
		renderpassInfo.clearValueCount = 1;
		renderpassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass( commandBuffers[i], &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE );
		vkCmdBindPipeline( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain->getPipeline() );

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize  offsets[] = { 0 };
		vkCmdBindVertexBuffers( commandBuffers[i], 0, 1, vertexBuffers, offsets );
		vkCmdBindIndexBuffer( commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16 );

		vkCmdDrawIndexed( commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0 );
		vkCmdEndRenderPass( commandBuffers[i] );

		if (vkEndCommandBuffer( commandBuffers[i] ) != VK_SUCCESS)
		{
			throw runtime_error( "command buffer recording failed" );
		}
	}
}

//TODO: look into a descriptor builder/factory
//descriptorType is repeated 3 times, correlation between layout, pool, and write
//need to account for creation order? descriptorSetLayout is needed when creating the swapchain (or earlier than the other parts at least)
void VulkanWindow::createDescriptorSetLayout()
{
	descriptorSetLayout = DescriptorSetLayoutBuilder( logicalDevice )
		.addBinding( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr )
		.addBinding( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr )
		.build();
}

void VulkanWindow::createDescriptorPool()
{
	descriptorPool = DescriptorPoolBuilder( logicalDevice )
		.addPoolSize( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 )
		.addPoolSize( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 )
		.build();
}

void VulkanWindow::createDescriptorSet()
{
	VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets( logicalDevice, &allocInfo, &descriptorSet ) != VK_SUCCESS)
	{
		throw runtime_error( "couldn't create descriptorset" );
	}

	VkWriteDescriptorSet descriptorWrites[2] = { {},{} };

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof( UniformBufferObject );

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = textureImageView;
	imageInfo.sampler = textureSampler;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;
	
	vkUpdateDescriptorSets( logicalDevice, 2, descriptorWrites, 0, nullptr );
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

//https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets
void VulkanWindow::update()
{
	timepoint now = chrono::high_resolution_clock::now();

	float time = chrono::duration<float, chrono::seconds::period>( now - startTime ).count() ;

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
	ubo.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
	ubo.proj = glm::perspective( glm::radians( 45.0f ),
		swapchain->getExtent().width / (float)swapchain->getExtent().height,
		0.1f, 10.0f );

	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory( logicalDevice, uniformMemory, 0, sizeof( ubo ) , 0, &data );
	memcpy( data, &ubo, (size_t)sizeof(ubo) );
	vkUnmapMemory( logicalDevice, uniformMemory );
}

void VulkanWindow::drawFrame()
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR( logicalDevice, swapchain->getSwapchain(), numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex );
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
		return;
	}
	if( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
	{
		throw runtime_error( "error getting swapchain image" );
	}

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

	VkSwapchainKHR swapchains[] = { swapchain->getSwapchain() };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR( presentQ, &presentInfo );

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapchain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error( "failed to present swap chain image!" );
	}

	vkQueueWaitIdle( presentQ );
}