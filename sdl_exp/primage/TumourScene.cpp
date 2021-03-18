#include "TumourScene.h"
#include <string>
#include <fstream>
#include "../visualisation/multipass/FrameBuffer.h"
#include "../visualisation/Visualisation.h"


TumourScene::SceneContent::SceneContent(std::shared_ptr<LightsBuffer> lights, const fs::path &tumourDataDirectory, std::shared_ptr<const NoClipCamera> camera)
    : lights(lights)
	, nbModel(new Entity(Stock::Models::ICOSPHERE, 10.0f, { std::make_shared<Shaders>("../sdl_exp/primage/instanced.vert", "../sdl_exp/primage/tumourcell_flat.frag") }))
    , scModel(new Entity(Stock::Models::ICOSPHERE, 10.0f, { std::make_shared<Shaders>("../sdl_exp/primage/instanced_white.vert", "../sdl_exp/primage/tumourcell_flat.frag") }))
	, cellModel(std::make_shared<CellBillboard>(camera))
	, cellIndex(0)
    , nb_instancedRenderOffset(0)
    , sc_instancedRenderOffset(0)
	, blur(new GaussianBlur(5, 1.75f))
	, depthIn()
	, depthOut(Texture2D::make(glm::uvec2(1280, 720), { GL_RED, GL_R32F, sizeof(float), GL_FLOAT }, nullptr, Texture::FILTER_MIN_LINEAR_MIPMAP_LINEAR | Texture::FILTER_MAG_LINEAR | Texture::WRAP_CLAMP_TO_EDGE))
{
    loadCells(tumourDataDirectory);
}
void TumourScene::SceneContent::loadCells(const fs::path &tumourDataDirectory)
{
    const fs::path &nbDataDirectory = tumourDataDirectory / fs::path("nb");
    const fs::path &scDataDirectory = tumourDataDirectory / fs::path("sc");
	const int EC_EVM_ELEMENTS = 0;
    int nb_totalCt = 0;
    int sc_totalCt = 0;
	size_t fileCt_max = 0;
    for (int i = 0; i < 139;++i)
    {
        // Count nb
		std::string tryFile = (nbDataDirectory / fs::path(std::to_string(i) + std::string(".bin"))).string();
		std::ifstream ifs;
		ifs.open(tryFile, std::ios::in | std::ios::binary | std::ios::ate);
		if (!ifs.is_open())
			break;
		size_t sz = ifs.tellg();
		ifs.close();
		sz -= (EC_EVM_ELEMENTS * sizeof(float));//Remove EC_EVM
		int fileCt = sz / (4 * sizeof(float));
		fileCt_max = fileCt > fileCt_max ? fileCt : fileCt_max;
		nb_cells.push_back({ nb_totalCt, fileCt });
        nb_totalCt += fileCt;
        //printf("step: %d, %d cells, %d total\n", i, fileCt, nb_totalCt);
        // Count sc
        tryFile = (scDataDirectory / fs::path(std::to_string(i) + std::string(".bin"))).string();
        ifs.open(tryFile, std::ios::in | std::ios::binary | std::ios::ate);
        if (!ifs.is_open())
            break;
        sz = ifs.tellg();
        ifs.close();
        sz -= (EC_EVM_ELEMENTS * sizeof(float));//Remove EC_EVM
        fileCt = sz / (4 * sizeof(float));
        fileCt_max = fileCt > fileCt_max ? fileCt : fileCt_max;
        sc_cells.push_back({ sc_totalCt, fileCt });
        sc_totalCt += fileCt;
    }
	//Allocate Texture Buffers
	nb_cellX = TextureBuffer<float>::make(nb_totalCt, 1);
    nb_cellY = TextureBuffer<float>::make(nb_totalCt, 1);
    nb_cellZ = TextureBuffer<float>::make(nb_totalCt, 1);
    nb_cellP53 = TextureBuffer<float>::make(nb_totalCt, 1);
    sc_cellX = TextureBuffer<float>::make(sc_totalCt, 1);
    sc_cellY = TextureBuffer<float>::make(sc_totalCt, 1);
    sc_cellZ = TextureBuffer<float>::make(sc_totalCt, 1);
    sc_cellP53 = TextureBuffer<float>::make(sc_totalCt, 1);
	char *t_buffer = (char*)malloc(fileCt_max * sizeof(float));
	for (int i = 0; i < nb_cells.size(); ++i)
	{
        // Load NB file for timestep
		std::string tryFile = (nbDataDirectory / fs::path(std::to_string(i) + std::string(".bin"))).string();
		std::ifstream ifs;
		ifs.open(tryFile, std::ios::in | std::ios::binary);
		if (ifs.is_open())
		{
			ifs.read(t_buffer, EC_EVM_ELEMENTS * sizeof(float));
			ifs.read(t_buffer, nb_cells[i].count * sizeof(float));
			nb_cellX->setData((float*)t_buffer, nb_cells[i].count * sizeof(float), nb_cells[i].offset * sizeof(float));
			ifs.read(t_buffer, nb_cells[i].count * sizeof(float));
            nb_cellY->setData((float*)t_buffer, nb_cells[i].count * sizeof(float), nb_cells[i].offset * sizeof(float));
			ifs.read(t_buffer, nb_cells[i].count * sizeof(float));
            nb_cellZ->setData((float*)t_buffer, nb_cells[i].count * sizeof(float), nb_cells[i].offset * sizeof(float));
			ifs.read(t_buffer, nb_cells[i].count * sizeof(float));
            //for (int i = 0; i < fileCt_max; ++i)
            //    reinterpret_cast<float*>(t_buffer)[i] = i;// reinterpret_cast<int*>(t_buffer)[i];
            nb_cellP53->setData((float*)t_buffer, nb_cells[i].count * sizeof(float), nb_cells[i].offset * sizeof(float));
            printf("%d, %d, %d\n", reinterpret_cast<int*>(t_buffer)[0], reinterpret_cast<int*>(t_buffer)[1000], reinterpret_cast<int*>(t_buffer)[2000]);
			assert(ifs.good());
			ifs.close();
		}
        // Load SC file for timestep
        tryFile = (scDataDirectory / fs::path(std::to_string(i) + std::string(".bin"))).string();
        ifs.open(tryFile, std::ios::in | std::ios::binary);
        if (ifs.is_open())
        {
            ifs.read(t_buffer, EC_EVM_ELEMENTS * sizeof(float));
            ifs.read(t_buffer, sc_cells[i].count * sizeof(float));
            sc_cellX->setData((float*)t_buffer, sc_cells[i].count * sizeof(float), sc_cells[i].offset * sizeof(float));
            ifs.read(t_buffer, sc_cells[i].count * sizeof(float));
            sc_cellY->setData((float*)t_buffer, sc_cells[i].count * sizeof(float), sc_cells[i].offset * sizeof(float));
            ifs.read(t_buffer, sc_cells[i].count * sizeof(float));
            sc_cellZ->setData((float*)t_buffer, sc_cells[i].count * sizeof(float), sc_cells[i].offset * sizeof(float));
            ifs.read(t_buffer, sc_cells[i].count * sizeof(float));
            sc_cellP53->setData((float*)t_buffer, sc_cells[i].count * sizeof(float), sc_cells[i].offset * sizeof(float));
            assert(ifs.good());
            ifs.close();
        }
	}
	free(t_buffer);
}

