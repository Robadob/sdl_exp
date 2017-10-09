#ifndef __TwoPassScene_h__
#define __TwoPassScene_h__
#include "visualisation/multipass/MultiPassScene.h"
#include "visualisation/Entity.h"
#include "visualisation/Sprite2D.h"
#include "visualisation/shader/GaussianBlur.h"

/**
 * Provides an example of how multi pass rendering can be used within sdl_exp
 * This scene provides a basic implementation of exponential shadow mapping
 * It doesn't implement detection and replacement of failure as described in section 3 of the paper
 * It also doesn't support spot lights, only point lights (had some issues handling perspectie projection mat's)
 */
class TwoPassScene : public MultiPassScene
{
	/**
	 * This struct holds all of the scene content, it is to be shared with all the render pass instances
	 */
	struct SceneContent
	{
		SceneContent();
		//Models used
        std::shared_ptr<Entity> deerModel;
        std::shared_ptr<Entity> sphereModel;
        std::shared_ptr<Entity> planeModel;
        std::shared_ptr<Entity> lightModel;
		//Blur provider
        std::shared_ptr<GaussianBlur> blur;
		//Texture we will blur into
        std::shared_ptr<Texture2D> gaussTex;
		//Point light position, target, view and projection matrices
        glm::vec3 pointlightPos, pointlightTarget;
        glm::mat4 pointlightV;
        glm::mat4 pointlightP;
		//Dimensions of the shadow texture
        glm::uvec2 shadowDims;
		//GL names of the rendered to and blurred to shadow textures
        GLuint shadowIn, shadowOut;
	};
	/**
	 * This is the first render pass, we use to to generate the shadow map
	 */
	class ShadowPass : public RenderPass
	{
	public:
        ShadowPass(std::shared_ptr<SceneContent> content);
    protected:
		void render() override;
	private:
		std::shared_ptr<SceneContent> content;
	};
	/**
	 * This is the second render pass, we use to render the objects, passing the shadow map texture as a parameter
	 */
	class CompositePass : public RenderPass
	{
	public:
        CompositePass(std::shared_ptr<SceneContent> content);
	protected:
		void render() override;
		std::shared_ptr<SceneContent> content;
	};
public:
	TwoPassScene(ViewportExt &visualisation);
	void reload() override;
	bool keypress(SDL_Keycode keycode, int x, int y) override;
	void update(unsigned int frameTime) override;
private:
	std::shared_ptr<SceneContent> content;

    std::shared_ptr<ShadowPass> sPass;
    std::shared_ptr<CompositePass> cPass;
    std::shared_ptr<Sprite2D> shadowMapPreview;

	float tick;
	float tick2;
	int polarity;
};

#endif //__TwoPassScene_h__