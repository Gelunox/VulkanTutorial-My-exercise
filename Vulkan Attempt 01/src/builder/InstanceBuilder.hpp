#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "../util/Globals.hpp"

using namespace std;

namespace com::gelunox::vulcanUtils
{
	class InstanceBuilder
	{
	public:
		typedef InstanceBuilder& This;
	private:
		VkApplicationInfo appInfo = {};
		VkInstanceCreateInfo createInfo = {};

		vector<const char*> extensions;
	public:
		InstanceBuilder();

		This setApplicationName( const char * name );
		This setApplicationVersion( uint32_t version );
		This setEngineName( const char * name );
		This setEngineVersion( uint32_t version );

		This addExtension( const char* extension );
		This addExtensions( vector<const char*> extensions );
		This setValidationLayersEnabled( bool enable );

		VkInstance build();
	};
};