TumourScene::TumourScene(Visualisation &visualisation, const fs::path &tumourDataDirectory)
    : MultiPassScene(visualisation)
    , content(std::make_shared<SceneContent>(Lights(), tumourDataDirectory, std::dynamic_pointer_cast<const NoClipCamera>(visualisation.getCamera())))
	, spherePass(std::make_shared<SpherePass>(content, std::dynamic_pointer_cast<const NoClipCamera>(visualisation.getCamera())))
	, dPass(std::make_shared<DepthPass>(content))
	, fPass(std::make_shared<FinalPass>(content))
	, implictSurfaceActive(false)
{
	//Register models
    registerEntity(content->nbModel);
    registerEntity(content->scModel);
	registerEntity(content->cellModel);
    content->grid.setViewMatPtr(this->visualisation.getCamera()->getViewMatPtr());
    content->grid.setProjectionMatPtr(this->visualisation.getProjectionMatPtr());
    content->grid.setLightsBuffer(Lights()->getBufferBindPoint());
    for(auto &a:content->labels)
    {
        registerEntity(a.second);
    }
	//Register render passes in correct order
	addPass(0, dPass, false);
	addPass(3, fPass, false);
	//Put a preview of the depth texture on the HUD
	depthMapPreview = std::make_shared<Sprite2D>(content->depthOut, std::make_shared<Shaders>(Stock::Shaders::SPRITE2D_HEAT), glm::uvec2(256, 144));
	if (auto a = this->visualisation.getHUD().lock())
		a->add(depthMapPreview, HUD::AnchorV::South, HUD::AnchorH::East);
	depthMapPreview->setVisible(false);
	//Register boring vis pass
	addPass(2000, spherePass);
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

	auto sphere0 = content->nbModel->getShaders(0);
	sphere0->addTexture("_texBufX", content->nb_cellX);
	sphere0->addTexture("_texBufY", content->nb_cellY);
	sphere0->addTexture("_texBufZ", content->nb_cellZ);
	sphere0->addTexture("_texBufP53", content->nb_cellP53);
	sphere0->addDynamicUniform("instanceOffset", &content->nb_instancedRenderOffset);
    sphere0 = content->scModel->getShaders(0);
    sphere0->addTexture("_texBufX", content->sc_cellX);
    sphere0->addTexture("_texBufY", content->sc_cellY);
    sphere0->addTexture("_texBufZ", content->sc_cellZ);
    sphere0->addTexture("_texBufP53", content->sc_cellP53);
    sphere0->addDynamicUniform("instanceOffset", &content->sc_instancedRenderOffset);

	auto cell0 = content->cellModel->getShaders();
	cell0->addTexture("_texBufX", content->nb_cellX);
	cell0->addTexture("_texBufY", content->nb_cellY);
	cell0->addTexture("_texBufZ", content->nb_cellZ);
	cell0->addTexture("_texBufP53", content->nb_cellP53);
	cell0->addDynamicUniform("instanceOffset", &content->nb_instancedRenderOffset);


	frameCt = std::make_shared<Text>("", 20, glm::vec3(0.0f), Stock::Font::ARIAL);
	frameCt->setUseAA(true);
	ec_evm = std::make_shared<Text>("", 20, glm::vec3(0.0f), Stock::Font::LUCIDIA_CONSOLE);
	ec_evm->setUseAA(true);
	if (auto a = this->visualisation.getHUD().lock())
	{
		a->add(frameCt, HUD::AnchorV::South, HUD::AnchorH::East, glm::ivec2(0), INT_MAX);
		 //a->add(ec_evm, HUD::AnchorV::South, HUD::AnchorH::West, glm::ivec2(0, 10), INT_MAX);
	}
	setFrameCt();
}
void TumourScene::update(const unsigned int &frameTime)
{
	auto p = Lights()->getPointLight(0);
	p.Position(visualisation.getCamera()->getEye());
    content->camera_pos = visualisation.getCamera()->getEye();
	//auto d = Lights()->getDirectionalLight(0);
	//d.Direction(std::dynamic_pointer_cast<NoClipCamera const>(visualisation.getCamera())->getLook());
	if (autostep) {
        const int SPEED_ms = 200;
        static unsigned int ft = 0;
        if (content->cellIndex < content->nb_cells.size() - 1) {

            ft += frameTime;
            if (ft > SPEED_ms) {
                content->cellIndex++;
                ft -= SPEED_ms;
                setFrameCt();
            }
        } else {
            autostep = false;
        }
	}
}
bool TumourScene::keypress(SDL_Keycode keycode, int x, int y)
{
	switch (keycode)
	{
	case SDLK_1:
		content->cellIndex--;
		break;
	case SDLK_2:
		content->cellIndex++;
		break;
	case SDLK_3:
		content->cellIndex -= 10;
		break;
	case SDLK_4:
		content->cellIndex += 10;
		break;
	case SDLK_5:
		content->cellIndex -= 100;
		break;
	case SDLK_6:
		content->cellIndex += 100;
		break;
	case SDLK_9:
		content->cellIndex = 0;
		break;
	case SDLK_0:
		content->cellIndex  = content->nb_cells.size()-1;
		break;
	case SDLK_p:
		implictSurfaceActive = !implictSurfaceActive;
		toggleImplicitSurface();
		break;
    case SDLK_o:
        autostep = !autostep;
        break;
    case SDLK_c:
        {
            Visualisation *v = dynamic_cast<Visualisation*>(&visualisation);
            v->Camera()->setEye(glm::vec3(140, 70, 140));
            v->Camera()->lookAt(glm::vec3(0, -20, 0));
            v->Camera()->move(-300.0f);
        }
        break;
	default:
		//Permit the keycode to be processed if we haven't handled personally
		return true;
	}
	content->cellIndex = content->cellIndex < 0 ? 0 : content->cellIndex;
	content->cellIndex = content->cellIndex >= content->nb_cells.size() ? content->nb_cells.size()-1 : content->cellIndex;
	setFrameCt();
	return false;
}
void TumourScene::setFrameCt()
{
	this->frameCt->setString("Time (hours): %d\nNB count: %d\nSC count: %d", content->cellIndex, content->nb_cells[content->cellIndex].count, content->sc_cells[content->cellIndex].count);
	//this->ec_evm->setString("Core o2: %3d%%\n  Rim o2: %3d%%\n  Mobility Detatch: %3d%%\n   Mobility Degrade: %3d%%\n  Mobility EMT: %3d%%\n  Mobility Vascular: %3d%%", 
	//	(int)(content->cells[content->cellIndex].oxygen_core *100),
	//	(int)(content->cells[content->cellIndex].oxygen_rim * 100),
	//	(int)(content->cells[content->cellIndex].mobility_detatch * 100),
	//	(int)(content->cells[content->cellIndex].mobility_degrade * 100),
	//	(int)(content->cells[content->cellIndex].mobility_EMT * 100),
	//	(int)(content->cells[content->cellIndex].mobility_vascular * 100)
	//);
}
void TumourScene::reload()
{
	//content->blur->reload();
}
TumourScene::DepthPass::DepthPass(std::shared_ptr<SceneContent> content)
	: RenderPass(std::make_shared<FrameBuffer>(FBAFactory::ManagedColorTexture(GL_R32F, GL_RED, GL_FLOAT), FBAFactory::ManagedDepthRenderBuffer(), FBAFactory::Disabled(), 0, 1.0f, true, glm::vec3(1.0f)))
	, content(content)

