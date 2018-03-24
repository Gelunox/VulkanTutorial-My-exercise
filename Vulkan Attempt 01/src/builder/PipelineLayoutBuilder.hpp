#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class PipelineLayoutBuilder
	{
	public:
		typedef PipelineLayoutBuilder & This;

	private:
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

		VkDevice device;

		vector<VkDescriptorSetLayout> descriptorSetLayouts;
	public:
		PipelineLayoutBuilder(VkDevice & device);

		This addDescriptorSetLayout( VkDescriptorSetLayout & layout );

		VkPipelineLayout build();
	};
};