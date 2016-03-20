#include "Skybox.h" 

/*
Constructs a skybox entity
@param projectionMat Pointer to the projection matrix to be used during render
@param modelViewMat Pointer to the model view matrix to be used during render
@param texturePath Path to directory containing files named left/right/back/front/up/down.png (bmp is also suitable).
@param texturePath The provided must end with a /
*/
Skybox::Skybox(const char *texturePath, float yOffset)
    : Entity(
        Stock::Models::CUBE, 
        50.0f,//Make it adequete distance away
        Stock::Shaders::SKYBOX,
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
void Skybox::render()
{
    glPushMatrix();
    ////Setup shaders

    // Enable/Disable features
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP);
    //glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glDepthMask(GL_FALSE);
    Entity::render();
    glDepthMask(GL_TRUE);

    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();
}


void Skybox::setModelViewMatPtr(const Camera *camera)
{
    if (shaders.get())
        shaders->setModelViewMatPtr(camera->getSkyboxViewMatPtr());
}
void Skybox::setYOffset(float yOffset)
{
    setLocation(glm::vec3(0, yOffset, 0));
}