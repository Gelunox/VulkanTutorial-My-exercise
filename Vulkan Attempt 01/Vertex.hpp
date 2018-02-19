#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

using namespace glm;
using namespace std;

namespace com::gelunox::vulcanUtils
{
	struct Vertex
	{
		vec2 position;
		vec3 color;

		static VkVertexInputBindingDescription getBindDescription()
		{
			VkVertexInputBindingDescription bindDescription = {};
			bindDescription.binding = 0;
			bindDescription.stride = sizeof( Vertex );
			bindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindDescription;
		}

		static array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof( Vertex, position );

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof( Vertex, color );

			return attributeDescriptions;
		}
	};
}