#include "InstanceBuilder.hpp"

using namespace com::gelunox::vulcanUtils;

typedef InstanceBuilder::This This;

InstanceBuilder::InstanceBuilder()
{
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "nameless application";
	appInfo.pEngineName = "nameless engine";
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_0;

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
}

This InstanceBuilder::setApplicationName( const char* name )
{
	appInfo.pApplicationName = name;

	return *this;
}

This InstanceBuilder::setApplicationVersion( uint32_t version )
{
	appInfo.applicationVersion = version;

	return *this;
}

This InstanceBuilder::setEngineName( const char* name )
{
	appInfo.pEngineName = name;

	return *this;
}

This InstanceBuilder::setEngineVersion( uint32_t version )
{
	appInfo.engineVersion = version;

	return *this;
}

This InstanceBuilder::addExtension( const char* extension )
{
	extensions.push_back( extension );

	return *this;
}

This InstanceBuilder::addExtensions( vector<const char*> extensions )
{
	this->extensions.insert( this->extensions.end(), extensions.begin(), extensions.end() );

	return *this;
}

This InstanceBuilder::setValidationLayersEnabled( bool enable )
{
	if (enable)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(VULKAN_VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VULKAN_VALIDATION_LAYERS.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	return *this;
}

VkInstance InstanceBuilder::build()
{
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());;
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkInstance instance;
	if (vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS)
	{
		throw runtime_error( "can't create vulkan instance" );
	}
	return instance;
}