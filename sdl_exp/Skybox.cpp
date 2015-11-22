#include "Skybox.h" 
#include "glm/gtc/type_ptr.hpp"
GLubyte checkImage[TEXTURE_SIZE][TEXTURE_SIZE][4]; 
float points[] = {
    -10.0f, 10.0f, -10.0f,
    -10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    10.0f, 10.0f, -10.0f,
    -10.0f, 10.0f, -10.0f,

    -10.0f, -10.0f, 10.0f,
    -10.0f, -10.0f, -10.0f,
    -10.0f, 10.0f, -10.0f,
    -10.0f, 10.0f, -10.0f,
    -10.0f, 10.0f, 10.0f,
    -10.0f, -10.0f, 10.0f,

    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,

    -10.0f, -10.0f, 10.0f,
    -10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, -10.0f, 10.0f,
    -10.0f, -10.0f, 10.0f,

    -10.0f, 10.0f, -10.0f,
    10.0f, 10.0f, -10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    -10.0f, 10.0f, 10.0f,
    -10.0f, 10.0f, -10.0f,

    -10.0f, -10.0f, -10.0f,
    -10.0f, -10.0f, 10.0f,
    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    -10.0f, -10.0f, 10.0f,
    10.0f, -10.0f, 10.0f
};
Skybox::Skybox(char* texturePath)
    : shaders("../shaders/skybox.v","../shaders/skybox.f")
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    //Generate a temporary image (http://www.glprogramming.com/red/chapter09.html)

    for (int i = 0; i < TEXTURE_SIZE; i++) {
        for (int j = 0; j < TEXTURE_SIZE; j++) {
            int c = ((i%2 == 0) ^ (j%2 == 0)) * 255;
            checkImage[i][j][0] = (GLubyte)c;
            checkImage[i][j][1] = (GLubyte)c;
            checkImage[i][j][2] = (GLubyte)c;
            checkImage[i][j][3] = (GLubyte)255;
        }
    }
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER);//If this buffer is bound TexImage2D will take texture from the buffer instead
    //Init the texture
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texName);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    //Init the cube
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &points, GL_STATIC_DRAW); 
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}
Skybox::~Skybox()
{
    //Dealloc the vertices
    //Deinit the texture
}
void Skybox::reload()
{
    shaders.reloadShaders();
}
void Skybox::render(Camera *camera, glm::mat4 projection)
{
    glPushMatrix();
    //Setup shaders
    shaders.useProgram();
    shaders.setUniformMatrix4fv(1, &camera->skyboxView()[0][0]);
    shaders.setUniformMatrix4fv(2, &projection[0][0]);
    shaders.setUniformi(3, 0);
    
    // Reset and transform the matrix.
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //glLoadMatrixf(glm::value_ptr(projection));
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //glLoadMatrixf(glm::value_ptr(camera->skyboxView()));

    // Enable/Disable features
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP); 
    //glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glDepthMask(GL_FALSE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texName); 
    //glBindSampler(0, linearFiltering);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    glDisableVertexAttribArray(0);
   
    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();
    shaders.clearProgram();
}