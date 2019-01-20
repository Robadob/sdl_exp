#ifndef __PortableDraw_h__
#define __PortableDraw_h__
#include "model/SceneGraphItem.h"
#include "Draw.h"
#include "util/StringUtils.h"

/**
 * This class isn't threadsafe!
 */
class PortableDraw : public SceneGraphItem
{
    friend class Draw;
    std::string name;
    std::shared_ptr<Draw::DrawData> data;
    PortableDraw(const std::string &name, std::shared_ptr<Draw::DrawData> data)
        : name(name)
        , data(data)
    {
        const auto &&f = data->stateDirectory.find(name);
        if (f == data->stateDirectory.end())
        {
            throw std::runtime_error(su::format("Draw state '%s' was not found for makePortable().\n", name.c_str()));
        }
        f->second.portableCount++;
        setSceneMat(glm::mat4(1));
        setLocalMat(glm::mat4(1));
        setInternalMat(glm::mat4(1));
    }
public:
    virtual ~PortableDraw()
    {
        auto &&f = data->stateDirectory.find(name);
        if (f != data->stateDirectory.end())
        {
            assert(f->second.portableCount != 0);
            f->second.portableCount--;
        }
    }
    void render(const unsigned int &shaderIndex = UINT_MAX, const glm::mat4 &transform = glm::mat4(1)) override
    {
        const auto &&f = data->stateDirectory.find(name);
        if (f == data->stateDirectory.end())
        {
            throw std::runtime_error(su::format("Draw state '%s' was not found for PortableDraw::render().\n", name.c_str()));
        }

        Draw::setWidth(f->second.mType, f->second.mWidth);
        data->shaders->useProgram();
        glm::mat4 m = transform * getModelMatRef();
        data->shaders->overrideModelMat(&m);
        GL_CALL(glDrawArrays(Draw::toGL(f->second.mType), f->second.offset, f->second.count));
        data->shaders->clearProgram();
        Draw::clearWidth(f->second.mType);
    }
};

#endif //__PortableDraw_h__