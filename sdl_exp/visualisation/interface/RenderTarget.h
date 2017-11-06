#ifndef __RenderTarget_h__
#define __RenderTarget_h__

#include "../util/GLcheck.h"
#include <glm/vec2.hpp>

class RenderTarget
{
public:
	virtual ~RenderTarget()
	{ }

	virtual void resize(const glm::uvec2 dimensions) = 0;
	virtual GLenum getName() const = 0;
};

#endif //__RenderTarget_h__