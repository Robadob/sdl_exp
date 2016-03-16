#define  _CRT_SECURE_NO_WARNINGS
#include "Entity.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <functional>

#define NORMALS_SIZE 3
#define DEFAULT_TEXTURE_SIZE 2
#define FACES_SIZE 3

/*
Constructs an entity from the provided .obj model
@param modelPath Path to .obj format model file
@param modelScale World size to scale the longest direction (in the x, y or z) axis of the model to fit
*/
Entity::Entity(const char *modelPath, float modelScale, Shaders *shaders)
    : vertices(0), normals(0), colors(0), textures(0), faces(0)
    , vertices_vbo(0), normals_vbo(0), colors_vbo(0), textures_vbo(0), faces_vbo(0)
    , v_count(0), n_count(0), c_count(0), t_count(0), f_count(0), vn_count(0)
    , v_size(3), c_size(3), t_size(2)
    , SCALE(modelScale)
    , material(0)
    , color(1,0,0)
    , location(0.0f)
    , rotation(0.0f)
    , shaders(shaders)
{
    loadModelFromFile(modelPath, modelScale);
    //If shaders have been provided, set them up
    if (vertices&&this->shaders)
    {
        this->shaders->setVertexAttributeDetail(vertices_vbo, 0, v_size, 0);
        if (normals_vbo)
            this->shaders->setVertexNormalAttributeDetail(normals_vbo, 0, NORMALS_SIZE, 0);
        if (colors_vbo)
            this->shaders->setVertexColorAttributeDetail(colors_vbo, 0, c_size, 0);
        if (textures_vbo)
            this->shaders->setVertexTextureAttributeDetail(textures_vbo, 0, t_size, 0);
    }
}
/*
Destructor, free's memory allocated to store the model and its material
*/
Entity::~Entity(){
    deleteVertexBufferObject(&vertices_vbo);
    if (normals_vbo)
        deleteVertexBufferObject(&normals_vbo);
    if (colors_vbo)
        deleteVertexBufferObject(&colors_vbo);
    if (textures_vbo)
        deleteVertexBufferObject(&textures_vbo);
    deleteVertexBufferObject(&faces_vbo);
    freeModel();
    freeMaterial();
}
/*
Calls the necessary code to render a single instance of the entity
@param vertLocation The shader attribute location to pass vertices
@param normalLocation The shader attribute location to pass normals
*/
void Entity::render(){
    if (shaders)
        shaders->useProgram();
    //Bind the faces to be rendered
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_vbo));

    glPushMatrix();
        //Translate the model according to it's location
        glMatrixMode(GL_MODELVIEW);
        GL_CALL(glRotatef(rotation.w, rotation.x, rotation.y, rotation.z));
        GL_CALL(glTranslatef(location.x, location.y, location.z));
        //Set the color and material
        if (this->material)
            this->material->useMaterial();
        GL_CALL(glColor4f(color.x, color.y, color.z, 1.0));
        GL_CALL(glDrawElements(GL_TRIANGLES, f_count * 3, GL_UNSIGNED_INT, 0));
    glPopMatrix();
    if (shaders)
        shaders->clearProgram();
}
/*
Calls the necessary code to render count instances of the entity
The index of the instance being rendered can be identified within the vertex shader with the variable gl_InstanceID
@param count The number of instances of the entity to render
@param vertLocation The shader attribute location to pass vertices
@param normalLocation The shader attribute location to pass normals
*/
void Entity::renderInstances(int count, GLuint vertLocation, GLuint normalLocation){
    if (shaders)
        shaders->useProgram();
    //Bind the faces to be rendered
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_vbo));

    glPushMatrix();
        //Translate the model(s) according to it's(their) location
        glMatrixMode(GL_MODELVIEW);
        GL_CALL(glRotatef(rotation.w, rotation.x, rotation.y, rotation.z));
        GL_CALL(glTranslatef(location.x, location.y, location.z));
        //Set the color and material
        if (this->material)
            this->material->useMaterial();
        GL_CALL(glColor4f(color.x, color.y, color.z, 1.0));
        GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, f_count * 3, GL_UNSIGNED_INT, 0, count));
    glPopMatrix();
    if (shaders)
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
unsigned long hashing_func(std::pair<unsigned int, unsigned int> key)
{
    return (!(key.first) << (sizeof(unsigned long) - sizeof(unsigned int))) ^ key.second;
}
/*
Used by loadModelFromFile() in a hashmap of vertex-normal pairs
*/
bool key_equal_fn(std::pair<unsigned int, unsigned int> t1, std::pair<unsigned int, unsigned int> t2)
{
    return t1.first == t2.first&&t1.second == t2.second;
}
/*
Loads and scales the specified model into this class's primitive storage
@param path The path to the .obj model to be loaded
@param modelScale The scale in world space which the model's longest distance (along the x, y or z axis) should be scaled to
@note Loading of vertex normals was disabled to suit some models that have non matching counts of vertices and vertex normals
*/
void Entity::loadModelFromFile(const char *path, float modelScale)
{
    printf("\r Loading Model: %s", path);
    //vn -3.631894 -0.637774 1.635071
    //vn norm.x norm.y norm.z
    //v - 1.878153 - 3.189111 0.041601 0.043137 0.039216 0.027451
    //v vert.x vert.y vert.z color.x color.y color.z
    //v may be in the form v v v, v v v v, v v v v c c c
    //f 85291//85291 85947//85947 86027//86027
    //f vert1/tex1/norm1 vert2/tex2/norm2 vert3/tex3/norm3
    //f may be in form: v, v//n or v/t/n

    //const char* VERTEX_IDENTIFIER = "v";
    //const char* VERTEX_NORMAL_IDENTIFIER = "vn";
    //const char* FACE_IDENTIFIER = "f";
    //const char* MTLLIB_IDENTIFIER = "mtllib";
    //const char* USEMTL_IDENTIFEIR = "usemtl";
    ////Placeholders
    //char buffer[100];
    //float x,y,z;
    //int f1a, f1b, f2a, f2b, f3a, f3b;
    //char materialFile[100] = ""; // @todo - this could be improved;
    //char materialName[100] = ""; // @todo - this could be improved;
    //Open file
    FILE* file = fopen(path, "r");
    //std::ifstream file(path);
    if (!file){
        printf("\rLoading Model: Could not open model '%s'!\n", path);
        return;
    }


    //Counters
    unsigned int vertices_read = 0;
    unsigned int normals_read = 0;
    unsigned int colors_read = 0;
    unsigned int textures_read = 0;
    unsigned int faces_read = 0;
    unsigned int parameters_read = 0;

    unsigned int vertices_size = 3;
    unsigned int colors_size = 0;
    unsigned int textures_size = 2;

    bool face_hasNormals = false;
    bool face_hasTextures = false;

    printf("\rLoading Model: %s [Counting Elements]                ", path);
    printf("\n");//Temp whilst we have vert counters inside loop
    ////Count vertices/faces, attributes
    char c;
    int dotCtr;
    unsigned int lnLen = 0, lnLenMax = 0;//Used to find the longest line of the file
    //For each line of the file (the end of the loop finds the end of the line before continuing)
    while ((c = fgetc(file)) != EOF) {
        lnLenMax = lnLenMax < lnLen ? lnLen : lnLenMax;
        lnLen=1;
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
                vertices_read++;
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
                    colors_size = 4;
                case 4:
                    vertices_size = 4;
                    break;
                case 7:
                    vertices_size = 4;
                case 6:
                    colors_size = 3;
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
                textures_read++;
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
                textures_size = dotCtr;
                continue;//Skip to next iteration, otherwise we will miss a line
            }
            break;
            //If the first char is 'f', increment face count
        case 'f':
            faces_read++;
            dotCtr=0;
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
                        face_hasTextures = true;
                        if (c == EOF)
                            goto exit_loop;
                    }
                    break;
                }
            }
            break;
        }
        //Speed to the end of the line and begin next iteration
        while ((c = fgetc(file)) != '\n')
        {
            lnLen++;
            if (c == EOF)
                goto exit_loop;
        }
        //printf("\rVert: %i:%i, Normal: %i, Face: %i, Tex: %i:%i, Color: %i:%i, Ln:%i", vertices_read, vertices_size, normals_read, faces_read, textures_read, textures_size, colors_read, colors_size, lnLenMax);
    }
