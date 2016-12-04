#ifndef __TwoPassScene_h__
#define __TwoPassScene_h__
#include "visualisation/multipass/MultiPassScene.h"
#include "visualisation/Entity.h"
#include "visualisation/Skybox.h"
#include "visualisation/Axis.h"
#include "visualisation/Sprite2D.h"

class TwoPassScene : public MultiPassScene
{
	struct SceneContent
	{
		SceneContent();
		std::shared_ptr<Entity> deerModel;
	};
	class ShadowPass : public RenderPass
	{
	public:
        ShadowPass(std::shared_ptr<SceneContent> content);
	protected:
		void render() override;
	private:
		std::shared_ptr<SceneContent> content;
	};
	class CompositePass : public RenderPass
	{
	public:
        CompositePass(std::shared_ptr<SceneContent> content);
		void setShadowMap(GLuint tex);
	protected:
		GLuint shadowMap;
		GLuint cTex;
		GLuint dTex;
		void render() override;
		std::shared_ptr<SceneContent> content;
		std::shared_ptr<Entity> frameEnt;
		std::shared_ptr<Texture2D> sampleTex;
		glm::mat4 mvMat;
		glm::mat4 projMat;

	};
public:
	TwoPassScene(Visualisation &visualisation);
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