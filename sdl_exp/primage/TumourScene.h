#ifndef __TumourScene_h__
#define __TumourScene_h__

#include <memory>
#include <vector>
#include "../visualisation/multipass/MultiPassScene.h"
#include "../visualisation/texture/TextureBuffer.h"
#include "../visualisation/Entity.h"
#include "../visualisation/Text.h"
#include "../visualisation/shader/GaussianBlur.h"
#include "../visualisation/Sprite2D.h"
#include "CellBillboard.h"
#include "../visualisation/Draw.h"
#include "TextBillboard.h"

#ifdef _MSC_VER
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
namespace fs = std::experimental::filesystem;

class TumourScene : public MultiPassScene
{
    struct CellMetaData
    {
        int offset;
        int count;
		float oxygen;
		float drug;
		float VEGF;
		float MMP;
		float TIMP;
		float MPTP;
    };
    /**
    * This struct holds all of the scene content, it is to be shared with all the render pass instances
    */
    struct SceneContent
    {
        SceneContent(std::shared_ptr<LightsBuffer> lights, const fs::path &tumourDataDirectory, std::shared_ptr<const NoClipCamera>);
        void loadCells(const fs::path &tumourDataDirectory);
        std::shared_ptr<LightsBuffer> lights;
        std::vector<CellMetaData> cells;
		std::shared_ptr<TextureBuffer<float>> cellX, cellY, cellZ, cellP53;
		std::shared_ptr<Entity> sphereModel;
		std::shared_ptr<CellBillboard> cellModel;
		int cellIndex = 0;
		int instancedRenderOffset = 0;
		//Blur provider
		std::shared_ptr<GaussianBlur> blur;
		//GL names of the rendered to and blurred to shadow textures
		std::shared_ptr<Texture2D> depthIn, depthOut;
        Draw grid;
        glm::vec3 camera_pos;
        std::map<int, std::shared_ptr<TextBillboard>> labels;
    };
	/**
	 * Basic solo pass for instanced rendering
	 */
	class SpherePass : public RenderPass
	{
	public:
		SpherePass(std::shared_ptr<SceneContent> content, std::shared_ptr<const NoClipCamera> camera);
	protected:
		void render() override;
        void renderGrid();
		std::shared_ptr<SceneContent> content;
	};
	/**
	 * Depth pass for implicit surface
	 */
	class DepthPass : public RenderPass
	{
	public:
		DepthPass(std::shared_ptr<SceneContent> content);
	protected:
		void render() override;
		std::shared_ptr<SceneContent> content;
	};
	/**
	 * Final pass for implicit surface
	 */
	class FinalPass : public RenderPass
	{
	public:
		FinalPass(std::shared_ptr<SceneContent> content);
	protected:
		void render() override;
		std::shared_ptr<SceneContent> content;
	};
public:
    TumourScene(Visualisation &visualisation, const fs::path &tumourDataDirectory);
    void reload() override;
    bool keypress(SDL_Keycode keycode, int x, int y) override;
    void update(const unsigned int &frameTime) override;
private:
	void setFrameCt();
    std::shared_ptr<SceneContent> content;
	std::shared_ptr<Text> frameCt, ec_evm;
	std::shared_ptr<SpherePass> spherePass;

	std::shared_ptr<Sprite2D> depthMapPreview;
	std::shared_ptr<DepthPass> dPass;
	std::shared_ptr<FinalPass> fPass;
	void TumourScene::toggleImplicitSurface();
	bool implictSurfaceActive;
};

#endif //__TumourScene_h__