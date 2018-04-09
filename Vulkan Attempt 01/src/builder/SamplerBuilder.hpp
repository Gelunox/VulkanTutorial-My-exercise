#pragma once

#include <vulkan/vulkan.h>
#include <vector>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class SamplerBuilder
	{
	public:
		typedef SamplerBuilder& This;
	private:
		VkSamplerCreateInfo samplerInfo = {};

		VkDevice device;
	public:
		SamplerBuilder(VkDevice& device);

		VkSampler build();
	};

};