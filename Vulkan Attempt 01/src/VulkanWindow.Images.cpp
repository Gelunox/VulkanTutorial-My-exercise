#include "VulkanWindow.hpp"

using namespace com::gelunox::vulcanUtils;
using namespace std;

void VulkanWindow::createImage()
{
	memFac.createTextureImage( "textures/chibi.png", textureImage, textureImageMemory );
	textureImageView = ImageViewBuilder( logicalDevice )
		.setFormat( VK_FORMAT_R8G8B8A8_UNORM )
		.setImage( textureImage )
		.build();
	textureSampler = SamplerBuilder( logicalDevice ).build();
}