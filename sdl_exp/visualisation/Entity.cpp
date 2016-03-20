#define  _CRT_SECURE_NO_WARNINGS
#include "Entity.h"

#include <unordered_map>
#include <functional>
#include <thread>
#include <glm/gtx/component_wise.hpp>

#define DEFAULT_TEXCOORD_SIZE 2
#define FACES_SIZE 3
#define VN_PAIR std::tuple<unsigned int, unsigned int, unsigned int>

/*
Convenience constructor.
*/
template<class T>
Entity::Entity(
    Stock::Models::Model const model,
    float modelScale,
    std::shared_ptr<Shaders> shaders,
    std::shared_ptr<T> texture
    ) : Entity(
    model.modelPath,
    modelScale,
    shaders.get() ? shaders : std::make_shared<Shaders>(model.defaultShaders.vertex, model.defaultShaders.fragment, model.defaultShaders.geometry),
    texture.get() ? texture : std::make_shared<Texture2D>(model.texturePath)
) { }
template<class T>
Entity::Entity(
    Stock::Models::Model const model,
    float modelScale,
    Stock::Shaders::ShaderSet const ss,
    std::shared_ptr<T> texture
    ) : Entity(
    model.modelPath,
    modelScale,
    shaders.get() ? shaders : std::make_shared<Shaders>(model.defaultShaders.vertex, model.defaultShaders.fragment, model.defaultShaders.geometry),
    texture.get() ? texture : std::make_shared<Texture2D>(model.texturePath)
) { }
template<class T>
Entity::Entity(
    const char *modelPath,
    float modelScale,
    Stock::Shaders::ShaderSet const ss,
    std::shared_ptr<T> texture
    ): Entity(
    modelPath,
    modelScale,
    std::make_shared<Shaders>(ss.vertex, ss.fragment, ss.geometry),
    texture
    ) { }
/*
Constructs an entity from the provided .obj model
@param modelPath Path to .obj format model file
@param modelScale World size to scale the longest direction (in the x, y or z) axis of the model to fit
*/
template<class T>
Entity::Entity(
    const char *modelPath, 
    float modelScale, 
    std::shared_ptr<Shaders> shaders, 
    std::shared_ptr<T> texture
    )
    : positions(GL_FLOAT, 3, sizeof(float))
    , normals(GL_FLOAT, NORMALS_SIZE, sizeof(float))
    , colors(GL_FLOAT, 3, sizeof(float))
    , texcoords(GL_FLOAT, 2, sizeof(float))
    , faces(GL_UNSIGNED_INT, FACES_SIZE, sizeof(unsigned int))
    , vn_count(0)
    , SCALE(modelScale)
    , modelPath(modelPath)
    , material(0)
    , color(1, 0, 0)
    , location(0.0f)
    , rotation(0.0f, 0.0f, 1.0f, 0.0f)
    , shaders(shaders)
    , texture(texture)
{
    GL_CHECK();
    loadModelFromFile();
    //If texture has been provied, set up
    if (texture.get())
    {
        texture->bindToShader(shaders.get());
    }
    //If shaders have been provided, set them up
    if (positions.data&&this->shaders != nullptr)
    {
        this->shaders->setPositionsAttributeDetail(positions);
        this->shaders->setNormalsAttributeDetail(normals);
        this->shaders->setColorsAttributeDetail(colors);
        this->shaders->setTexCoordsAttributeDetail(texcoords);
    }
}
//Don't repeat the non template items when Entity-impl.cpp is compiled
#ifndef __Entity_impl_cpp__

const char *Entity::OBJ_TYPE = ".obj";
const char *Entity::EXPORT_TYPE = ".obj.sdl_export";