{
	//Pass the shadow texture to the second shader of each model
	std::shared_ptr<FrameBuffer> t = std::dynamic_pointer_cast<FrameBuffer>(getFrameBuffer());
	if (t)
	{
		content->depthIn = std::dynamic_pointer_cast<Texture2D>(t->getColorTexture());
		//content->depthOut = content->depthIn;//Uses the pre blur shadow map (aka hard shadows)
	}
	//content->sphereModel->getShaders(1)->addTexture("_shadowMap", content->depthOut);
}
TumourScene::FinalPass::FinalPass(std::shared_ptr<SceneContent> content)
	: RenderPass(std::make_shared<BackBuffer>())
	, content(content)

{
}
void TumourScene::DepthPass::render()
{
	//content->sphereModel->renderInstances(content->cells[0].count, 0);
	content->nb_instancedRenderOffset = content->nb_cells[content->cellIndex].offset;
	content->cellModel->renderInstances(content->nb_cells[content->cellIndex].count);
}
void TumourScene::FinalPass::render()
{
	//Slightly blur shadow map to reduce harshness of edges
	content->blur->blurR32F(content->depthIn, content->depthOut);
	//Generate mip-map
	content->depthOut->updateMipMap();
	//Render models using shadow map
    content->nb_instancedRenderOffset = content->nb_cells[content->cellIndex].offset;
	content->cellModel->renderInstances(content->nb_cells[content->cellIndex].count);

}
TumourScene::SpherePass::SpherePass(std::shared_ptr<SceneContent> content, std::shared_ptr<const NoClipCamera> camera)
	: RenderPass(std::make_shared<BackBuffer>(true, glm::vec3(1)))
	, content(content)

