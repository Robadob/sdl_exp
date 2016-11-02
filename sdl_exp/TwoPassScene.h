#ifndef __TwoPassScene_h__
#define __TwoPassScene_h__
#include "visualisation/multipass/MultiPassScene.h"
#include "visualisation/Entity.h"
#include "visualisation/Skybox.h"
#include "visualisation/Axis.h"

class TwoPassScene : public MultiPassScene
{
	struct SceneContent
	{
		SceneContent();
		std::shared_ptr<Entity> deerModel;
		std::shared_ptr<Skybox> skybox;
		std::shared_ptr<Axis> axis;
	};
	class VelocityPass : public RenderPass
	{
	public:
		VelocityPass(std::shared_ptr<SceneContent> content);
		void setSkybox(bool i){ renderSkybox = i; }
		void setAxis(bool i){ renderAxis = i; }
	protected:
		void render() override;
	private:
		bool renderSkybox;
		bool renderAxis;
		std::shared_ptr<SceneContent> content;
	};
	class ColorPass : public RenderPass
	{
	public:
		ColorPass(std::shared_ptr<SceneContent> content);
		void setSkybox(bool i){ renderSkybox = i; }
		void setAxis(bool i){ renderAxis = i; }
	protected:
		void render() override;
	private:
		bool renderSkybox;
		bool renderAxis;
		std::shared_ptr<SceneContent> content;
	};
	class MotionBlurCompositePass : public RenderPass
	{
	public:
		MotionBlurCompositePass(std::shared_ptr<SceneContent> content);
		void setVelocityTex(GLuint tex);
		void setColorTex(GLuint tex);
		void setDepthTex(GLuint tex);
	protected:
		GLuint vTex;
		GLuint cTex;
		GLuint dTex;
		void render() override;
		std::shared_ptr<SceneContent> content;
		std::shared_ptr<Shaders> compositeShader;
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

	std::shared_ptr<VelocityPass> vPass;
	std::shared_ptr<ColorPass> cPass;
	std::shared_ptr<MotionBlurCompositePass> mbcPass;

	float tick;
	float tick2;
	int polarity;
};

#endif //__TwoPassScene_h__