#ifndef __Model_assimpUtils_h__
#define __Model_assimpUtils_h__

#include <assimp/types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
/**
 * Methods are stored here to hide assimp from header files
 * and to provide utilities for converting assimp objects to our local formats
 * without bloating Model.cpp
 */
namespace au
{
    std::shared_ptr<Material> getMaterial(aiMaterial *aiMat)
    {
        std::shared_ptr<Material> mat = std::make_shared<Material>();

        return mat;
    }

};

#endif //__Model_assimpUtils_h__