exit_loop:;
    lnLenMax = lnLenMax < lnLen ? lnLen : lnLenMax;

    if (parameters_read > 0){
        fprintf(stderr, "Model '%s' contains parameter space vertices, these are unsupported at this time.", path);
        return;
    }

    //Set instance var counts
    v_count = vertices_read;
    c_count = colors_read;
    n_count = normals_read;
    t_count = textures_read;
    f_count = faces_read;
    if (v_count == 0 || f_count == 0)
    {
        fprintf(stderr, "Vertex or face data missing.\nAre you sure that '%s' is a wavefront (.obj) format model?\n");
        fclose(file);
        return;
    }
    if ((c_count != 0 && v_count != c_count))
    {
        fprintf(stderr, "Vertex color count does not match vertex count, vertex colors will be ignored.\n");
        c_count = 0;
    }
    if(t_count != 0 && v_count != t_count)
    {
        fprintf(stderr, "Vertex texture count does not match vertex count, vertex textures will be ignored.\n");
        t_count = 0;
    }
    //Set instance var sizes
    v_size = vertices_size;//3-4
    t_size = textures_size;//2-3
    c_size = colors_size;//3-4
    //Allocate faces
    faces = (unsigned int*)malloc(f_count*FACES_SIZE*sizeof(unsigned int));
    //Reset file pointer
    clearerr(file);
    fseek(file, 0, SEEK_SET);
    //Allocate temporary buffers for components that may require aligning with relevant vertices
    //TODO: Create T_Normal, T_Color, T_Texture, T_norm_pos, T_tex_pos
    float *t_vertices = (float *)malloc(vertices_read * vertices_size * sizeof(float));
    float *t_colors = (float *)malloc(colors_read * colors_size * sizeof(float));
    float *t_normals = (float *)malloc(normals_read * NORMALS_SIZE * sizeof(float));
    float *t_textures = (float *)malloc(textures_read*textures_size*sizeof(float));
    //float *t_colors = (float *)malloc(colors_read*colors_size*sizeof(float));
    //3 parts to each face,store the relevant norm and tex indexes
    unsigned int *t_norm_pos=0;
    if (face_hasNormals)
        t_norm_pos = (unsigned int *)malloc(f_count*FACES_SIZE*sizeof(unsigned int));
    else
        n_count = 0;
    unsigned int *t_tex_pos=0;
    if (face_hasTextures)
        t_tex_pos = (unsigned int *)malloc(f_count*FACES_SIZE*sizeof(unsigned int));
    else
        v_count;
    //Reset local counters
    vertices_read = 0;
    colors_read = 0;
    normals_read = 0;
    textures_read = 0;
    faces_read = 0;
    unsigned int componentsRead = 0;
    unsigned int componentLength = 0;
    //Create buffer to read lines of the file into
    unsigned int bufferLen = lnLenMax + 2;
    char *buffer = new char[bufferLen];

    printf("\rLoading Model: %s [Loading Elements]      ", path);
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
                    t_vertices[(vertices_read * v_size) + componentsRead] = (float)atof(buffer);
                    componentsRead++;
                } while (componentsRead<v_size);
                vertices_read++;
                if (c == '\n')
                    continue;
                componentsRead = 0;
                //Read all color components (if required)
                while (componentsRead<c_size)
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
                    } while (((c = fgetc(file)) >= '0' && c <= '9' )|| c == '.');
                    //End component string
                    buffer[componentLength] = '\0';
                    //Load it into the color array
                    t_colors[(colors_read * c_size) + componentsRead] = (float)atof(buffer);//t_colors
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
                    t_normals[(normals_read * NORMALS_SIZE) + componentsRead] = (float)atof(buffer);
                    componentsRead++;
                } while (componentsRead<NORMALS_SIZE);
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
                        if (c != ' ')
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
                    t_textures[(textures_read * t_size) + componentsRead] = (float)atof(buffer);
                } while (componentsRead<DEFAULT_TEXTURE_SIZE);
                //Read the final texture element if provided (special case, enclosed in [])
                if (componentsRead < t_size)//If 3 texture coords
                {
                    //Find the first char
                    while ((c = fgetc(file)) != EOF) {
                        if (c != ' ')
                            break;
                    }
                    if (c == '[')
                    {
                        //Fill buffer with the components
                        componentLength = 0;
                        while ((c = fgetc(file)) != ']');
                        {
                            if (c == EOF)
                                goto exit_loop2;
                            buffer[componentLength] = c;
                            componentLength++;
                        }
                        //End component string
                        buffer[componentLength] = '\0';
                        //Load it into the temporary textures array
                        t_textures[(textures_read * t_size) + componentsRead] = (float)atof(buffer);
                    }
                    componentsRead++;
                }
                textures_read++;
                if (c == '\n')
                    continue;
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
                } while ((c = fgetc(file)) >='0' && c<='9');
                //End component string
                buffer[componentLength] = '\0';
                //Decide which array to load it into (faces, tex pos, norm pos)
                switch (componentsRead%(1+(int)face_hasNormals+(int)face_hasTextures))
                {
                //This is a vertex index
                case 0: //Decrease value by 1, obj is 1-index, our arrays are 0-index
                    faces[(faces_read*FACES_SIZE) + (componentsRead / (1 + (int)face_hasNormals + (int)face_hasTextures))] = (unsigned int)std::strtoul(buffer, 0, 0)-1;
                    break;
                //This is a normal index
                case 1:
                    t_norm_pos[(faces_read*FACES_SIZE) + (componentsRead / (1 + (int)face_hasNormals + (int)face_hasTextures))] = (unsigned int)std::strtoul(buffer, 0, 0)-1;
                    break;
                //This is a texture index
                case 2:
                    t_tex_pos[(faces_read*FACES_SIZE) + (componentsRead / (1 + (int)face_hasNormals + (int)face_hasTextures))] = (unsigned int)std::strtoul(buffer, 0, 0)-1;
                    break;
                }
                componentsRead++;

               
            } while (componentsRead<(unsigned int)((1 + (int)face_hasNormals + (int)face_hasTextures))*FACES_SIZE);
            faces_read++;
            if (c == '\n')
                continue;
        }
        //Speed to the end of the line and begin next iteration
        while ((c = fgetc(file)) != '\n')
        {
            lnLen++;
            if (c == EOF)
                goto exit_loop2;
        }
    }
