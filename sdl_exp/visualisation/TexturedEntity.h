#include "Entity.h"
#include "Texture.h"

class TexturedEntity : public Entity
{
public:    
    TexturedEntity(Stock::Models::Model model, float scale, std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr));
    TexturedEntity(const char *modelPath, const char * textureFile, float modelScale = 1.0f, std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr));
    void render();
    void renderInstances(int count);
private:
    Texture tex;
};
