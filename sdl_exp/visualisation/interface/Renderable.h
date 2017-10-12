#ifndef __Renderable_h__
#define __Renderable_h__

#include "Reloadable.h"
#include <memory>
#include <glm/glm.hpp>

#include "Viewport.h"
#include "Camera.h"
#include <vector>

/**
 * Represents things which hold shaders (and can be rendered)
 * @note Entities are the target audience
 */
class Renderable : public Reloadable
{
public:
	/**
	 * Binds the provided modelview matrix to the internal shader
	 * @param viewMat Ptr to modelview matrix
	 * @note This is normally found within the Camera object
	 */
    virtual void setViewMatPtr(glm::mat4 const *viewMat) = 0;
	/**
	 * Binds the provided projection matrix to the internal shader
	 * @param projectionMat Ptr to model view matrix
	 * @note This is normally found within the Viewport object
	 */
	virtual void setProjectionMatPtr(glm::mat4 const *projectionMat) = 0;
	/**
	 * Binds the provided view matrix to the internal shader
	 * @param camera Ptr to view matrix
	 * @note This is normally found within the Camera object
	 * @note This method is overriden by Skybox as it required modelview sans translation
	 */
	virtual void setViewMatPtr(std::shared_ptr<const Camera> camera)
	{
		setViewMatPtr(camera->getViewMatPtr());
	}
	virtual void setViewMatPtr(const Camera *camera)
	{
		setViewMatPtr(camera->getViewMatPtr());
	}
	/**
	 * Binds the provided projection matrix to the internal shader
	 * @param visualisation Ptr to model view matrix
	 * @note Convenience wrapper
	 */
	virtual void setProjectionMatPtr(std::shared_ptr<const Viewport> visualisation) final
	{
		setProjectionMatPtr(visualisation->getProjMatPtr());
	}
	virtual void setProjectionMatPtr(const Viewport *visualisation) final
	{
		setProjectionMatPtr(visualisation->getProjMatPtr());
	}
    //Render
    virtual glm::mat4 render(const unsigned int &shaderIndex=0, glm::mat4 transform = glm::mat4()) = 0;
    void renderSceneGraph(const unsigned int &shaderIndex = 0, glm::mat4 transform = glm::mat4())
    {
        //Render me with transform
        transform = render(shaderIndex, transform);
        //Render children with transform
        for (const auto &pair : children)
        {
            if (auto child = std::get<0>(pair).lock())
            {
				const glm::mat4 *ptr = std::get<1>(pair);
				child->renderSceneGraph(std::get<2>(pair), ptr == nullptr ? transform : (transform*(*ptr)));
            }
        }
    }
    //Scene graph mgmt
    void addChild(std::weak_ptr<Renderable> a, const glm::mat4 *b = nullptr, unsigned int shaderIndex = 0)
    {
        children.push_back(std::make_tuple( a, b, shaderIndex));
    };
    void clearChildren()
    {
        children.clear();
    }
    unsigned int removeChildren(const std::shared_ptr<Renderable> &a)
    {
        unsigned int rtn = 0;
        for (auto &&it = children.begin(); it != children.end();)
        {
            if (std::get<0>(*it).lock())
            {
                it = children.erase(it);
                rtn++;
                continue;
            }
            ++it;
        }
        return rtn;
    }
	unsigned int removeChildren(const glm::mat4 * &b)
    {
        unsigned int rtn = 0;
        for (auto &&it = children.begin(); it != children.end();)
        {
            if (std::get<1>(*it) == b)
            {
                it = children.erase(it);
                rtn++;
                continue;
            }
            ++it;
        }
        return rtn;
    }
	unsigned int removeChildren(const std::shared_ptr<Renderable> &a, const glm::mat4 * &b, const unsigned int shaderIndex = 0)
    {
        unsigned int rtn = 0;
        for (auto &&it = children.begin(); it != children.end();)
        {
            if (std::get<0>(*it).lock() == a&&std::get<1>(*it) == b)
            {
                it = children.erase(it);
                rtn++;
                continue;
            }
            ++it;
        }
        return rtn;
    }
	void copySceneGraph(std::shared_ptr<Renderable> r)
    {
		children.clear();
		children.insert(children.end(), r->children.begin(), r->children.end());
    }
private:
	std::list<std::tuple<std::weak_ptr<Renderable>, const glm::mat4 *, unsigned int>> children;

};

#endif //__Renderable_h__