exit_loop2:;
    //Cleanup buffer
    delete buffer;
    printf("\rLoading Model: %s [Calculating VN Pair Count]       ", path);
    std::unordered_map<
        std::pair<unsigned int, unsigned int>, 
        unsigned int,
        std::function<unsigned long(std::pair<unsigned int, unsigned int>)>,
        std::function<bool(std::pair<unsigned int, unsigned int>, std::pair<unsigned int, unsigned int>) >
    > vn_pairs(f_count*FACES_SIZE, hashing_func, key_equal_fn);
    vn_pairs.reserve(f_count*FACES_SIZE);//f_count is max number of pairs, should reduce resizing
    //Calculate the number of unique vertex-normal pairs
    for (unsigned int i = 0; i < f_count*FACES_SIZE; i++)
    {
        if (face_hasNormals)
            vn_pairs[std::pair<unsigned int, unsigned int>(faces[i], t_norm_pos[i])] = UINT_MAX;
        else
            vn_pairs[std::pair<unsigned int, unsigned int>(faces[i], 0)] = UINT_MAX;

    }
    vn_count = vn_pairs.size();
    //Allocate instance vars
    vertices = (float*)malloc(vn_count*v_size*sizeof(float));
    if (face_hasNormals)//1 normal per vertex
        normals = (float*)malloc(vn_count*NORMALS_SIZE*sizeof(float));
    if (c_count)//1 color per vertex
        colors = (float*)malloc(vn_count*c_size*sizeof(float));
    if (face_hasTextures)//1 texture per vertex
        textures = (float*)malloc(vn_count*t_size*sizeof(float));

    printf("\rLoading Model: %s [Assigning Elements]", path);
    unsigned int vn_assigned = 0;
    for (unsigned int i = 0; i < f_count*FACES_SIZE; i++)
    {
        int i_norm = face_hasNormals ? t_norm_pos[i] : 0;
        int i_vert = faces[i];
        //If vn pair hasn't been assigned an id yet
        if (vn_pairs[std::pair<unsigned int, unsigned int>(i_vert, i_norm)] == UINT_MAX)
        {
            //Set all n components of vertices and attributes to that id
            for (unsigned int k = 0; k < v_size; k++)
                vertices[(vn_assigned*v_size) + k] = t_vertices[(i_vert*v_size) + k];
            if (face_hasNormals)
                for (unsigned int k = 0; k < NORMALS_SIZE; k++)
                    normals[(vn_assigned*NORMALS_SIZE) + k] = t_normals[(t_norm_pos[i] * NORMALS_SIZE) + k];
            if (c_count)
                for (unsigned int k = 0; k < c_size; k++)
                    colors[(vn_assigned*c_size) + k] = t_colors[(i_vert*c_size) + k];
            if (face_hasTextures)
                for (unsigned int k = 0; k < t_size; k++)
                    textures[(vn_assigned*t_size) + k] = t_textures[(t_tex_pos[i] * t_size) + k];
            //Assign it new lowest id
            vn_pairs[std::pair<unsigned int, unsigned int>(i_vert, i_norm)] = vn_assigned++;
        }
        //Update index from face
        faces[i] = vn_pairs[std::pair<unsigned int, unsigned int>(i_vert, i_norm)];
    }
    //Free temps
    free(t_vertices);
    free(t_colors);
    free(t_normals);
    free(t_textures);
    free(t_norm_pos);
    free(t_tex_pos);
    //Load VBOs
    printf("\rLoading Model: %s [Generating VBOs!]              ", path);

    createVertexBufferObject(&vertices_vbo, GL_ARRAY_BUFFER, vn_count*v_size*sizeof(float), (void*)vertices);
    if (face_hasNormals)
        createVertexBufferObject(&normals_vbo, GL_ARRAY_BUFFER, vn_count*NORMALS_SIZE*sizeof(float), (void*)normals);
    if (c_count)
        createVertexBufferObject(&colors_vbo, GL_ARRAY_BUFFER, vn_count*c_size*sizeof(float), (void*)colors);
    if (face_hasTextures)
        createVertexBufferObject(&textures_vbo, GL_ARRAY_BUFFER, vn_count*t_size*sizeof(float), (void*)textures);
    createVertexBufferObject(&faces_vbo, GL_ELEMENT_ARRAY_BUFFER, f_count*FACES_SIZE*sizeof(unsigned int), (void*)faces);
    printf("\rLoading Model: %s [Complete!]                 \n", path);
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
    unsigned found = objectPath.find_last_of('/');
    std::string materialPath = objectPath.substr(0, found).append("/").append(materialFilename);

    //Open file
    FILE* file = fopen(materialPath.c_str(), "r");
    if (file == NULL){
        printf("Could not open ,material '%s'!\n", materialPath.c_str());
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
Frees the storage for the model's primitives
@see allocateModel()
*/
void Entity::freeModel(){
    free(vertices);
    if (normals)
        free(normals);
    if (colors)
        free(colors);
    if (textures)
        free(textures);
    free(faces);
}
/*
Scales the values within vertices according to the provided scale
@param modelScale Scaling factor
*/
void Entity::scaleModel(float modelScale){
    unsigned int v_total = v_count*v_size;
    for (unsigned int i = 0; i<v_total; i++)
    {
        vertices[i]*=modelScale;
    }
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
