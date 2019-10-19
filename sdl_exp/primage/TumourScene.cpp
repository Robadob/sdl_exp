#include "TumourScene.h"
#include <string>
#include <fstream>


TumourScene::SceneContent::SceneContent(std::shared_ptr<LightsBuffer> lights, const fs::path &tumourDataDirectory)
    : lights(lights)
	//, sphereModel(new Entity(Stock::Models::SPHERE, 10.0f, { std::make_shared<Shaders>("../sdl_exp/primage/instanced.vert", "../sdl_exp/primage/material_phong.frag") }))
	, sphereModel(new Entity(Stock::Models::SPHERE, 10.0f, { std::make_shared<Shaders>("../sdl_exp/primage/instanced.vert", "material_flat.frag") }))
{
    loadCells(tumourDataDirectory);
	sphereModel->setMaterial(Stock::Materials::RED_PLASTIC);
}
void TumourScene::SceneContent::loadCells(const fs::path &tumourDataDirectory)
{
	int totalCt = 0;
	size_t fileCt_max = 0;
    for (int i = 0; i < 100;++i)
    {
		std::string tryFile = (tumourDataDirectory / fs::path(std::to_string(i) + std::string(".bin"))).string();
		std::ifstream ifs;
		ifs.open(tryFile, std::ios::in | std::ios::binary | std::ios::ate);
		if (!ifs.is_open())
			break;
		size_t sz = ifs.tellg();
		ifs.close();
		int fileCt = sz / (3 * sizeof(float));
		fileCt_max = fileCt > fileCt_max ? fileCt : fileCt_max;
		cells.push_back({ totalCt, fileCt });
		totalCt += fileCt;
    }
	//Allocate Texture Buffers
	cellX = TextureBuffer<float>::make(totalCt, 1);
	cellY = TextureBuffer<float>::make(totalCt, 1);
	cellZ = TextureBuffer<float>::make(totalCt, 1);
	char *t_buffer = (char*)malloc(fileCt_max * sizeof(float));
	for (int i = 0; i < cells.size(); ++i)
	{
		std::string tryFile = (tumourDataDirectory / fs::path(std::to_string(i) + std::string(".bin"))).string();
		std::ifstream ifs;
		ifs.open(tryFile, std::ios::in | std::ios::binary);
		if (ifs.is_open())
		{
			ifs.read(t_buffer, cells[i].count * sizeof(float));
			cellX->setData((float*)t_buffer, cells[i].count * sizeof(float), cells[i].offset * sizeof(float));
			ifs.read(t_buffer, cells[i].count * sizeof(float));
			cellY->setData((float*)t_buffer, cells[i].count * sizeof(float), cells[i].offset * sizeof(float));
			ifs.read(t_buffer, cells[i].count * sizeof(float));
			cellZ->setData((float*)t_buffer, cells[i].count * sizeof(float), cells[i].offset * sizeof(float));
			assert(ifs.good());
			ifs.close();
		}
	}
	free(t_buffer);
}

TumourScene::TumourScene(Visualisation &visualisation, const fs::path &tumourDataDirectory)
    : MultiPassScene(visualisation)
    , content(std::make_shared<SceneContent>(Lights(), tumourDataDirectory))
	, fPass(std::make_shared<FinalPass>(content))
{
	//Register models
	registerEntity(content->sphereModel);
	//Register render passes in correct order
	addPass(3, fPass);
	//Enable defaults
	this->visualisation.setWindowTitle("PRIMAGE Neuroblastoma Visualiser");

	//Approx sun point light 
	//Really wants to be abitrary directional light, but putting it really far out with constant attenuation should also work
	//DirectionalLight p = Lights()->addDirectionalLight();
	//p.Direction(glm::normalize(glm::vec3(-1, 0, 1)));
	//p.Ambient(glm::vec3(0.89f, 0.64f, 0.36f));
	//p.Diffuse(glm::vec3(1.0f, 0.75f, 0.39f));//else 1.0f, 1.0f, 0.49f (more yellow, less orange)
	//p.Specular(glm::vec3(1, 1, 1));
	//p.ConstantAttenuation(1.0f);
	PointLight _p = Lights()->addPointLight();
	_p.Ambient(glm::vec3(0.0f));
	_p.Diffuse(glm::vec3(0.5f));
	_p.Specular(glm::vec3(0.02f));
	_p.ConstantAttenuation(0.5f);

	auto sphere0 = content->sphereModel->getShaders(0);
	sphere0->addTexture("_texBufX", content->cellX);
	sphere0->addTexture("_texBufY", content->cellY);
	sphere0->addTexture("_texBufZ", content->cellZ);
	    
}
void TumourScene::update(const unsigned int &frameTime)
{
	auto p = Lights()->getPointLight(0);
	p.Position(visualisation.getCamera()->getEye());
	//auto d = Lights()->getDirectionalLight(0);
	//d.Direction(std::dynamic_pointer_cast<NoClipCamera const>(visualisation.getCamera())->getLook());
}
bool TumourScene::keypress(SDL_Keycode keycode, int x, int y)
{
	switch (keycode)
	{
	default:
		//Permit the keycode to be processed if we haven't handled personally
		return true;
	}
	return false;
}

void TumourScene::reload()
{
	//content->blur->reload();
}
TumourScene::FinalPass::FinalPass(std::shared_ptr<SceneContent> content)
	: RenderPass(std::make_shared<BackBuffer>())
	, content(content)

{
}
//Uses the shadow map to render the normal scene
void TumourScene::FinalPass::render()
{
	//Slightly blur shadow map to reduce harshness of edges
	//content->blur->blurR32F(content->shadowIn, content->shadowOut);
	//Generate mip-map
	//content->shadowOut->updateMipMap();
	//Render models using shadow map
	//content->sphereModel->renderInstances(content->cells[0].count, 0);
	content->sphereModel->renderInstances(1000, 0);
	//Render something at the lights location
	//content->lights->render();
}