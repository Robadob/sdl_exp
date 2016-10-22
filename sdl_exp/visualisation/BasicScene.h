#ifndef __BasicScene_h__
#define __BasicScene_h__
#include "Scene.h"
#include "multipass/RenderPass.h"
#include "Skybox.h"

// ReSharper disable CppHidingFunction
class BasicScene : protected Scene, private RenderPass
{
    BasicScene(Visualisation& vis)
        : Scene(vis)
        , RenderPass()
        , renderAxisState(false)
        , axis(25)
        , skybox(0)
    {  }
    void executeRender()override final
    {
        render();
    }
    virtual void render() override final
    {
        if (this->skybox)
            this->skybox->render();
        this->visualisation.defaultProjection();
        if (this->renderAxisState)
            this->axis.render();
        this->defaultLighting();
        _render();
    }
    virtual void _render();
    void resize(const int width, const int height)override final
    {
        RenderPass::resize(width, height);
    }
    bool keypress(SDL_Keycode keycode, int x, int y) override final{
        //Pass key events to the scene and skip handling if false is returned 
        if (!_keypress(keycode, x, y))
            return;
        switch (keycode){
        case SDLK_F9:
            this->setSkybox(!this->skybox);
            break;
        default:
            return true;
            break;
        }
        return false;
    }
    void _reload() override final
    {
        printf("Reloading Shaders.\n");
        for (std::vector<std::shared_ptr<Entity>>::iterator i = entities.begin(); i != entities.end(); i++)
        {
            (*i)->reload();
        }
        reload();
    }
    virtual void reload();
    virtual bool _keypress(SDL_Keycode keycode, int x, int y);

    //Default objects
    Axis axis;
    Skybox *skybox;


    bool renderAxisState;


    void defaultLighting();

    void setSkybox(bool state);

    void setRenderAxis(bool state);
};
// ReSharper restore CppHidingFunction
#endif //__BasicScene_h__