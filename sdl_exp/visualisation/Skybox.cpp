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
        TextureCubeMap::load(texturePath)
    )
{    
    //Set height
    setLocalTranslation(glm::vec3(0, yOffset, 0));
    //Flip entity vertex order
    flipVertexOrder();
    //Flip normals
}
/*
Renders the skybox
*/
void Skybox::depthRenderInstances(const GLenum &glDepthFc, int count, unsigned int shaderIndex)
{
    GL_CALL(glDepthFunc(glDepthFc));
    Entity::renderInstances(count, shaderIndex);
    GL_CALL(glDepthFunc(GL_LESS));
}
void Skybox::renderInstances(int count, unsigned int shaderIndex)
{
    depthRenderInstances(GL_LEQUAL, count, shaderIndex);
}

void Skybox::depthRender(const GLenum &glDepthFc, const unsigned int &shaderIndex, const glm::mat4 &transform)
{
    GL_CALL(glDepthFunc(glDepthFc));
    Entity::render(0, transform);
    GL_CALL(glDepthFunc(GL_LESS));
}
void Skybox::render(const unsigned int &shaderIndex, const glm::mat4 &transform)
{
    depthRender(GL_LEQUAL, shaderIndex, transform);
}
/**
 * Overrides the Entity setModelViewMatPtr, to allow the skybox ModelViewMatrix to be used
 */
void Skybox::setViewMatPtr(std::shared_ptr<const Camera> camera)
{
	Entity::setViewMatPtr(camera->getSkyboxViewMatPtr());
}
/**
 * Adjusts the vertical offset of the skybox
 * @param yOffset The desired offset
 * @note Offsets large than -2.0f-2.0f can make the corners of the skybox visible
 */
void Skybox::setYOffset(float yOffset)
{
    setLocalTranslation(glm::vec3(0, yOffset, 0));
}