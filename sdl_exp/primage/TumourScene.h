#ifndef __TumourScene_h__
#define __TumourScene_h__

#include <memory>
#include <vector>
#include "../visualisation/multipass/MultiPassScene.h"
#include "../visualisation/texture/TextureBuffer.h"
#include "../visualisation/Entity.h"

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
		std::shared_ptr<TextureBuffer<float>> cellX, cellY, cellZ;
		std::shared_ptr<Entity> sphereModel;
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
    std::shared_ptr<SceneContent> content;

	std::shared_ptr<FinalPass> fPass;
};

#endif //__TumourScene_h__