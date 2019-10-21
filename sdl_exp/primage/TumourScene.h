#ifndef __TumourScene_h__
#define __TumourScene_h__

#include <memory>
#include <vector>
#include "../visualisation/multipass/MultiPassScene.h"
#include "../visualisation/texture/TextureBuffer.h"
#include "../visualisation/Entity.h"
#include "../visualisation/Text.h"

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
        SceneContent(std::shared_ptr<LightsBuffer> lights, const fs::path &tumourDataDirectory);
        void loadCells(const fs::path &tumourDataDirectory);
        std::shared_ptr<LightsBuffer> lights;
        std::vector<CellMetaData> cells;
		std::shared_ptr<TextureBuffer<float>> cellX, cellY, cellZ, cellP53;
		std::shared_ptr<Entity> sphereModel;
		int cellIndex = 0;
		int instancedRenderOffset = 0;
    };
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
	std::shared_ptr<FinalPass> fPass;
};

#endif //__TumourScene_h__