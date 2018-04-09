#include "SamplerBuilder.hpp"

using namespace com::gelunox::vulcanUtils;

typedef SamplerBuilder::This This;

SamplerBuilder::SamplerBuilder(VkDevice& device): device(device)
{
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR; //interpolation magnification
	samplerInfo.minFilter = VK_FILTER_LINEAR; //interpolation minification
	//repetition mode in each direction
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;//x
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;//y
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;//z
	
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE; // 0 to 1 vs 0 to texture-length

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = .0f;
	samplerInfo.minLod = .0f;
	samplerInfo.maxLod = .0f;
}

VkSampler SamplerBuilder::build()
{
	VkSampler sampler;

	if (vkCreateSampler( device, &samplerInfo, nullptr, &sampler ) != VK_SUCCESS)
	{
		throw runtime_error( "failed to create image sampler" );
	}

	return sampler;
}