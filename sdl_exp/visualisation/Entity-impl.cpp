#ifndef __Entity_impl_cpp__
#define __Entity_impl_cpp__
/*
This file contains all the template implementations of Entity, to force them to be compiled
*/
#include "Entity.cpp"
#include "Texture2D.h"

//Texture2D
template Entity::Entity(
    Stock::Models::Model const model,
    float scale,
    std::shared_ptr<Shaders> shaders,
    std::shared_ptr<Texture2D> texture
    );
template Entity::Entity(
    Stock::Models::Model const model,
    float scale,
    Stock::Shaders::ShaderSet const ss,
    std::shared_ptr<Texture2D> texture
    );
template Entity::Entity(
    const char *modelPath,
    float modelScale,
    Stock::Shaders::ShaderSet const ss,
    std::shared_ptr<Texture2D> texture
    );
template Entity::Entity(
    const char *modelPath,
    float modelScale,
    std::shared_ptr<Shaders> shaders,
    std::shared_ptr<Texture2D> texture
    );

//TextureCubeMap
#endif //ifndef __Entity_impl_cpp__