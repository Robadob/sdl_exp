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