/*
Destructor, free's memory allocated to store the model and its material
*/
Entity::~Entity(){
    //All attribs (except faces) share the same vbo, so delete once
    deleteVertexBufferObject(&positions.vbo);
    deleteVertexBufferObject(&faces.vbo);
    //All attribs (except faces) share the same malloc, so delete once
    free(positions.data);
    free(faces.data);
    freeMaterial();
}
/*
Calls the necessary code to render a single instance of the entity
@param vertLocation The shader attribute location to pass vertices
@param normalLocation The shader attribute location to pass normals
*/
void Entity::render(){
    if (this->shaders != nullptr)
        shaders->useProgram(this);
    //Bind the faces to be rendered
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces.vbo));

    glPushMatrix();
    //Translate the model according to it's location
    if (this->material)
        this->material->useMaterial();
    GL_CALL(glColor4f(color.x, color.y, color.z, 1.0));
    GL_CALL(glDrawElements(GL_TRIANGLES, faces.count * faces.components, GL_UNSIGNED_INT, 0));
    glPopMatrix();
    if (this->shaders != nullptr)
        shaders->clearProgram();
}
/*
Calls the necessary code to render count instances of the entity
The index of the instance being rendered can be identified within the vertex shader with the variable gl_InstanceID
@param count The number of instances of the entity to render
@param vertLocation The shader attribute location to pass vertices
@param normalLocation The shader attribute location to pass normals
*/
void Entity::renderInstances(int count){
    if (this->shaders != nullptr)
        shaders->useProgram(this);
    //Bind the faces to be rendered
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces.vbo));

    glPushMatrix();
    //Set the color and material
    if (this->material)
        this->material->useMaterial();
    GL_CALL(glColor4f(color.x, color.y, color.z, 1.0));
    GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, faces.count * faces.components, GL_UNSIGNED_INT, 0, count));
    glPopMatrix();
    if (this->shaders != nullptr)
        shaders->clearProgram();
}
/*
Creates a vertex buffer object of the specified size
@param vbo The pointer to store the buffer objects location in
@param target The type of buffer to bind the buffer object (e.g. GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER)
@param size The size of the buffer in bytes
*/
void Entity::createVertexBufferObject(GLuint *vbo, GLenum target, GLuint size, void *data){
    GL_CALL(glGenBuffers(1, vbo));
    GL_CALL(glBindBuffer(target, *vbo));
    GL_CALL(glBufferData(target, size, data, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(target, 0));
}
/*
Deallocates the specified vertex buffer object
@param vbo A pointer to the buffer objects location
*/
void Entity::deleteVertexBufferObject(GLuint *vbo){
    GL_CALL(glDeleteBuffers(1, vbo));
}
/*
Used by loadModelFromFile() in a hashmap of vertex-normal pairs
*/
unsigned long hashing_func(VN_PAIR key)
{
    static int offset = sizeof(unsigned long) / 3;
    return (std::get<2>(key) << offset * 2) & (std::get<1>(key) << offset) & std::get<0>(key);
}
/*
Used by loadModelFromFile() in a hashmap of vertex-normal pairs
*/
bool key_equal_fn(VN_PAIR t1, VN_PAIR t2)
{
    return std::get<0>(t1) == std::get<0>(t2) &&
        std::get<1>(t1) == std::get<1>(t2) &&
        std::get<2>(t1) == std::get<2>(t2);
}
/*
Loads and scales the specified model into this classes primitive storage

This method support most mutations of .obj files;
Vertices: 3-4 components
Colors: 3-4 components
Normals: 3 components
Textures: 2-3 components (the optional 3rd component is wrapped in [], and is expected to be 1.0)
Faces: 3 components per, each indexing a vertex, and optionally a normal, or a normal and a texture.
The attributes that support variable length chars are designed according to the wikipedia spec
*/
void Entity::loadModelFromFile()
{
    FILE* file;
    //Redirect pre-exported models, and cancel if not .obj
    if (endsWith(modelPath, OBJ_TYPE))
    {
        std::string exportPath(modelPath);
        std::string objPath(OBJ_TYPE);
        exportPath = exportPath.substr(0, exportPath.length() - objPath.length()).append(EXPORT_TYPE);
        if (endsWith(modelPath, EXPORT_TYPE))
        {
            importModel(modelPath);
            return;
        }
        else if ((file = fopen(exportPath.c_str(), "r")))
        {
            fclose(file);
            importModel(exportPath.c_str());
            return;
        }
    }
    else
    {
        fprintf(stderr, "Model file '%s' is of an unsupported format, aborting load.\n Support types: %s, %s\n", modelPath, OBJ_TYPE, EXPORT_TYPE);
        return;
    }
    printf("\r Loading Model: %s", modelPath);

    //Open file
    file = fopen(modelPath, "r");
    if (!file){
        printf("\rLoading Model: Could not open model '%s'!\n", modelPath);
        return;
    }

    //Counters
    unsigned int positions_read = 0;
    unsigned int normals_read = 0;
    unsigned int colors_read = 0;
    unsigned int texcoords_read = 0;
    unsigned int faces_read = 0;
    unsigned int parameters_read = 0;

    unsigned int position_components_count = 3;
    unsigned int color_components_count = 0;
    unsigned int texcoords_components_count = 2;

    bool face_hasNormals = false;
    bool face_hasTexcoords = false;

    printf("\rLoading Model: %s [Counting Elements]", modelPath);
    //Count vertices/faces, attributes
    char c;
    int dotCtr;
    unsigned int lnLen = 0, lnLenMax = 0;//Used to find the longest line of the file
    //For each line of the file (the end of the loop finds the end of the line before continuing)
    while ((c = fgetc(file)) != EOF) {
        lnLenMax = lnLenMax < lnLen ? lnLen : lnLenMax;
        lnLen = 1;
        //If the first char == 'v'
        switch (c)
        {
        case 'v':
            if ((c = fgetc(file)) == EOF)
                goto exit_loop;
            //If the second char == 't', 'n', 'p' or ' '
            switch (c)
            {
                //Vertex found, increment count and check whether it also contains a colour value many elements it contains
            case ' ':
                positions_read++;
                dotCtr = 0;
                //Count the number of '.', if >4 we assume there are colours
                while ((c = fgetc(file)) != '\n')
                {
                    lnLen++;
                    if (c == EOF)
                        goto exit_loop;
                    else if (c == '.')
                        dotCtr++;
                }
                //Workout vertex and colour sizes
                switch (dotCtr)
                {
                case 8:
                    colors_read++;
                    color_components_count = 4;
                case 4:
                    position_components_count = 4;
                    break;
                case 7:
                    position_components_count = 4;
                case 6:
                    color_components_count = 3;
                    colors_read++;
                    break;
                }
                continue;//Skip to next iteration, otherwise we will miss a line
                //Normal found, increment count
            case 'n':
                normals_read++;
                break;
                //Parameter found, we don't support this but count anyway
            case 'p':
                parameters_read++;
                break;
                //Texture found, increment count and check how many components it contains
            case 't':
                texcoords_read++;
                dotCtr = 0;
                //Count the number of '.' before the next newline
                while ((c = fgetc(file)) != '\n')
                {
                    lnLen++;
                    if (c == EOF)
                        goto exit_loop;
                    else if (c == '.')
                        dotCtr++;
                }
                texcoords_components_count = dotCtr;
                continue;//Skip to next iteration, otherwise we will miss a line
            }
            break;
            //If the first char is 'f', increment face count
        case 'f':
            faces_read++;
            dotCtr = 0;
            //Workout whether the format is 'v' 'v//n' or 'v/t/n'
            while ((c = fgetc(file)) != '\n')
            {
                lnLen++;
                if (c == EOF){
                    goto exit_loop;
                }//If we find 1 slash, check whether we find 2 in a row
                else if (c == '/')
                {
                    face_hasNormals = true;
                    //If not two / in a row, then we have textures
                    if ((c = fgetc(file)) != '/')
                    {
                        face_hasTexcoords = true;
                        if (c == EOF)
                            goto exit_loop;
                    }
                    break;
                }
            }
            break;
        }
        //Speed to the end of the line and begin next iteration
        while (c != '\n')
        {
            lnLen++;
            if (c == EOF)
                goto exit_loop;
            c = fgetc(file);
        }
        //printf("\rVert: %i:%i, Normal: %i, Face: %i, Tex: %i:%i, Color: %i:%i, Ln:%i", vertices_read, vertices_size, normals_read, faces_read, textures_read, textures_size, colors_read, colors_size, lnLenMax);
    }
exit_loop:;
    lnLenMax = lnLenMax < lnLen ? lnLen : lnLenMax;

    if (parameters_read > 0){
        fprintf(stderr, "\nModel '%s' contains parameter space vertices, these are unsupported at this time.", modelPath);
        return;
    }

    //Set instance var counts
    positions.count = positions_read;
    colors.count = colors_read;
    normals.count = normals_read;
    texcoords.count = texcoords_read;
    faces.count = faces_read;
    if (positions.count == 0 || faces.count == 0)
    {
        fprintf(stderr, "\nVertex or face data missing.\nAre you sure that '%s' is a wavefront (.obj) format model?\n", modelPath);
        fclose(file);
        return;
    }
    if ((colors.count != 0 && positions.count != colors.count))
    {
        fprintf(stderr, "\nVertex color count does not match vertex count, vertex colors will be ignored.\n");
        colors.count = 0;
    }
    //Set instance var sizes
    normals.components = NORMALS_SIZE;
    faces.components = FACES_SIZE;
    positions.components = position_components_count;//3-4
    texcoords.components = texcoords_components_count;//2-3
    colors.components = color_components_count;//3-4
    //Allocate faces
    faces.data = malloc(faces.count*faces.components*faces.componentSize);
    //Reset file pointer
    clearerr(file);
    fseek(file, 0, SEEK_SET);
    //Allocate temporary buffers for components that may require aligning with relevant vertices
    float *t_vertices = (float *)malloc(positions.count * positions.components * positions.componentSize);
    float *t_colors = (float *)malloc(colors.count * colors.components * colors.componentSize);
    float *t_normals = (float *)malloc(normals.count * normals.components * normals.componentSize);
    float *t_texcoords = (float *)malloc(texcoords.count * texcoords.components*texcoords.componentSize);
    //3 parts to each face,store the relevant norm and tex indexes
    unsigned int *t_norm_pos = 0;
    if (face_hasNormals)
        t_norm_pos = (unsigned int *)malloc(faces.count*faces.components*faces.componentSize);
    else
        normals.count = 0;
    unsigned int *t_tex_pos = 0;
    if (face_hasTexcoords)
        t_tex_pos = (unsigned int *)malloc(faces.count*faces.components*faces.componentSize);
    else
        texcoords.count = 0;
    //Reset local counters
    positions_read = 0;
    colors_read = 0;
    normals_read = 0;
    texcoords_read = 0;
    faces_read = 0;
    unsigned int componentsRead = 0;
    unsigned int componentLength = 0;
    //Create buffer to read lines of the file into
    unsigned int bufferLen = lnLenMax + 2;
    char *buffer = new char[bufferLen];

    printf("\rLoading Model: %s [Loading Elements] ", modelPath);
    glm::vec3 modelMin(FLT_MAX);
    glm::vec3 modelMax(-FLT_MAX);
    //Read file by line, again.
    while ((c = fgetc(file)) != EOF) {
        //If the first char == 'v'
        switch (c)
        {
        case 'v':
            if ((c = fgetc(file)) == EOF)
                goto exit_loop;
            //If the second char == 't', 'n', 'p' or ' '
            switch (c)
            {
            case ' ':
                //Read vertex line of file
                componentsRead = 0;
                //Read all vertex components
                do
                {
                    //Find the first char
                    while ((c = fgetc(file)) != EOF) {
                        if (c != ' ')
                            break;
                    }
                    //Fill buffer with the vertex components
                    componentLength = 0;
                    do
                    {
                        if (c == EOF)
                            goto exit_loop2;
                        buffer[componentLength] = c;
                        componentLength++;
                    } while (((c = fgetc(file)) >= '0' && c <= '9') || c == '.');
                    //End component string
                    buffer[componentLength] = '\0';
                    //Load it into the vert array
                    t_vertices[(positions_read * positions.components) + componentsRead] = (float)atof(buffer);
                    //Check for model min/max
                    if (t_vertices[(positions_read * positions.components) + componentsRead] > modelMax[componentsRead])
                        modelMax[componentsRead] = t_vertices[(positions_read * positions.components) + componentsRead];
                    if (t_vertices[(positions_read * positions.components) + componentsRead] < modelMin[componentsRead])
                        modelMin[componentsRead] = t_vertices[(positions_read * positions.components) + componentsRead];
                    componentsRead++;
                } while (componentsRead<positions.components);
                positions_read++;
                if (c == '\n')
                    continue;
                componentsRead = 0;
                //Read all color components (if required)
                while (componentsRead<colors.components)
                {
                    //Find the first char
                    while ((c = fgetc(file)) != EOF) {
                        if (c != ' ')
                            break;
                    }
                    //Fill buffer with the color components
                    componentLength = 0;
                    do
                    {
                        if (c == EOF)
                            goto exit_loop2;
                        buffer[componentLength] = c;
                        componentLength++;
                    } while (((c = fgetc(file)) >= '0' && c <= '9') || c == '.');
                    //End component string
                    buffer[componentLength] = '\0';
                    //Load it into the color array
                    t_colors[(colors_read * colors.components) + componentsRead] = (float)atof(buffer);//t_colors
                    componentsRead++;
                }
                //If we read a color, increment count
                if (componentsRead > 0)
                    colors_read++;
                if (c == '\n')
                    continue;
                //Speed to the end of the vertex line
                while ((c = fgetc(file)) != '\n')
                {
                    if (c == EOF)
                        goto exit_loop2;
                }
                continue;//Skip to next iteration, otherwise we will miss a line
            case 'n':
                //Read normal line of file
                componentsRead = 0;
                //Read all components
                do
                {
                    //Find the first char
                    while ((c = fgetc(file)) != EOF) {
                        if (c != ' ')
                            break;
                    }
                    //Fill buffer with the normal components
                    componentLength = 0;
                    do
                    {
                        if (c == EOF)
                            goto exit_loop2;
                        buffer[componentLength] = c;
                        componentLength++;
                    } while (((c = fgetc(file)) >= '0' && c <= '9') || c == '.');
                    //End component string
                    buffer[componentLength] = '\0';
                    //Load it into the temporary normal array
                    t_normals[(normals_read * normals.components) + componentsRead] = (float)atof(buffer);
                    componentsRead++;
                } while (componentsRead<normals.components);
                normals_read++;
                if (c == '\n')
                    continue;
                //Speed to the end of the normal line
                while ((c = fgetc(file)) != '\n')
                {
                    if (c == EOF)
                        goto exit_loop2;
                }
                continue;//Skip to next iteration, otherwise we will miss a line
            case 't':
                //Read texture line of file
                componentsRead = 0;
                //Read all components
                do
                {
                    //Find the first char
                    while ((c = fgetc(file)) != EOF) {
                        if (c >= '0'&&c<='9')
                            break;
                    }
                    //Fill buffer with the vert/tex/norm index components
                    componentLength = 0;
                    do
                    {
                        if (c == EOF)
                            goto exit_loop2;
                        buffer[componentLength] = c;
                        componentLength++;
                    } while (((c = fgetc(file)) >= '0' && c <= '9') || c == '.');
                    //End component string
                    buffer[componentLength] = '\0';
                    //Load it into the temporary textures array
                    t_texcoords[(texcoords_read * texcoords.components) + componentsRead] = (float)atof(buffer);
                    componentsRead++;
                } while (componentsRead<texcoords.components);
                texcoords_read++;
                if (c == '\n')
                {
                    continue;
                }
                //Speed to the end of the texture line
                while ((c = fgetc(file)) != '\n')
                {
                    if (c == EOF)
                        goto exit_loop2;
                }
                continue;//Skip to next iteration, otherwise we will miss a line
            }
            break;
            //If the first char is 'f', increment face count
        case 'f':
            //Read face line of file
            componentsRead = 0;
            //Read all components
            do
            {
                //Find the first char
                while ((c = fgetc(file)) != EOF) {
                    if (c >= '0' && c <= '9')
                        break;
                }
                //Fill buffer with the components
                componentLength = 0;
                do
                {
                    if (c == EOF)
                        goto exit_loop2;
                    buffer[componentLength] = c;
                    componentLength++;
                } while ((c = fgetc(file)) >= '0' && c <= '9');
                //End component string
                buffer[componentLength] = '\0';
                //Decide which array to load it into (faces, tex pos, norm pos)
                switch (componentsRead % (1 + (int)face_hasNormals + (int)face_hasTexcoords))
                {
                    //This is a vertex index
                case 0: //Decrease value by 1, obj is 1-index, our arrays are 0-index
                    ((unsigned int *)faces.data)[(faces_read*faces.components) + (componentsRead / (1 + (int)face_hasNormals + (int)face_hasTexcoords))] = (unsigned int)std::strtoul(buffer, 0, 0) - 1;
                    break;
                    //This is a normal index
                case 1:
                    if (face_hasTexcoords)
                    {//Drop #2nd item onto texture if we have no normals
                        t_tex_pos[(faces_read*faces.components) + (componentsRead / (1 + (int)face_hasNormals + (int)face_hasTexcoords))] = (unsigned int)std::strtoul(buffer, 0, 0) - 1;
                        break;
                    }
                case 2:
                    t_norm_pos[(faces_read*faces.components) + (componentsRead / (1 + (int)face_hasNormals + (int)face_hasTexcoords))] = (unsigned int)std::strtoul(buffer, 0, 0) - 1;
                    break;
                    //This is a texture index
                }
                componentsRead++;
            } while (componentsRead<(unsigned int)((1 + (int)face_hasNormals + (int)face_hasTexcoords))*FACES_SIZE);
            faces_read++;
            if (c == '\n')
            {
                continue;
            }
        }
        //Speed to the end of the line and begin next iteration
        while (c!='\n')
        {
            lnLen++; 
            c = fgetc(file);
            if (c == EOF)
                goto exit_loop2;
        }
    }
exit_loop2:;
    //Cleanup buffer
    delete buffer;
    printf("\rLoading Model: %s [Calculating Pairs]", modelPath);
    auto vn_pairs = new std::unordered_map <
        VN_PAIR,
        unsigned int,
        std::function<unsigned long(VN_PAIR)>,
        std::function < bool(VN_PAIR, VN_PAIR) >
    >(faces.count*faces.components, hashing_func, key_equal_fn);
    //Calculate the number of unique vertex-normal pairs
    for (unsigned int i = 0; i < faces.count*faces.components; i++)
    {
        if (face_hasTexcoords)
            (*vn_pairs)[VN_PAIR(((unsigned int *)faces.data)[i], t_norm_pos[i], t_tex_pos[i])] = UINT_MAX;
        else if (face_hasNormals)
            (*vn_pairs)[VN_PAIR(((unsigned int *)faces.data)[i], t_norm_pos[i], 0)] = UINT_MAX;
        else
            (*vn_pairs)[VN_PAIR(((unsigned int *)faces.data)[i], 0, 0)] = UINT_MAX;
    }
    vn_count = (unsigned int)vn_pairs->size();
    
    //Allocate instance vars from a single malloc
    unsigned int bufferSize = 0;
    bufferSize += vn_count*positions.components*positions.componentSize;
    bufferSize += (normals.count>0)*vn_count*normals.components*normals.componentSize;
    bufferSize += (colors.count>0)*vn_count*colors.components*colors.componentSize;
    bufferSize += (texcoords.count>0)*vn_count*positions.components*positions.componentSize;
    positions.data = malloc(bufferSize);
    positions.count = vn_count;
    bufferSize = vn_count*positions.components*positions.componentSize;
    if (normals.count>0)
    {
        normals.data = (char*)positions.data + bufferSize;
        normals.count = vn_count;
        normals.offset = bufferSize;
        bufferSize += vn_count*normals.components*normals.componentSize;
    }
    if (colors.count>0)
    {
        colors.data = (char*)positions.data + bufferSize;
        colors.count = vn_count;
        colors.offset = bufferSize;
        bufferSize += vn_count*colors.components*colors.componentSize;
    }
    if (texcoords.count>0)
    {
        texcoords.data = (char*)positions.data + bufferSize;
        texcoords.count = vn_count;
        texcoords.offset = bufferSize;
        bufferSize += vn_count*texcoords.components*texcoords.componentSize;
    }
    //Calculate scale factor
    float scaleFactor = 1.0;
    if (SCALE>0)
        scaleFactor = SCALE / glm::compMax(modelMax - modelMin);
    printf("\rLoading Model: %s [Assigning Elements]", modelPath);
    unsigned int vn_assigned = 0;
    for (unsigned int i = 0; i < faces.count*faces.components; i++)
    {
        int i_tex = face_hasTexcoords ? t_tex_pos[i] : 0;
        int i_norm = face_hasNormals ? t_norm_pos[i] : 0;
        int i_vert = ((unsigned int *)faces.data)[i];
        glm::vec3 t_normalised_norm;
        //If vn pair hasn't been assigned an id yet
        if ((*vn_pairs)[VN_PAIR(i_vert, i_norm, i_tex)] == UINT_MAX)
        {
            //Set all n components of vertices and attributes to that id
            for (unsigned int k = 0; k < positions.components; k++)
                ((float*)positions.data)[(vn_assigned*positions.components) + k] = t_vertices[(i_vert*positions.components) + k] * scaleFactor;
            if (face_hasNormals)
            {//Normalise normals
                t_normalised_norm = normalize(glm::vec3(t_normals[(t_norm_pos[i] * normals.components)], t_normals[(t_norm_pos[i] * normals.components)] + 1, t_normals[(t_norm_pos[i] * normals.components)] + 2));
                for (unsigned int k = 0; k < normals.components; k++)
                    ((float*)normals.data)[(vn_assigned*normals.components) + k] = t_normalised_norm[k];
            }
            if (colors.count)
                for (unsigned int k = 0; k < colors.components; k++)
                    ((float*)colors.data)[(vn_assigned*colors.components) + k] = t_colors[(i_vert*colors.components) + k];
            if (face_hasTexcoords)
                for (unsigned int k = 0; k < texcoords.components; k++)
                    ((float*)texcoords.data)[(vn_assigned*texcoords.components) + k] = t_texcoords[(t_tex_pos[i] * texcoords.components) + k];
            //Assign it new lowest id
            (*vn_pairs)[VN_PAIR(i_vert, i_norm, i_tex)] = vn_assigned++;
        }
        //Update index from face
        ((unsigned int *)faces.data)[i] = (*vn_pairs)[VN_PAIR(i_vert, i_norm, i_tex)];
    }
    ////Free temps
    printf("\rLwaiting              ");
    std::thread([vn_pairs](){
        delete vn_pairs;
    }).detach();
    free(t_vertices);
    free(t_colors);
    free(t_normals);
    free(t_texcoords);
    free(t_norm_pos);
    free(t_tex_pos);
    //Load VBOs
    printf("\rLoading Model: %s [Generating VBOs!]              ", modelPath);
    generateVertexBufferObjects();
    //Can the host copies be freed after a bind?
    //free(vertices);
    //if (normals)
    //    free(normals);
    //if (colors)
    //    free(colors);
    //if (textures)
    //    free(textures);
    //free(faces);
    fclose(file);
    printf("\rLoading Model: %s [Complete!]                 \n", modelPath);
}
/*
Loads a single material from a .mtl file
@param objPath The path to the model (we extract the directory_
@param materialFilename The filename of the material file(.mtl)
@param materialName The name of the material
@todo - this only works with 1 material per file.
*/
void Entity::loadMaterialFromFile(const char *objPath, const char *materialFilename, const char *materialName){
    // Figure out the actual filepath, obj path dir but with matrial filename on the end.
    std::string objectPath = objPath;
    unsigned int found = (unsigned int)objectPath.find_last_of('/');
    std::string materialPath = objectPath.substr(0, found).append("/").append(materialFilename);

    //Open file
    FILE* file = fopen(materialPath.c_str(), "r");
    if (file == NULL){
        printf("Could not open material: '%s'!\n", materialPath.c_str());
        return;
    }
    // Prep vars for storing mtl properties
    char buffer[100];
    char temp[100];
    float r, g, b;
    int i;

    // identifier strings
    const char* MATERIAL_NAME_IDENTIFIER = "newmtl";
    const char* AMBIENT_IDENTIFIER = "Ka";
    const char* DIFFUSE_IDENTIFIER = "Kd";
    const char* SPECULAR_IDENTIFIER = "Ks";
    const char* SPECULAR_EXPONENT_IDENTIFIER = "Ns";
    const char* DISSOLVE_IDENTIFEIR = "d"; //alpha
    const char* ILLUMINATION_MODE_IDENTIFIER = "illum";

    this->material = new Material();

    // iter file
    while (!feof(file)) {
        if (fscanf(file, "%s", buffer) == 1){
            if (strcmp(buffer, MATERIAL_NAME_IDENTIFIER) == 0){
                if (fscanf(file, "%s", &temp) == 1){
                    //this->material->materialName = temp; // @todo
                }
                else {
                    printf("Bad material file...");
                }
            }
            else if (strcmp(buffer, AMBIENT_IDENTIFIER) == 0){
                if (fscanf(file, "%f %f %f", &r, &g, &b) == 3){
                    this->material->ambient = glm::vec4(r, g, b, 1.0);
                }
                else {
                    printf("Bad material file...");
                }
            }
            else if (strcmp(buffer, DIFFUSE_IDENTIFIER) == 0){
                if (fscanf(file, "%f %f %f", &r, &g, &b) == 3){
                    this->material->diffuse = glm::vec4(r, g, b, 1.0);

                }
                else {
                    printf("Bad material file...");
                }
            }
            else if (strcmp(buffer, SPECULAR_IDENTIFIER) == 0){
                if (fscanf(file, "%f %f %f", &r, &g, &b) == 3){
                    this->material->specular = glm::vec4(r, g, b, 1.0);

                }
                else {
                    printf("Bad material file...");
                }
            }
            else if (strcmp(buffer, SPECULAR_EXPONENT_IDENTIFIER) == 0){
                if (fscanf(file, "%f", &r) == 1){
                    this->material->shininess = r;
                }
                else {
                    printf("Bad material file...");
                }
            }
            else if (strcmp(buffer, DISSOLVE_IDENTIFEIR) == 0) {
                if (fscanf(file, "%f", &r) == 1){
                    this->material->dissolve = r;
                }
                else {
                    printf("Bad material file...");
                }
            }
            else if (strcmp(buffer, ILLUMINATION_MODE_IDENTIFIER) == 0) {
                if (fscanf(file, "%d", &i) == 1){
                    //@todo  ignore this for now.

                    //this->material->illuminationMode = i;
                }
                else {
                    printf("Bad material file...");
                }
            }

        }
    }

}
/*
Set the color of the model
If the model has an associated material this value will be ignored
*/
void Entity::setColor(glm::vec3 color){
    this->color = color;
}
/*
Set the location of the model in world space
*/
void Entity::setLocation(glm::vec3 location){
    this->location = location;
}
/*
Set the rotation of the model in world space
@param rotation glm::vec4(axis.x, axis.y, axis.z, degrees)
*/
void Entity::setRotation(glm::vec4 rotation){
    this->rotation = rotation;
}
/*
Returns the location of the entity
@return a vec3 containing the x, y, z coords the model should be translated to
*/
glm::vec3 Entity::getLocation() const
{
    return this->location;
}
/*
Returns the rotation of the entity
@return a vec4 containing the x, y, z coords of the axis the model should be rotated w degrees around
*/
glm::vec4 Entity::getRotation() const
{
    return this->rotation;
}
/*
Public alias to freeMaterial() for backwards compatibility purposes
@see freeMaterial()
*/
void Entity::clearMaterial(){
    freeMaterial();
}
/*
Frees the storage for the model's material
*/
void Entity::freeMaterial(){
    if (this->material){
        delete this->material;
        this->material = 0;
    }
}
/*
Exports the current model to a faster loading binary format which represents a direct copy of the buffers required by the model
Models are stored by appending .sdl_export to their existing filename
Models are stored in the following format;
#Header
[1 byte]                File type flag
[1 byte]                Exporter version
[1 byte]                float length (bytes)
[1 byte]                uint length (bytes)
[1 float]               Model scale (length of longest axis)
[1 uint]                vn_count
[1 bit]                 File contains (float) vertices of size 3
[1 bit]                 File contains (float) vertices of size 4
[1 bit]                 File contains (float) normals of size 3
[1 bit]                 File contains (float) colors of size 3
[1 bit]                 File contains (float) colors of size 4
[1 bit]                 File contains (float) texcoords of size 2
[1 bit]                 File contains (float) texcoords of size 3
[1 bit]                 File contains (uint) faces of size 3
[4 byte]                Reserved space for future expansion
##Data## (Per item marked true from the bit fields in the header)
[1 uint]                (faces only) Number of items
[n x size float/uint]   Item data
##Footer##
[1 byte]    File type flag
*/
void Entity::exportModel() const
{
    std::string exportPath(modelPath);
    std::string objPath(OBJ_TYPE);
    if (!endsWith(modelPath, EXPORT_TYPE))
    {
        exportPath = exportPath.substr(0, exportPath.length() - objPath.length()).append(EXPORT_TYPE);
    }
    FILE * file;
    if ((file = fopen(exportPath.c_str(), "r")))
    {
        fprintf(stderr, "Cannot export model, file already exists.\n");
        fclose(file);
        return;
    }
    file = fopen(exportPath.c_str(), "wb");
    if (!file)
    {
        fprintf(stderr, "Could not open file for writing: %s\n", exportPath.c_str());
    }
    printf("Exporting model: %s\n", exportPath.c_str());
    //Generate export mask
    ExportMask mask
    {
        FILE_TYPE_FLAG,
        FILE_TYPE_VERSION,
        sizeof(float),
        sizeof(unsigned int),
        SCALE,
        vn_count,
        positions.count&&positions.components == 3,
        positions.count&&positions.components == 4,
        normals.count&&normals.components == 3,
        colors.count&&colors.components == 3,
        colors.count&&colors.components == 4,
        texcoords.count&&texcoords.components == 2,
        texcoords.count&&texcoords.components == 3,
        faces.count&&faces.components == 3,
        0
    };
    //Write out the export mask
    fwrite(&mask, sizeof(ExportMask), 1, file);
    //Write out each buffer in order
    if (positions.count && (positions.components == 3 || positions.components == 4))
    {
        fwrite(positions.data, positions.componentSize, positions.count*positions.components, file);
    }
    if (normals.count && (NORMALS_SIZE == 3))
    {
        fwrite(positions.data, positions.componentSize, positions.count*positions.components, file);
    }
    if (colors.count && (colors.components == 3 || colors.components == 4))
    {
        fwrite(colors.data, colors.componentSize, colors.count*colors.components, file);
    }
    if (texcoords.count && (texcoords.components == 2 || texcoords.components == 3))
    {
        fwrite(texcoords.data, texcoords.componentSize, texcoords.count*texcoords.components, file);
    }
    if (faces.count && (faces.components == 3))
    {
        fwrite(&faces.count, sizeof(unsigned int), 1, file);
        fwrite(faces.data, faces.componentSize, faces.count*faces.components, file);
    }
    //Finish by writing the file type flag again
    fwrite(&FILE_TYPE_FLAG, sizeof(char), 1, file);
    fclose(file);
}
/*
Util method used to check whether the candidate string ends with the suffix
@param candidate The string we will search for the suffix
@param suffix The substring to check whether
*/
bool Entity::endsWith(const char *candidate, const char *suffix)
{
    std::string t_candidate(candidate);
    std::string t_suffix(suffix);
    std::transform(t_candidate.begin(), t_candidate.end(), t_candidate.begin(), ::tolower);
    std::transform(t_suffix.begin(), t_suffix.end(), t_suffix.begin(), ::tolower);
    return (t_suffix == t_candidate.substr(t_candidate.length() - t_suffix.length(), t_suffix.length()));
}
/*
Exports the current model to a fast loading binary format which represents a direct copy of the buffers required by the model
@param file Path to the desired output file
*/
void Entity::importModel(const char *path)
{
    //Generate import path
    std::string importPath(path);
    std::string objPath(OBJ_TYPE);
    if (endsWith(path, OBJ_TYPE))
    {
        importPath = importPath.substr(0, importPath.length() - objPath.length()).append(EXPORT_TYPE);
    }
    else if (!endsWith(path, EXPORT_TYPE))
    {
        fprintf(stderr, "Model File: %s, is not a support filetype (e.g. %s, %s)\n", path, OBJ_TYPE, EXPORT_TYPE);
        return;
    }
    //Open file
    FILE * file;
    file = fopen(importPath.c_str(), "rb");
    if (!file)
    {
        fprintf(stderr, "Could not open file for reading: %s. Aborting import\n", importPath.c_str());
    }
    printf("Importing Model: %s\n", importPath.c_str());
    //Read in the export mask
    ExportMask mask;
    fread(&mask, sizeof(ExportMask), 1, file);
    //Check file type flag exists
    if (mask.FILE_TYPE_FLAG != FILE_TYPE_FLAG)
    {
        fprintf(stderr, "FILE TYPE FLAG missing from file header: %s. Aborting import\n", importPath.c_str());
        fclose(file);
        return;
    }
    //Check version is supported
    if (mask.VERSION_FLAG > FILE_TYPE_VERSION)
    {
        fprintf(stderr, "File %s is of newer version %i, this software supports a maximum version of %i. Aborting import\n", importPath.c_str(), (unsigned int)mask.VERSION_FLAG, (unsigned int)FILE_TYPE_VERSION);
        fclose(file);
        return;
    }
    //Check float/uint lengths aren't too short
    if (sizeof(float) != mask.SIZE_OF_FLOAT)
    {
        fprintf(stderr, "File %s uses floats of %i bytes, this architecture has floats of %i bytes. Aborting import\n", importPath.c_str(), mask.SIZE_OF_FLOAT, sizeof(float));
        fclose(file);
        return;
    }
    if (sizeof(unsigned int) != mask.SIZE_OF_UINT)
    {
        fprintf(stderr, "File %s uses uints of %i bytes, this architecture has floats of %i bytes. Aborting import\n", importPath.c_str(), mask.SIZE_OF_UINT, sizeof(unsigned));
        fclose(file);
        return;
    }
    vn_count = mask.VN_COUNT;    
    //Set components (sizes should be defaults)
    positions.components = mask.FILE_HAS_VERTICES_3 ? 3 : 4;
    normals.components = mask.FILE_HAS_NORMALS_3 ? 3 : NORMALS_SIZE;
    colors.components = mask.FILE_HAS_COLORS_3 ? 3 : 4;
    texcoords.components = mask.FILE_HAS_TEXCOORDS_2 ? 2 : 3;
    //Allocate instance vars from a single malloc
    unsigned int bufferSize = 0;
    bufferSize += vn_count*positions.components*positions.componentSize;//Positions required
    bufferSize += (mask.FILE_HAS_NORMALS_3)*vn_count*normals.components*normals.componentSize;
    bufferSize += (mask.FILE_HAS_COLORS_3 || mask.FILE_HAS_COLORS_4)*vn_count*colors.components*colors.componentSize;
    bufferSize += (mask.FILE_HAS_TEXCOORDS_2 || mask.FILE_HAS_TEXCOORDS_3)*vn_count*positions.components*positions.componentSize;
    positions.data = malloc(bufferSize);
    positions.count = vn_count;
    bufferSize = vn_count*positions.components*positions.componentSize;
    if (mask.FILE_HAS_NORMALS_3)
    {
        normals.data = (char*)positions.data + bufferSize;
        normals.count = vn_count;
        normals.offset = bufferSize;
        bufferSize += vn_count*normals.components*normals.componentSize;
    }
    if (mask.FILE_HAS_COLORS_3 || mask.FILE_HAS_COLORS_4)
    {
        colors.data = (char*)positions.data + bufferSize;
        colors.count = vn_count;
        colors.offset = bufferSize;
        bufferSize += vn_count*colors.components*colors.componentSize;
    }
    if (mask.FILE_HAS_TEXCOORDS_2 || mask.FILE_HAS_TEXCOORDS_3)
    {
        texcoords.data = (char*)positions.data + bufferSize;
        texcoords.count = vn_count;
        texcoords.offset = bufferSize;
        //bufferSize += vn_count*texcoords.components*texcoords.componentSize;
    }

    //Read in buffers
    if (positions.count)
    {
        if (vn_count*positions.components != fread(positions.data, positions.componentSize, vn_count*positions.components, file))
        {
            fprintf(stderr, "fread err: vertices\n");
        };
    }
    if (normals.count)
    {
        if (vn_count*normals.components != fread(normals.data, normals.componentSize, vn_count*normals.components, file))
        {
            fprintf(stderr, "fread err: normals\n");
        };
    }
    if (colors.count)
    {
        if (vn_count*colors.components != fread(colors.data, colors.componentSize, vn_count*colors.components, file))
        {
            fprintf(stderr, "fread err: colors\n");
        };
    }
    if (texcoords.count)
    {
        if (vn_count*texcoords.components != fread(texcoords.data, texcoords.componentSize, vn_count*texcoords.components, file))
        {
            fprintf(stderr, "fread err: textures\n");
        };
    }
    if (mask.FILE_HAS_FACES_3)
    {
        fread(&faces.count, sizeof(unsigned int), 1, file);
        faces.data = malloc(faces.componentSize*faces.components*faces.count);
        if (faces.count*faces.components != fread(faces.data, sizeof(unsigned int), faces.count*faces.components, file))
        {
            fprintf(stderr, "fread err: faces\n");
        };
    }
    //Check file footer contains flag (to confirm it was closed correctly
    fread(&mask.FILE_TYPE_FLAG, sizeof(char), 1, file);
    if (mask.FILE_TYPE_FLAG != FILE_TYPE_FLAG)
    {
        fprintf(stderr, "FILE TYPE FLAG missing from file footer: %s, model may be corrupt.\n", importPath.c_str());
        return;
    }
    fclose(file);
    //Check model scale
    if (SCALE>0 && mask.SCALE <= 0)
    {
        fprintf(stderr, "File %s contains a model of scale: %.3f, this is invalid, model will not be scaled.\n", importPath.c_str(), mask.SCALE);
    }
    //Scale the model
    else if (SCALE>0 && mask.SCALE != SCALE)
    {
        float scaleFactor = SCALE / mask.SCALE;
        for (unsigned int i = 0; i < positions.count*positions.components; i++)
            ((float*)positions.data)[i] *= scaleFactor;
    }
    //Allocate VBOs
    generateVertexBufferObjects();
    printf("Model import was successful: %s\n", importPath.c_str());
}
/*
Creates the necessary vertex buffer objects, and fills them with the relevant instance var data.
*/
void Entity::generateVertexBufferObjects()
{/*
    if (texcoords.count>0)
    {
        Shaders::VertexAttributeDetail t = colors;
        colors = texcoords;
        texcoords = t;
    }*/
    unsigned int bufferSize = 0;
    bufferSize += positions.count * positions.components * positions.componentSize;//Positions required
    bufferSize +=   normals.count *   normals.components *   normals.componentSize;
    bufferSize +=    colors.count *    colors.components *    colors.componentSize;
    bufferSize += texcoords.count * texcoords.components * texcoords.componentSize;
    createVertexBufferObject(&positions.vbo, GL_ARRAY_BUFFER, bufferSize, positions.data);
    unsigned int offset = vn_count*positions.components*positions.componentSize;
    if (normals.count)
    {
        normals.vbo = positions.vbo;
        //redundant
        normals.offset = offset;
        offset += normals.count*normals.components*normals.componentSize;
    }
    if (colors.count)
    {
        colors.vbo = positions.vbo;
        //redundant
        colors.offset = offset;
        offset += colors.count*colors.components*colors.componentSize;
    }
    if (texcoords.count)
    {
        texcoords.vbo = positions.vbo;
        //redundant
        texcoords.offset = offset;
        //offset += texcoords.count*texcoords.components*texcoords.componentSize;
    }
    createVertexBufferObject(&faces.vbo, GL_ELEMENT_ARRAY_BUFFER, faces.count*faces.components*faces.componentSize, faces.data);
}
/*
Returns a shared pointer to this entities shaders
*/
std::shared_ptr<Shaders> Entity::getShaders() const
{
    if (shaders.get())
        return shaders;
    else
        return std::shared_ptr<Shaders>(0);
}

void Entity::reload()
{
    if (shaders.get())
        shaders->reload(true);
    if (texture.get())
        texture->reload();
}
#endif //ifdef __Entity_impl_cpp__