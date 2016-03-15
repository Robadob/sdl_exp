#define  _CRT_SECURE_NO_WARNINGS
#include "Entity.h"

#include <fstream>
#include <sstream>
#include <string>
#include <regex>

/*
Constructs an entity from the provided .obj model
@param modelPath Path to .obj format model file
@param modelScale World size to scale the longest direction (in the x, y or z) axis of the model to fit
*/
Entity::Entity(const char *modelPath, float modelScale, Shaders *shaders)
    : vertices(0), normals(0), colors(0), textures(0), faces(0)
    , vertices_vbo(0), normals_vbo(0), colors_vbo(0), textures_vbo(0), faces_vbo(0)
    , v_count(0), n_count(0), c_count(0), t_count(0), f_count(0)
    , v_size(3), c_size(3), t_size(2)
    , SCALE(modelScale)
    , material(0)
    , color(1,0,0)
    , location(0.0f)
    , rotation(0.0f)
    , shaders(shaders)
{
    loadModelFromFile(modelPath, modelScale);
    if (!vertices)
        return;
    createVertexBufferObject(&vertices_vbo, GL_ARRAY_BUFFER, v_count*sizeof(glm::vec3)*2);//vertices+norms
    createVertexBufferObject(&faces_vbo, GL_ELEMENT_ARRAY_BUFFER, f_count*sizeof(glm::ivec3));
    fillBuffers();
    //If shaders have been provided, set them up
    if (this->shaders)
    {
        this->shaders->setVertexAttributeDetail(vertices_vbo, 0, 3, 0);
        this->shaders->setVertexNormalAttributeDetail(vertices_vbo, v_count*sizeof(glm::vec3), 3, 0);
    }
}
/*
Destructor, free's memory allocated to store the model and its material
*/
Entity::~Entity(){
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
void Entity::createVertexBufferObject(GLuint *vbo, GLenum target, GLuint size){
    GL_CALL(glGenBuffers(1, vbo));
    GL_CALL(glBindBuffer(target, *vbo));
    GL_CALL(glBufferData(target, size, 0, GL_STATIC_DRAW));
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
Copies the vertex, normal and face data into the relevant buffer objects
*/
void Entity::fillBuffers(){
    glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
    glBufferData(GL_ARRAY_BUFFER, v_count*sizeof(glm::vec3)*2, vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, f_count*sizeof(glm::ivec3), faces, GL_DYNAMIC_DRAW);
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

    //const std::regex VERTEX_LINE("v +(-?[0-9]+\.[0-9]+ *){3,8}", std::regex::optimize);
   // const std::regex TEXTURE_LINE("vt +(-?[0-9]+\.[0-9]+ *){3}", std::regex::optimize);
    //const std::regex NORMAL_LINE("vn +(-?[0-9]+\.[0-9]+ *){3}", std::regex::optimize);
    //const std::regex FACE_LINE("f +(([0-9]+)(/([0-9]+)?/([0-9]+))? *){3} *", std::regex::optimize);


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

    printf("\rLoading Model: %s [Counting Elements]", path);
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
            break;
        }
        //Speed to the end of the line and begin next iteration
        while ((c = fgetc(file)) != '\n')
        {
            lnLen++;
            if (c == EOF)
                goto exit_loop;
        }
        printf("\rVert: %i:%i, Normal: %i, Face: %i, Tex: %i:%i, Color: %i:%i, Ln:%i", vertices_read, vertices_size, normals_read, faces_read, textures_read, textures_size, colors_read, colors_size, lnLenMax);
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
    t_count = textures_read;
    n_count = normals_read;
    f_count = faces_read;
    //Set instance var sizes
    v_size = vertices_size;//3-4
    t_size = textures_size;//2-3
    c_size = colors_size;//3-4
    //Allocate memory
    allocateModel();
    //Reset file pointer
    clearerr(file);
    fseek(file, 0, SEEK_SET);
    //Reset local counters
    v_count = 0;
    c_count = 0;
    t_count = 0;
    n_count = 0;
    f_count = 0;
    //Allocate temporary buffers for components that may require aligning with relevant vertices
//TODO: Create T_Normal, T_Color, T_Texture
    //Create buffer to read lines of the file into
    unsigned int bufferLen = lnLenMax + 2;
    char *buffer = new char[bufferLen];

    printf("\rLoading Model: %s [Loading Elements]", path);
    //Read file by line, again.
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
//TODO: Read vertex line of file
                break;
                //Normal found, increment count
            case 'n':
//TODO: Read normal line of file
                break;
                //Parameter found, we don't support this but count anyway
            case 'p':
//TODO: Read parameter line of file
                break;
                //Texture found, increment count and check how many components it contains
            case 't':
//TODO: Read texture line of file
                break;
            }
            break;
            //If the first char is 'f', increment face count
        case 'f':
//TODO: Read face line of file
            break;
        }
        //Speed to the end of the line and begin next iteration
        while ((c = fgetc(file)) != '\n')
        {
            lnLen++;
            if (c == EOF)
                goto exit_loop;
        }
    }
    //Cleanup buffer
    delete buffer;

    printf("\rLoading Model: %s [Assigning Elements]", path);

    printf("\rLoading Model: %s [Complete!]\n", path);
    ////Iterate file, reading in vertices, normals & faces
    //while (!feof(file)) {
    //    if (fscanf(file, "%s", buffer) == 1){
    //        if(strcmp(buffer, VERTEX_IDENTIFIER) == 0){
    //            if (fscanf(file, "%f %f %f", &x, &y, &z) == 3){
    //                vertices[vertices_read][0] = x;
    //                vertices[vertices_read][1] = y;
    //                vertices[vertices_read][2] = z;
    //                vertices_read++;
    //            }else{
    //                printf("Incomplete vertex data\n");
    //            }
    //        }else if(strcmp(buffer, VERTEX_NORMAL_IDENTIFIER) == 0){
    //            if (fscanf(file, "%f %f %f", &x, &y, &z) == 3){
    //                t_normals[normals_read][0] = x;
    //                t_normals[normals_read][1] = y;
    //                t_normals[normals_read][2] = z;
    //                normals_read++;
    //            }else{
    //                printf("Incomplete vertex normal data\n");
    //            }
    //        }else if(strcmp(buffer, FACE_IDENTIFIER) == 0){
    //            //expect 3 vertices
    //            if (fscanf(file, "%i//%i %i//%i %i//%i", &f1a, &f1b, &f2a, &f2b, &f3a, &f3b) == 6){
    //                faces[faces_read][0] = f1a-1;
    //                faces[faces_read][1] = f2a-1;
    //                faces[faces_read][2] = f3a-1;
    //                //Copy across all referenced normals
    //                normals[f1a-1][0] = t_normals[f1b-1][0];
    //                normals[f1a-1][1] = t_normals[f1b-1][1];
    //                normals[f1a-1][2] = t_normals[f1b-1][2];

    //                normals[f2a-1][0] = t_normals[f2b-1][0];
    //                normals[f2a-1][1] = t_normals[f2b-1][1];
    //                normals[f2a-1][2] = t_normals[f2b-1][2];

    //                normals[f3a-1][0] = t_normals[f3b-1][0];
    //                normals[f3a-1][1] = t_normals[f3b-1][1];
    //                normals[f3a-1][2] = t_normals[f3b-1][2];
    //                faces_read++;
    //            }else{
    //                printf("Incomplete face data\n");
    //            }    
    //        }
    //        else if (strcmp(buffer, MTLLIB_IDENTIFIER) == 0){
    //            if (fscanf(file, "%s", &materialFile) != 1){
    //                printf("Incomplete material filename");
    //                // reset materialFile
    //                materialFile[0] = '\0';
    //            }
    //        }
    //        else if (strcmp(buffer, USEMTL_IDENTIFEIR) == 0){
    //            if (fscanf(file, "%s", &materialName) != 1){
    //                printf("Incomplete material name");
    //                // reset materialName
    //                materialName[0] = '\0';
    //            }
    //        }
    //    }
    //}
    //if (materialFile[0] != '\0' && materialName != '\0'){
    //    // Materials are in use, attempt to load the material file
    //    loadMaterialFromFile(path, materialFile, materialName);
    //}


    //printf("%s: %i Vertices, %i V-Normals & %i Faces were loaded.\n",path,vertices_read,normals_read,faces_read);
    //if(modelScale>0)
    //{
    //    glm::vec3 minVert = {100000,100000,100000};
    //    glm::vec3 maxVert = {-100000,-100000,-100000};
    //    for(int i=0;i<vertices_read;i++)
    //    {
    //        minVert[0]=(minVert[0]<vertices[i][0])?minVert[0]:vertices[i][0];
    //        maxVert[0]=(maxVert[0]>vertices[i][0])?maxVert[0]:vertices[i][0];
    //    
    //        minVert[1]=(minVert[1]<vertices[i][1])?minVert[1]:vertices[i][1];
    //        maxVert[1]=(maxVert[1]>vertices[i][1])?maxVert[1]:vertices[i][1];

    //        minVert[2]=(minVert[2]<vertices[i][2])?minVert[2]:vertices[i][2];
    //        maxVert[2]=(maxVert[2]>vertices[i][2])?maxVert[2]:vertices[i][2];
    //    }
    //
    //    printf("Original Model Dimensions(%0.4f,%0.4f,%0.4f)",maxVert[0],maxVert[1],maxVert[2]);
    //    //maxVert-=minVert;
    //    maxVert[0]-=minVert[0];    maxVert[1]-=minVert[1];    maxVert[2]-=minVert[2];
    //    float longestAxis = (maxVert[0]>maxVert[1])?maxVert[0]:maxVert[1];
    //    longestAxis = (longestAxis>maxVert[2])?longestAxis:maxVert[2];
    //    printf(", Scaling model by: %0.4f\n",modelScale/longestAxis);
    //    scaleModel(modelScale/longestAxis);
    //}
    //fclose(file);
    //free(t_normals);
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
Allocates the storage for the model's primitives (vertices, normals and faces) according to the instance variables v_count and f_count
@see freeModel()
*/
void Entity::allocateModel(){
    vertices = (float*)malloc(v_count*v_size*sizeof(float));
    if (n_count)//1 normal per vertex
        normals = (glm::vec3*)malloc(v_count*sizeof(glm::vec3));
    if (c_count)//1 color per vertex
        colors = (float*)malloc(v_count*c_size*sizeof(float));
    if (t_count)//1 texture per vertex
        textures = (float*)malloc(v_count*t_size*sizeof(float));
    faces = (glm::ivec3*)malloc(f_count*sizeof(glm::ivec3));
}
/*
Frees the storage for the model's primitives
@see allocateModel()
*/
void Entity::freeModel(){
    if(vertices)
        free(vertices);
    if(faces)
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