{
    GL_CALL(glEnable(GL_LINE_SMOOTH));
    float dim = 15; // Cell width
    glm::vec3 envMin = glm::vec3(-45);
    glm::vec3 envMax = glm::vec3(45);
    // Construct grid (dimensions hardcoded)
    {
        content->grid.color(glm::vec3(0));
        content->grid.width(1.5f);
        content->grid.begin(Draw::Lines, "x+");
        for (float i = envMin.y; i <= envMax.y; i += dim)
        {
            content->grid.vertex(glm::vec3(envMax.x, i, envMin.z));
            content->grid.vertex(glm::vec3(envMax.x, i, envMax.z));
        }
        for (float i = envMin.z; i <= envMax.z; i += dim)
        {
            content->grid.vertex(glm::vec3(envMax.x, envMin.y, i));
            content->grid.vertex(glm::vec3(envMax.x, envMax.y, i));
        }
        content->grid.save();
        content->grid.begin(Draw::Lines, "x-");
        for (float i = envMin.y; i <= envMax.y; i += dim)
        {
            content->grid.vertex(glm::vec3(envMin.x, i, envMin.z));
            content->grid.vertex(glm::vec3(envMin.x, i, envMax.z));
        }
        for (float i = envMin.z; i <= envMax.z; i += dim)
        {
            content->grid.vertex(glm::vec3(envMin.x, envMin.y, i));
            content->grid.vertex(glm::vec3(envMin.x, envMax.y, i));
        }
        content->grid.save();
        content->grid.begin(Draw::Lines, "y+");
        for (float i = envMin.x; i <= envMax.x; i += dim)
        {
            content->grid.vertex(glm::vec3(i, envMax.y, envMin.z));
            content->grid.vertex(glm::vec3(i, envMax.y, envMax.z));
        }
        for (float i = envMin.z; i <= envMax.z; i += dim)
        {
            content->grid.vertex(glm::vec3(envMin.x, envMax.y, i));
            content->grid.vertex(glm::vec3(envMax.x, envMax.y, i));
        }
        content->grid.save();
        content->grid.begin(Draw::Lines, "y-");
        for (float i = envMin.x; i <= envMax.x; i += dim)
        {
            content->grid.vertex(glm::vec3(i, envMin.y, envMin.z));
            content->grid.vertex(glm::vec3(i, envMin.y, envMax.z));
        }
        for (float i = envMin.z; i <= envMax.z; i += dim)
        {
            content->grid.vertex(glm::vec3(envMin.x, envMin.y, i));
            content->grid.vertex(glm::vec3(envMax.x, envMin.y, i));
        }
        content->grid.save();
        content->grid.begin(Draw::Lines, "z+");
        for (float i = envMin.x; i <= envMax.x; i += dim)
        {
            content->grid.vertex(glm::vec3(i, envMin.y, envMax.z));
            content->grid.vertex(glm::vec3(i, envMax.y, envMax.z));
        }
        for (float i = envMin.y; i <= envMax.y; i += dim)
        {
            content->grid.vertex(glm::vec3(envMin.x, i, envMax.z));
            content->grid.vertex(glm::vec3(envMax.x, i, envMax.z));
        }
        content->grid.save();
        content->grid.begin(Draw::Lines, "z-");
        for (float i = envMin.y; i <= envMax.z; i += dim)
        {
            content->grid.vertex(glm::vec3(i, envMin.y, envMin.z));
            content->grid.vertex(glm::vec3(i, envMax.y, envMin.z));
        }
        for (float i = envMin.z; i <= envMax.z; i += dim)
        {
            content->grid.vertex(glm::vec3(envMin.x, i, envMin.z));
            content->grid.vertex(glm::vec3(envMax.x, i, envMin.z));
        }
        content->grid.save();
    }

    // Prep labels
    {
        for (float i = envMin.x;i<=envMax.x;i+=dim)
        {
            auto a = std::make_shared<TextBillboard>(camera, std::to_string((int)i).c_str());
            //a->getShaders()->
            content->labels.emplace((int)i, a);
        }
    }
}
//Uses the shadow map to render the normal scene
void TumourScene::SpherePass::render()
{
    GL_CALL(glEnable(GL_BLEND));
    //GL_CALL(glDisable(GL_CULL_FACE));
	//Slightly blur shadow map to reduce harshness of edges
	//content->blur->blurR32F(content->shadowIn, content->shadowOut);
	//Generate mip-map
	//content->shadowOut->updateMipMap();
	//Render models using shadow map
    renderGrid();
	content->nb_instancedRenderOffset = content->nb_cells[content->cellIndex].offset;
	content->nbModel->renderInstances(content->nb_cells[content->cellIndex].count, 0);
    content->sc_instancedRenderOffset = content->sc_cells[content->cellIndex].offset;
    content->scModel->renderInstances(content->sc_cells[content->cellIndex].count, 0);
	//content->sphereModel->renderInstances(1000, 0);
	//Render something at the lights location
	//content->lights->render();
    //glEnable(GL_CULL_FACE);
    //GL_CALL(glDisable(GL_BLEND));
}
void TumourScene::toggleImplicitSurface()
{
	setPassActive(spherePass, !implictSurfaceActive);
	setPassActive(fPass, implictSurfaceActive);
	setPassActive(dPass, implictSurfaceActive);
	depthMapPreview->setVisible(implictSurfaceActive);
}

void TumourScene::SpherePass::renderGrid()
{
    if(content->camera_pos.x>0)
        content->grid.render("x-");
    if (content->camera_pos.x<0)
        content->grid.render("x+");
    if (content->camera_pos.y>0)
        content->grid.render("y-");
    if (content->camera_pos.y<0)
        content->grid.render("y+");
    if (content->camera_pos.z>0)
        content->grid.render("z-");
    if (content->camera_pos.z<0)
        content->grid.render("z+");
    //Labels
    //glDisable(GL_DEPTH_TEST);
    //float dim = 20; // Cell width
    //glm::vec3 envMin = glm::vec3(-60);
    //glm::vec3 envMax = glm::vec3(60);
    //for (float i = envMin.x; i <= envMax.x; i += dim)
    //{
    //    auto a = content->labels.at((int)i);
    //    a->render(glm::vec3(envMin.x, envMin.y, i));
    //}
    //glEnable(GL_DEPTH_TEST);
}