#include "TexturedEntity.h"

TexturedEntity::TexturedEntity(Stock::Models::Model model, float scale, std::shared_ptr<Shaders> shaders)
    : Entity(model, scale, shaders)
    , tex(model.texturePath)
{
    tex.bindToShader(shaders.get());
}

TexturedEntity::TexturedEntity(const char *modelPath, const char *textureFile, float modelScale, std::shared_ptr<Shaders> shaders)
    : Entity(modelPath, modelScale, shaders)
    , tex(textureFile)
{
    tex.bindToShader(shaders.get());    
}

void TexturedEntity::render()
{
    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    Entity::render();
    glDisable(GL_TEXTURE_2D);
    
    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();
}
void TexturedEntity::renderInstances(int count)
{

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    Entity::renderInstances(count);
    glPopMatrix();
}