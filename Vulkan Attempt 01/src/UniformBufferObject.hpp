#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

using namespace glm;
using namespace std;

namespace com::gelunox::vulcanUtils
{
	struct UniformBufferObject
	{
		mat4 model;
		mat4 view;
		mat4 proj;
	};
}