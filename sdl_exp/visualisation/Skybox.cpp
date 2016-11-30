#include "Skybox.h" 

/*
Constructs a skybox entity
@param texturePath The directory containing the textures for the cubemap
@param yOffset The vertical offset to the skybox view (setting this further than around -2.0f-2.0f can make the corners visible of the skybox if it was not designed for off center)
@note The directory should contain the /, and textures should be named in the form right/left/up/down/front/back,(png|tga|bmp)
*/
Skybox::Skybox(const char *texturePath, float yOffset)
    : Entity(
        Stock::Models::CUBE, 
        50.0f,//Make it adequate distance away
		{ Stock::Shaders::SKYBOX },
        std::make_shared<TextureCubeMap>(texturePath)
    )
{    
    //Set height
    setLocation(glm::vec3(0, yOffset, 0));
    //Flip entity vertex order
    flipVertexOrder();
    //Flip normals
}
/*
Renders the skybox
*/
void Skybox::render(unsigned int shaderIndex)
{
    bool isBlend=false;
    glGetBooleanv(GL_BLEND, &isBlend);
    // Enable/Disable features
    GL_CALL(glDisable(GL_DEPTH_TEST));
    if(isBlend)
      GL_CALL(glDisable(GL_BLEND));
    Entity::render(shaderIndex);
    if(isBlend)
      GL_CALL(glEnable(GL_BLEND));
}
/**
 * Overrides the Entity setModelViewMatPtr, to allow the skybox ModelViewMatrix to be used
 */
void Skybox::setViewMatPtr(const Camera *camera)
{
	for (auto &&it : shaders)
		if (it)
			it->setViewMatPtr(camera->getSkyboxViewMatPtr());
}
/**
 * Adjusts the vertical offset of the skybox
 * @param yOffset The desired offset
 * @note Offsets large than -2.0f-2.0f can make the corners of the skybox visible
 */
void Skybox::setYOffset(float yOffset)
{
    setLocation(glm::vec3(0, yOffset, 0));
}