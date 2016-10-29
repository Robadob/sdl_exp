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
	class ScenePass : public RenderPass
	{
	public:
		ScenePass(std::shared_ptr<SceneContent> content);
		void setSkybox(bool i){ renderSkybox = i; }
		void setAxis(bool i){ renderAxis = i; }
	protected:
		void render() override;
	private:
		bool renderSkybox;
		bool renderAxis;
		std::shared_ptr<SceneContent> content;
	};
	class PostProcessPass : public RenderPass
	{

	};
public:
	TwoPassScene(Visualisation &visualisation);
	void reload() override;
	bool keypress(SDL_Keycode keycode, int x, int y) override;
	void update(unsigned int frameTime) override;
private:
	std::shared_ptr<SceneContent> content;
	std::shared_ptr<ScenePass> sPass;
	std::shared_ptr<PostProcessPass> ppPass;

	float tick;
	int polarity;
};

#endif //__TwoPassScene_h__