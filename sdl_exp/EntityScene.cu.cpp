#include "EntityScene.h"
#include <glm/gtc/type_ptr.hpp>

/*
Constructor, modify this to change what happens
*/
EntityScene::EntityScene(Visualisation &visualisation)
    : Scene(visualisation)
    , deerModel(new Entity(Stock::Models::DEER, 10.0f, Stock::Shaders::TEXTURE))
    , colorModel(new Entity(Stock::Models::ROTHWELL, 45.0f, Stock::Shaders::COLOR))
    , tick(0.0f)
    , polarity(-1)
    , instancedSphere(new Entity(Stock::Models::ICOSPHERE, 1.0f, Stock::Shaders::INSTANCED))
#ifdef __CUDACC__
    , cuTexBuf(mallocGLInteropTextureBuffer<float>(100, 3))
    , texBuf("_texBuf", cuTexBuf, true)
#else
    , texBuf("_texBuf", 100, 3)
    , billboardShaders(new Shaders(Stock::Shaders::BILLBOARD))
#endif
{
    registerEntity(deerModel);
    registerEntity(colorModel);
    registerEntity(instancedSphere);
    this->visualisation.setSkybox(true);
    this->visualisation.setWindowTitle("Entity Render Sample");
    this->visualisation.setRenderAxis(true); 
    srand((unsigned int)time(0));
    this->colorModel->setRotation(glm::vec4(1.0, 0.0, 0.0, -90));
    this->colorModel->setCullFace(false);
    this->deerModel->flipVertexOrder();
#ifdef __CUDACC__
    cuInit();
#else
    float *tempData = (float*)malloc(sizeof(float) * 3 * 100);
    for (int i = 0; i < 100;i++)
    {
        tempData[(i * 3) + 0] = 100 * (float)sin(i*3.6);
        tempData[(i * 3) + 1] = -50.0f;
        tempData[(i * 3) + 2] = 100 * (float)cos(i*3.6);
    }
    texBuf.setData(tempData);
    free(tempData);
#endif
    texBuf.bindToShader(this->instancedSphere->getShaders().get());
    this->instancedSphere->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
    initParticles();
}
/*
Called once per frame when Scene animation calls should be 
@param Milliseconds since last time this method was called
*/
void EntityScene::update(unsigned int frameTime)
{
    this->tick += this->polarity*((frameTime*60)/1000.0f)*0.01f;
    this->tick = (float)fmod(this->tick,360);
    this->deerModel->setRotation(glm::vec4(0.0, 1.0, 0.0, this->tick*-100));
    this->deerModel->setLocation(glm::vec3(50 * sin(this->tick), 0, 50 * cos(this->tick)));
#ifdef __CUDACC__
    cuUpdate();
#endif
}
/*
Called once per frame when Scene render calls should be executed
*/
void EntityScene::render()
{
    //colorModel->render();
    //deerModel->render();
    //this->instancedSphere->renderInstances(100);
    renderParticles();
}
/*
Called when the user requests a reload
*/
void EntityScene::reload()
{
    this->instancedSphere->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
    this->billboardShaders->reload(true);
}

bool EntityScene::keypress(SDL_Keycode keycode, int x, int y)
{
    switch (keycode)
    {
    case SDLK_p:
        this->polarity = ++this->polarity>1 ? -1 : this->polarity;
        break;
    case SDLK_HASH:
        this->colorModel->exportModel();
        this->deerModel->exportModel();
    default:
        //Only permit the keycode to be processed if we haven't handled personally
        return true;
    }
    return false;
}
void EntityScene::initParticles()
{
    this->billboardShaders->setModelViewMatPtr(this->visualisation.getCamera()->getViewMatPtr());//&particleTransform);// 
    this->billboardShaders->setProjectionMatPtr(this->visualisation.getFrustrumPtr());
    //Init vbo's
    unsigned int bufferSize = 0;
    bufferSize += 4 * sizeof(glm::vec3); //4 points to a quad
    bufferSize += 4 * sizeof(glm::vec2);//4 points to a tex coord
    particleData = malloc(bufferSize);
    //Setup tex coords
    glm::vec2 *texCoords = static_cast<glm::vec2*>(static_cast<void*>(static_cast<glm::vec3*>(particleData)+4));
    texCoords[0] = glm::vec2(0.0f, 1.0f); //TopLeft
    texCoords[1] = glm::vec2(0.0f, 0.0f); //BottomLeft
    texCoords[2] = glm::vec2(1.0f, 1.0f); //TopRight
    texCoords[3] = glm::vec2(1.0f, 0.0f); //BottomRight
    //Initalise buffer
    GL_CALL(glGenBuffers(1, &vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, bufferSize, particleData, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Setup vertices
    glm::vec3 *topLeft = static_cast<glm::vec3*>(particleData);
    glm::vec3 *bottomLeft = topLeft + 1;
    glm::vec3 *topRight = bottomLeft + 1;
    glm::vec3 *bottomRight = topRight + 1;
    const float size = 1;
    *topLeft = glm::vec3(-size, -size, 0);
    *bottomLeft = glm::vec3(size, -size, 0);
    *topRight = glm::vec3(-size, size, 0);
    *bottomRight = glm::vec3(size, size, 0);
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::vec3), particleData));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Link Vertex Attributes
    Shaders::VertexAttributeDetail pos(GL_FLOAT, 3, sizeof(float));
    pos.vbo = vbo;
    pos.count = 4;
    pos.data = particleData;
    pos.offset = 0;
    pos.stride = 0;
    this->billboardShaders->setPositionsAttributeDetail(pos);
    Shaders::VertexAttributeDetail texCo(GL_FLOAT, 2, sizeof(float));
    texCo.vbo = vbo;
    texCo.count = 4;
    texCo.data = texCoords;
    texCo.offset = 4 * sizeof(glm::vec3);
    texCo.stride = 0;
    this->billboardShaders->setTexCoordsAttributeDetail(texCo);
    //Setup faces
    GL_CALL(glGenBuffers(1, &fvbo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
    const int faces[] = { 0, 1, 2, 3 };
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(int), &faces, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    //Init uniforms
    this->particleTick = 0;
    this->billboardShaders->addDynamicUniform("_tick", &this->particleTick);
}
void EntityScene::renderParticles()
{
    glm::vec3 t = this->visualisation.getCamera()->getUp();
    this->billboardShaders->addStaticUniform("_up", value_ptr(t), 3);
    glm::vec3 r = this->visualisation.getCamera()->getRight();
    this->billboardShaders->addStaticUniform("_right", value_ptr(r), 3);

    GL_CALL(glEnable(GL_BLEND));
    //Use Shader
    billboardShaders->useProgram();
    //Render quad
    glDisable(GL_CULL_FACE);
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
    glPushMatrix();
    GL_CALL(glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0));
    glPopMatrix();

    //Unload shader
    glEnable(GL_CULL_FACE);
    billboardShaders->clearProgram();

    GL_CALL(glDisable(GL_BLEND));
    this->particleTick+=0.02f;
}