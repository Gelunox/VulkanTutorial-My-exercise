#include "VulkanWindow.hpp"

using namespace com::gelunox::vulcanUtils;
using namespace std;

void VulkanWindow::createBuffers()
{
	memFac.createBufferMemory( sizeof( vertices[0] ) * vertices.size(), vertices.data(), vertexBuffer, vertexMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );
	memFac.createBufferMemory( sizeof(  indices[0] ) *  indices.size(),  indices.data(),  indexBuffer , indexMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT );

	//uniformbuffer
	memFac.createBuffer( sizeof( UniformBufferObject ),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		uniformBuffer, uniformMemory );

	memFac.createTextureImage( "textures/chibi.png", textureImage, textureImageMemory );
}