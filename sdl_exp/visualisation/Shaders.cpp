#define  _CRT_SECURE_NO_WARNINGS

//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "Shaders.h"

#include <stdio.h>
#include <stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "Entity.h"

/*
Constructs a shader object from one of the stock shader sets
@param set The shader set to create
*/
Shaders::Shaders(Stock::Shaders::ShaderSet set)
    :Shaders(set.vertex, set.fragment, set.geometry){}
/*
Constructs a shader program from the provided shader files
@param vertexShaderPath Path to the GLSL vertex shader (nullptr if not required)
@param fragmentShaderPath Path to the GLSL fragment shader (nullptr if not required)
@param geometryShaderPath Path to the GLSL geometry shader (nullptr if not required)
@param camera The camera object to pull the modelView and projection matrices from
*/
Shaders::Shaders(const char *vertexShaderPath, const char *fragmentShaderPath, const char *geometryShaderPath)
    : vertexShaderPath(vertexShaderPath)
    , fragmentShaderPath(fragmentShaderPath)
    , geometryShaderPath(geometryShaderPath)
    , vertexShaderVersion(0)
    , fragmentShaderVersion(0)
    , geometryShaderVersion(0)
    , compileSuccessFlag(true)
    , versionRegex("#version ([0-9]+)", std::regex::ECMAScript | std::regex_constants::icase)
    , modelview()
    , projection()
    , modelviewprojection(-1)
    , positions(GL_FLOAT, 3, sizeof(float))
    , normals(GL_FLOAT, NORMALS_SIZE, sizeof(float))
    , colors(GL_FLOAT, 3, sizeof(float))
    , texcoords(GL_FLOAT, 2, sizeof(float))
    , textures()
    , programId(0)
{
    GL_CHECK();
    this->createShaders();
}
/*
Destructor, free's the shader program
*/
Shaders::~Shaders(){
    this->destroyProgram();
}
/*
Returns whether a vertex shader has been loaded
@return True if a vertex shader is present, else false
*/
bool Shaders::hasVertexShader() const
{
    return this->vertexShaderPath!=0;
}
/*
Returns whether a fragment shader has been loaded
@return True if a fragment shader is present, else false
*/
bool Shaders::hasFragmentShader() const
{
    return this->fragmentShaderPath != 0;
}
/*
Returns whether a geometry shader has been loaded
@return True if a geometry shader is present, else false
*/
bool Shaders::hasGeometryShader() const{
    return this->geometryShaderPath != 0;
}
/*
Returns whether the last call to createShaders() (via object creation or a call to reload()) was successful
@return True if the currently loaded shaders match the last loaded shaders
*/
bool Shaders::getCompileSuccess() const{
    return this->compileSuccessFlag;
}
/*
Compiles the shader sources specified at the objects creation
Errors are reported directly to stderr, and will cause compileSuccessFlag to be set to false
Exiting the program on error can be configured by defining EXIT_ON_ERROR
*/
void Shaders::createShaders(){
    // Reset the flag
    this->compileSuccessFlag = true;
    // Load shader files
    const char* vertexSource = loadShaderSource(this->vertexShaderPath);
    const char* fragmentSource = loadShaderSource(this->fragmentShaderPath);
    const char* geometrySource = loadShaderSource(this->geometryShaderPath);
    //Check for shaders that didn't load correctly
    if (vertexSource == 0) this->vertexShaderPath = 0;
    if (fragmentSource == 0) this->fragmentShaderPath = 0;
    if (geometrySource == 0) this->geometryShaderPath = 0;

    // For each shader we have been able to read.
    // Create the empty shader handle
    // Attempt to compile the shader
    // Check compilation
    if (hasVertexShader()){
        this->vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        GL_CALL(glShaderSource(this->vertexShaderId, 1, &vertexSource, 0));
        GL_CALL(glCompileShader(this->vertexShaderId));
        this->checkShaderCompileError(this->vertexShaderId, this->vertexShaderPath);
    }
    else
    {
        this->vertexShaderId = -1;        
    }
    if (hasFragmentShader()){
        this->fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        GL_CALL(glShaderSource(this->fragmentShaderId, 1, &fragmentSource, 0));
        GL_CALL(glCompileShader(this->fragmentShaderId));
        this->checkShaderCompileError(this->fragmentShaderId, this->fragmentShaderPath);
    }
    else
    {
        this->fragmentShaderId = -1;        
    }
    if (hasGeometryShader()){
        this->geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER);
        GL_CALL(glShaderSource(this->geometryShaderId, 1, &geometrySource, 0));
        GL_CALL(glCompileShader(this->geometryShaderId));
        this->checkShaderCompileError(this->geometryShaderId, this->geometryShaderPath);
    }
    else
    {
        this->geometryShaderId = -1;        
    }
    // Only attempt to link the program if the compilation of each individual shader was successful.
    if (this->compileSuccessFlag){

        // Create the program
        int newProgramId = GL_CALL(glCreateProgram());

        // Attach each included shader
        if (this->hasVertexShader())
            GL_CALL(glAttachShader(newProgramId, this->vertexShaderId));
        if (this->hasFragmentShader())
            GL_CALL(glAttachShader(newProgramId, this->fragmentShaderId));
        if (this->hasGeometryShader())
            GL_CALL(glAttachShader(newProgramId, this->geometryShaderId));
        
        // Link the program and Ensure the program compiled correctly;
        GL_CALL(glLinkProgram(newProgramId));

        this->checkProgramCompileError(newProgramId);
        // If the program compiled ok, then we update the instance variable (for live reloading)
        if (this->compileSuccessFlag){
            // Destroy the old program
            this->destroyProgram();
            // Update the class var for the next usage.
            this->programId = newProgramId;
        }
    }

    //Compilation was successful, lets try and detect how to configure the shader
    if (this->compileSuccessFlag)
    {
        //Detect shader version numbers
        if (this->hasVertexShader())
            this->vertexShaderVersion = this->findShaderVersion(vertexSource);
        if (this->hasFragmentShader())
            this->fragmentShaderVersion = this->findShaderVersion(fragmentSource);
        if (this->hasGeometryShader())
            this->geometryShaderVersion = this->findShaderVersion(geometrySource);

        //Locate the modelView matrix uniform
        std::pair<int, GLenum> u_MV = findUniform(MODELVIEW_MATRIX_UNIFORM_NAME, this->programId);
        if (u_MV.first >= 0 && u_MV.second == GL_FLOAT_MAT4)
            this->modelview.location = u_MV.first;
        else
            this->modelview.location = -1;
        //Locate the projection matrix uniform
        std::pair<int, GLenum> u_PM = findUniform(PROJECTION_MATRIX_UNIFORM_NAME, this->programId);
        if (u_PM.first >= 0 && u_PM.second == GL_FLOAT_MAT4)
            this->projection.location = u_PM.first;
        else
            this->projection.location = -1;
        //Locate the modelViewProjection matrix uniform
        std::pair<int, GLenum> u_MVP = findUniform(MODELVIEWPROJECTION_MATRIX_UNIFORM_NAME, this->programId);
        if (u_MVP.first >= 0 && u_MVP.second == GL_FLOAT_MAT4)
            this->modelviewprojection = u_MVP.first;
        else
            this->modelviewprojection = -1;
        //Locate the vertexPosition attribute
        std::pair<int, GLenum> a_V = findAttribute(VERTEX_ATTRIBUTE_NAME, this->programId);
        if (a_V.first >= 0 && (a_V.second == GL_FLOAT_VEC3 || a_V.second == GL_FLOAT_VEC4))
            this->positions.location = a_V.first;
        else
            this->positions.location = -1;
        //Locate the vertexNormal attribute
        std::pair<int, GLenum> a_N = findAttribute(NORMAL_ATTRIBUTE_NAME, this->programId);
        if (a_N.first >= 0 && (a_N.second == GL_FLOAT_VEC3 || a_N.second == GL_FLOAT_VEC4))
            this->normals.location = a_N.first;
        else
            this->normals.location = -1;
        //Locate the vertexColor attribute
        std::pair<int, GLenum> a_C = findAttribute(COLOR_ATTRIBUTE_NAME, this->programId);
        if (a_C.first >= 0 && (a_C.second == GL_FLOAT_VEC3 || a_C.second == GL_FLOAT_VEC4))
            this->colors.location = a_C.first;
        else
            this->colors.location = -1;
        //Locate the vertexTexCoords attribute
        std::pair<int, GLenum> a_T = findAttribute(TEXCOORD_ATTRIBUTE_NAME, this->programId);
        if (a_T.first >= 0 && (a_T.second == GL_FLOAT_VEC2 || a_T.second == GL_FLOAT_VEC3))
            this->texcoords.location = a_T.first;
        else
            this->texcoords.location = -1;
        //Locate the color uniform   
        std::pair<int, GLenum> u_C = findUniform(COLOR_ATTRIBUTE_NAME, this->programId);
        if (u_C.first >= 0 && (u_C.second == GL_FLOAT_VEC3 || u_C.second == GL_FLOAT_VEC4))
            this->colorUniformLocation = u_C.first;
        else
            this->colorUniformLocation = -1;
        this->colorUniformSize = u_C.second == GL_FLOAT_VEC3 ? 3 : 4;

        //Refresh dynamic uniforms
        std::map<GLint, DynamicUniformDetail> t_dynamicUniforms;
        for (std::map<GLint, DynamicUniformDetail>::iterator i = dynamicUniforms.begin(); i != dynamicUniforms.end(); i++)
        {
            DynamicUniformDetail d = i->second;
            GLint location = GL_CALL(glGetUniformLocation(this->programId, i->second.uniformName));
            if (location != -1)
                t_dynamicUniforms[location] = d;
            else//If the uniform isn't found again, assign it to a semi random location, in hopes of it not being lost from the map
            {
                lostDynamicUniforms.push_front(d);
                printf("Dynamic uniform '%s' could not be located on shader reload.\n", d.uniformName);
            }
        }
        //Check whether lost uniforms have reappeared
        for (std::list<DynamicUniformDetail>::iterator i = lostDynamicUniforms.begin(); i != lostDynamicUniforms.end(); i++)
        {
            GLint location = GL_CALL(glGetUniformLocation(this->programId, i->uniformName));
            if (location != -1)
            {
                t_dynamicUniforms[location] = *i;
                lostDynamicUniforms.erase(i);
            }
            else//If the uniform isn't found again, remind the user
            {
                printf("Dynamic uniform '%s' remains not located on shader reload.\n", i->uniformName);
            }
        }
        //Refresh static uniforms
        glUseProgram(this->programId);
        for (std::forward_list<StaticUniformDetail>::iterator i = staticUniforms.begin(); i != staticUniforms.end(); i++)
        {
            GLint location = GL_CALL(glGetUniformLocation(this->programId, i->uniformName));
            if (location != -1)
            {
                if (i->type == GL_FLOAT)
                {
                    if (sizeof(int) != sizeof(float))
                        printf("Error: int and float sizes differ, static float uniforms may be corrupted.\n");
                    if (i->count == 1){
                        GL_CALL(glUniform1fv(location, 1, reinterpret_cast<GLfloat *>(glm::value_ptr(i->data))));
                    }
                    else if (i->count == 2){
                        GL_CALL(glUniform2fv(location, 1, reinterpret_cast<GLfloat *>(glm::value_ptr(i->data))));
                    }
                    else if (i->count == 3){
                        GL_CALL(glUniform3fv(location, 1, reinterpret_cast<GLfloat *>(glm::value_ptr(i->data))));
                    }
                    else if (i->count == 4){
                        GL_CALL(glUniform4fv(location, 1, reinterpret_cast<GLfloat *>(glm::value_ptr(i->data))));
                    }
                }
                else if (i->type == GL_INT)
                {
                    if (i->count == 1){
                        GL_CALL(glUniform1iv(location, 1, glm::value_ptr(i->data)));
                    }
                    else if (i->count == 2){
                        GL_CALL(glUniform2iv(location, 1, glm::value_ptr(i->data)));
                    }
                    else if (i->count == 3){
                        GL_CALL(glUniform3iv(location, 1, glm::value_ptr(i->data)));
                    }
                    else if (i->count == 4){
                        GL_CALL(glUniform4iv(location, 1, glm::value_ptr(i->data)));
                    }
                }
            }
            else//If the uniform isn't found again, remind the user
            {
                printf("Static uniform '%s' could not located on shader reload.\n", i->uniformName);
            }
        }
        glUseProgram(0);
    }

    // Clean up any shaders
    this->destroyShaders();
    // and delete sources
    delete vertexSource;
    delete fragmentSource;
    delete geometrySource;
}
/*
Reloads the shaders live, according to the file paths provided at the objects creation
If shader creation fails, the existing shaders will continue to be used
@param silent When false 'Reloading shaders' will be printed to console
@return True when the new shaders were loaded successfully
*/
bool Shaders::reload(bool silent){
    if (!silent)
        printf("Reloading Shaders\n");
    this->createShaders();
    return this->compileSuccessFlag;
}
/*
Call this prior to rendering to enable the program and automatically bind known uniforms and attributes
@param e If an entity is passed, their location and rotation values will be applied to the ModelView matrix before it is loaded
*/
void Shaders::useProgram(Entity *e){
	GL_CALL(glUseProgram(this->programId));

	//glPushAttrib(GL_ALL_ATTRIB_BITS)? To shortern clearProgram?

	//Set the projection matrix (e.g. glFrustum, normally provided by the Visualisation)
	if (this->vertexShaderVersion <= 120 && this->projection.matrixPtr > 0)
	{//If old shaders where gl_ModelViewProjectionMatrix is available
		glMatrixMode(GL_PROJECTION);
		GL_CALL(glLoadMatrixf(glm::value_ptr(*this->projection.matrixPtr)));
	}
	if (this->projection.location >= 0 && this->projection.matrixPtr > 0)
	{//If projection matrix location and camera ptr are known
		GL_CALL(glUniformMatrix4fv(this->projection.location, 1, GL_FALSE, glm::value_ptr(*this->projection.matrixPtr)));
	}
	//Calculate modelview matrix
	glm::mat4 mv;
	if (this->modelview.matrixPtr)
	{
		mv = *this->modelview.matrixPtr;
		if (e)
		{
			glm::vec4 rot = e->getRotation();
			glm::vec3 tran = e->getLocation();
			if ((rot.x != 0 || rot.y != 0 || rot.z != 0) && rot.w != 0)
				mv = glm::rotate(mv, glm::radians(rot.w), glm::vec3(rot.x, rot.y, rot.z));
			mv = glm::translate(mv, tran);
		}
	}
	//Set the model view matrix (e.g. gluLookAt, normally provided by the Camera)
	if (this->vertexShaderVersion <= 120 && this->modelview.matrixPtr > 0)
	{//If old shaders where gl_ModelViewMatrix is available
		glMatrixMode(GL_MODELVIEW);
		GL_CALL(glLoadMatrixf(glm::value_ptr(mv)));
	}
	if (this->modelview.location >= 0 && this->modelview.matrixPtr > 0)
	{//If modeview matrix location and camera ptr are known
		GL_CALL(glUniformMatrix4fv(this->modelview.location, 1, GL_FALSE, glm::value_ptr(mv)));
	}
	//Set the model view projection matrix (e.g. projection * modelview)
	if (this->modelviewprojection >= 0 && this->modelview.matrixPtr && this->projection.matrixPtr)
	{
		glm::mat4 mvp = *this->projection.matrixPtr * mv;
		GL_CALL(glUniformMatrix4fv(this->modelviewprojection, 1, GL_FALSE, glm::value_ptr(mvp)));
	}
	_useProgram();
}
/*
Call to setup shader with external modelview and projection matrices
@note Used by Overlay/HUD
*/
void Shaders::useProgram(const glm::mat4 *mv, const glm::mat4 *proj)
{
	GL_CALL(glUseProgram(this->programId));

	//glPushAttrib(GL_ALL_ATTRIB_BITS)? To shortern clearProgram?

	//Set the projection matrix (e.g. glFrustum, normally provided by the Visualisation)
	if (this->vertexShaderVersion <= 120 && proj > 0)
	{//If old shaders where gl_ModelViewProjectionMatrix is available
		glMatrixMode(GL_PROJECTION);
		GL_CALL(glLoadMatrixf(glm::value_ptr(*proj)));
	}
	if (this->projection.location >= 0 && proj > 0)
	{//If projection matrix location and camera ptr are known
		GL_CALL(glUniformMatrix4fv(this->projection.location, 1, GL_FALSE, glm::value_ptr(*proj)));
	}
	//Set the model view matrix (e.g. gluLookAt, normally provided by the Camera)
	if (this->vertexShaderVersion <= 120 && mv > 0)
	{//If old shaders where gl_ModelViewMatrix is available
		glMatrixMode(GL_MODELVIEW);
		GL_CALL(glLoadMatrixf(glm::value_ptr(*mv)));
	}
	if (this->modelview.location >= 0 && mv > 0)
	{//If modeview matrix location and camera ptr are known
		GL_CALL(glUniformMatrix4fv(this->modelview.location, 1, GL_FALSE, glm::value_ptr(*mv)));
	}
	//Set the model view projection matrix (e.g. projection * modelview)
	if (this->modelviewprojection >= 0 && mv && proj)
	{
		glm::mat4 mvp = *proj * *mv;
		GL_CALL(glUniformMatrix4fv(this->modelviewprojection, 1, GL_FALSE, glm::value_ptr(mvp)));
	}
	_useProgram();
}
/*
Used internally by useProgram() fns, to handle shader setup after matrix setup
*/
void Shaders::_useProgram()
{

    //Don't think we need to use
    //GL_CALL(glBindAttribLocation(this->programId, 0, "in_position"));

    GLuint activeVBO = 0;
    //Set the vertex (location) attribute
    if (this->vertexShaderVersion <= 120 && this->positions.vbo > 0)
    {//If old shaders where gl_Vertex is available
        glEnableClientState(GL_VERTEX_ARRAY);
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->positions.vbo));
        glVertexPointer(this->positions.components, this->positions.componentType, this->positions.stride, ((char *)NULL + this->positions.offset));
        activeVBO = this->positions.vbo;
    }
    if (this->positions.location >= 0 && this->positions.vbo > 0)
    {//If vertex attribute location and vbo are known
        glEnableVertexAttribArray(this->positions.location);
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->positions.vbo));
        GL_CALL(glVertexAttribPointer(this->positions.location, this->positions.components, this->positions.componentType, GL_FALSE, this->positions.stride, ((char *)NULL + this->positions.offset)));
        activeVBO = this->positions.vbo;
    }

    //Set the vertex normal attribute
    if (this->vertexShaderVersion <= 120 && this->normals.vbo > 0)
    {//If old shaders where gl_Vertex is available
        glEnableClientState(GL_NORMAL_ARRAY);
        if (activeVBO != this->normals.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->normals.vbo));
            activeVBO = this->normals.vbo;
        }
        glNormalPointer(this->normals.componentType, this->normals.stride, ((char *)NULL + this->normals.offset));
    }
    if (this->normals.location >= 0 && this->normals.vbo > 0)
    {//If vertex attribute location and vbo are known
        glEnableVertexAttribArray(this->normals.location);
        if (activeVBO != this->normals.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->normals.vbo));
            activeVBO = this->normals.vbo;
        }
        GL_CALL(glVertexAttribPointer(this->normals.location, this->normals.components, this->positions.componentType, GL_FALSE, this->normals.stride, ((char *)NULL + this->normals.offset)));
    }

    //Set the vertex color attributes
    if (this->vertexShaderVersion <= 120 && this->colors.vbo > 0)
    {//If old shaders where gl_Color is available
        glEnableClientState(GL_COLOR_ARRAY);
        if (activeVBO != this->colors.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->colors.vbo));
            activeVBO = this->colors.vbo;
        }
        glColorPointer(this->colors.components, this->colors.componentType, this->colors.stride, ((char *)NULL + this->colors.offset));
    }
    if (this->colors.location >= 0 && this->colors.vbo > 0)
    {//If color attribute location and vbo are known
        glEnableVertexAttribArray(this->colors.location);
        if (activeVBO != this->colors.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->colors.vbo));
            activeVBO = this->colors.vbo;
        }
        GL_CALL(glVertexAttribPointer(this->colors.location, this->colors.components, this->colors.componentType, GL_FALSE, this->colors.stride, ((char *)NULL + this->colors.offset)));
    }

    //Set the vertex texture coord attributes
    if (this->vertexShaderVersion <= 140 && this->texcoords.vbo > 0)
    {//If old shaders where gl_TexCoord is available
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        if (activeVBO != this->texcoords.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->texcoords.vbo));
            activeVBO = this->texcoords.vbo;
        }
        //glColorPointer(this->texcoords.components, this->texcoords.componentType, this->texcoords.stride, ((char *)NULL + this->texcoords.offset));
        glTexCoordPointer(this->texcoords.components, this->texcoords.componentType, this->texcoords.stride, ((char *)NULL + this->texcoords.offset));
    }
    if (this->texcoords.location >= 0 && this->texcoords.vbo > 0)
    {//If texture attribute location and vbo are known
        glEnableVertexAttribArray(this->texcoords.location);
        if (activeVBO != this->texcoords.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->texcoords.vbo));
            //activeVBO = this->texcoords.vbo;
        }
        GL_CALL(glVertexAttribPointer(this->texcoords.location, this->texcoords.components, this->texcoords.componentType, GL_FALSE, this->texcoords.stride, ((char *)NULL + this->texcoords.offset)));
    }

    //Set any Texture buffers
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + textures[i].bufferId);
        glBindTexture(textures[i].type, textures[i].name);
    }

    //Set any dynamic uniforms
    for (std::map<GLint, DynamicUniformDetail>::iterator i = dynamicUniforms.begin(); i != dynamicUniforms.end(); i++)
    {
        if (i->second.type == GL_FLOAT)
        {
            if (i->second.count == 1)
            {
                GL_CALL(glUniform1fv(i->first, 1, (GLfloat *)i->second.data));
            }
            else if (i->second.count == 2){
                GL_CALL(glUniform2fv(i->first, 1, (GLfloat *)i->second.data));
            }
            else if (i->second.count == 3){
                GL_CALL(glUniform3fv(i->first, 1, (GLfloat *)i->second.data));
            }
            else if (i->second.count == 4){
                GL_CALL(glUniform4fv(i->first, 1, (GLfloat *)i->second.data));
            }
        }
        else if (i->second.type == GL_INT)
        {
            if (i->second.count == 1){
                GL_CALL(glUniform1iv(i->first, 1, (GLint *)i->second.data));
            }
            else if (i->second.count == 2){
                GL_CALL(glUniform2iv(i->first, 1, (GLint *)i->second.data));
            }
            else if (i->second.count == 3){
                GL_CALL(glUniform3iv(i->first, 1, (GLint *)i->second.data));
            }
            else if (i->second.count == 4){
                GL_CALL(glUniform4iv(i->first, 1, (GLint *)i->second.data));
            }
        }
    }
}


/*
Disables the currently active shader progam and attributes attached to this shader
*/
void Shaders::clearProgram(){
    //Massively shorten this with glPopAttrib()?
    //Vertex location
    if (this->vertexShaderVersion <= 140 && this->positions.vbo > 0)
    {//If old shaders where gl_Vertex is available
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    if (this->positions.location >= 0 && this->positions.vbo > 0)
    {//If vertex attribute location and vbo are known
        glDisableVertexAttribArray(this->positions.location);
    }
    //Vertex normal
    if (this->vertexShaderVersion <= 140 && this->normals.vbo > 0)
    {//If old shaders where gl_Normal is available
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    if (this->normals.location >= 0 && this->normals.vbo > 0)
    {//If vertex attribute location and vbo are known
        glDisableVertexAttribArray(this->normals.location);
    }
    //Vertex color
    if (this->vertexShaderVersion <= 140 && this->colors.vbo > 0)
    {//If old shaders where gl_Color is available
        glDisableClientState(GL_COLOR_ARRAY);
    }
    if (this->colors.location >= 0 && this->colors.vbo > 0)
    {//If vertex attribute location and vbo are known
        glDisableVertexAttribArray(this->colors.location);
    }
    //Vertex color
    if (this->vertexShaderVersion <= 140 && this->texcoords.vbo > 0)
    {//If old shaders where gl_Color is available
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (this->texcoords.location >= 0 && this->texcoords.vbo > 0)
    {//If vertex attribute location and vbo are known
        glDisableVertexAttribArray(this->texcoords.location);
    }
    GL_CALL(glUseProgram(0));
}
/*
Loads the text from the provided filepath
@return A pointer to the loaded shader source
@note the returned pointer is allocated via malloc, and should be free'd when nolonger required
*/
char* Shaders::loadShaderSource(const char* file){
    // If file path is 0 it is being omitted. kinda gross
    if (file != 0){
        FILE* fptr = fopen(file, "rb");
        if (!fptr){
            fprintf(stderr,"Shader not found: %s\n", file);
#ifdef EXIT_ON_ERROR
            getchar();
            exit(1);
#endif
            return 0;
        }
        fseek(fptr, 0, SEEK_END);
        long length = ftell(fptr);
        char* buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
        fseek(fptr, 0, SEEK_SET);
        fread(buf, length, 1, fptr);
        fclose(fptr);
        buf[length] = '\0'; // Null terminator
        return buf;
    }
    else {
        return 0;
    }
}
/*
Destroys the compiled shaders
This is called after the shader program has been linked (not in the destructor), they are nolonger required after that point
*/
void Shaders::destroyShaders(){
    if (this->hasVertexShader())
        GL_CALL(glDeleteShader(this->vertexShaderId));

    if (this->hasFragmentShader())
        GL_CALL(glDeleteShader(this->fragmentShaderId));

    if (this->hasGeometryShader())
        GL_CALL(glDeleteShader(this->geometryShaderId));
}
/*
Destroys the shader program
*/
void Shaders::destroyProgram(){
    GL_CALL(glDeleteProgram(this->programId));
}
/*
Checks whether the specified shader compiled succesfully.
Compilation errors are printed to stderr and compileSuccessflag is set to false on failure.
@param shaderId Location of the shader to check
@param shaderPath Path to the shader being checked (so that it can be easily identified in the error log)
@return True if no errors were detected
*/
bool Shaders::checkShaderCompileError(int shaderId, const char* shaderPath){
    GL_CHECK();
    GLint status;
    GL_CALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status));
    if (status == GL_FALSE){
        // Get the length of the info log
        GLint len;
        GL_CALL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &len));
        // Get the contents of the log message
        char* log = new char[len + 1];
        GL_CALL(glGetShaderInfoLog(shaderId, len, &len, log));
        // Print the message
        fprintf(stderr, "Shader compilation error (%s) :\n", shaderPath);
        fprintf(stderr, "%s\n", log);
        delete log;
        this->compileSuccessFlag = false;
#ifdef EXIT_ON_ERROR
        getchar();
        exit(1);
#endif
        return false;
    }
    return true;
}
/*
Checks whether the specified shader program linked succesfully.
Linking errors are printed to stderr and compileSuccessflag is set to false on failure.
@param programId Location of the shader program to check
@return True if no errors were detected
@note For some reason program compilation failure logs don't seem to work (the same as shader compilation)
*/
bool Shaders::checkProgramCompileError(int programId){
    GL_CHECK();
    GLint status;
    GL_CALL(glGetProgramiv(programId, GL_LINK_STATUS, &status));
    if (status == GL_FALSE){
        // Get the length of the info log
        GLint len = 0;
        GL_CALL(glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &len));
        // Get the contents of the log message
        char* log = new char[len+1];
        GL_CALL(glGetProgramInfoLog(programId, len, 0, log));
        // Print the message
        fprintf(stderr, "Program compilation error:\n");
        fprintf(stderr, "%s\n", log);
        this->compileSuccessFlag = false;
#if EXIT_ON_ERROR
        getchar();
        exit(1);
#endif
        return false;
    }
    return true;
}
/*
Looks for the '#version xx' tag in the provided shader source and returns the numeric value
@param shaderSource The shader code to detect the version from
@return The detected shader version, 0 if one was not found
*/
unsigned int Shaders::findShaderVersion(const char *shaderSource)
{
    std::cmatch match;
    //\#version ([0-9]+)\ <-versionRegex
    if (std::regex_search(shaderSource, match, this->versionRegex))
        return stoul(match[1]);
    return 0;
}
/*
Attempts to locate the speicified uniform's location and type
@param uniformName The name of the uniform
@param shaderProgram The programId of the shader
@return A pair object whereby the first item is the uniform'd location, and the second item is the type. On failure the first item will be -1
@note Type can be any enum from: GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_SAMPLER_2D, or GL_SAMPLER_CUBE
*/
std::pair<int, GLenum> Shaders::findUniform(const char *uniformName, const int shaderProgram)
{
    int result = GL_CALL(glGetUniformLocation(shaderProgram, uniformName));
    if (result > -1)
    {
        GLenum type;
        GLint size;//Collect size, because its not documented that you can pass 0
        GL_CALL(glGetActiveUniform(shaderProgram, result, 0, 0, &size, &type, 0));
        return std::pair<int, GLenum>(result, type);
    }
    return  std::pair<int, GLenum>(-1, 0);
}
/*
Attempts to locate the specified attribute's location and type
@param attributeName The name of the attribute
@param shaderProgram The programId of the shader
@return A pair object whereby the first item is the attribute's location, and the second item is the type. On failure the first item will be -1
@note Type can be any enum from: GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_SAMPLER_2D, or GL_SAMPLER_CUBE
*/
std::pair<int, GLenum> Shaders::findAttribute(const char *attributeName, const int shaderProgram)
{
    int result = GL_CALL(glGetAttribLocation(shaderProgram, attributeName));
    if (result > -1)
    {
        GLenum type;
        GLint size;//Collect size, because its not documented that you can pass 0
        GL_CALL(glGetActiveAttrib(shaderProgram, result, 0, 0, &size, &type, 0));
        return std::pair<int, GLenum>(result, type);
    }
    return  std::pair<int, GLenum>(-1, 0);
}
/*
Sets the pointer from which the ModelView matrix should be loaded from
This pointer is likely provided by the Camera object
*/
void Shaders::setModelViewMatPtr(glm::mat4 const *modelViewMat){
    this->modelview.matrixPtr = modelViewMat;
}
/*
Sets the pointer from which the Projection matrix should be loaded from
This pointer is likely provided by the Visualisation object
*/
void Shaders::setProjectionMatPtr(glm::mat4 const *projectionMat){
    this->projection.matrixPtr = projectionMat;
}
/*
Stores the details necessary for passing vertex position attributes to the shader via the modern method
@param vad The VertexAttributeDetail object containing the attribute data
*/
void Shaders::setPositionsAttributeDetail(VertexAttributeDetail vad)
{
    vad.location = this->positions.location;
    this->positions = vad;
}
/*
Stores the details necessary for passing vertex normal attributes
@param vad The VertexAttributeDetail object containing the attribute data
*/
void Shaders::setNormalsAttributeDetail(VertexAttributeDetail vad)
{
    vad.location = this->normals.location;
    this->normals = vad;
}
/*
Stores the details necessary for passing vertex color attributes to the shader
@param vad The VertexAttributeDetail object containing the attribute data
*/
void Shaders::setColorsAttributeDetail(VertexAttributeDetail vad)
{
    vad.location = this->colors.location;
    this->colors = vad;
}
/*
Stores the details necessary for passing vertex texture attributes to the shader
@param vad The VertexAttributeDetail object containing the attribute data
*/
void Shaders::setTexCoordsAttributeDetail(VertexAttributeDetail vad)
{
    vad.location = this->texcoords.location;
    this->texcoords = vad;
}
/*
Permenently binds a texture buffer to be loaded when useProgram() is called
@param texture The name of the texture (as returned by glGenTexture())
@param uniformName The name of the uniform within the shader this texture should be bound to
@param type The type of texture being bound
@return The texture unit the texture has been bound to, on failure (due to no texture units remaining) -1
*/
int Shaders::addTextureUniform(GLuint texture, char *uniformName, GLenum type)
{
    GLint bufferId = textures.size();
    GLint maxTex;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTex);
    if (bufferId<maxTex && addStaticUniform(uniformName, &bufferId))
    {
        textures.push_back(UniformTextureDetail{ texture, bufferId, type });
        return bufferId;
    }
    return -1;
}
/*
Remembers a pointer to an array of upto 4 integers that will be updated everytime useProgram() is called on this Shaders object
@param uniformName The name of the uniform within the shader
@param array A pointer to the array of integers
@param count The number of integers provided in the array (a maximum of 4)
@returns false if the uniform name was not found or count is outside of the inclusive range 1-4
@note Even when false is returned, the value will be stored for reprocessing on shader reloads
*/
bool Shaders::addDynamicUniform(char *uniformName, GLint *array, unsigned int count)
{
    if (this->programId > 0 && count > 0 && count <= 4)
    {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
        if (location != -1)
        {
            dynamicUniforms[location] = DynamicUniformDetail{ GL_INT, (void*)array, count, uniformName };
            return true;
        }
        else
        {
            fprintf(stderr, "Dynamic uniform named: %s was not found in shaders '%s' etc.\n", uniformName, this->vertexShaderPath ? this->vertexShaderPath : (this->fragmentShaderPath ? this->fragmentShaderPath : ""));
        }
    }
    return false;
}
/*
Remembers a pointer to an array of upto 4 floats that will be updated everytime useProgram() is called on this Shaders object
@param uniformName The name of the uniform within the shader
@param array A pointer to the array of floats
@param count The number of floats provided in the array (a maximum of 4)
@returns false if the uniform name was not found or count is outside of the inclusive range 1-4
@note Even when false is returned, the value will be stored for reprocessing on shader reloads
*/
bool Shaders::addDynamicUniform(char *uniformName, GLfloat *array, unsigned int count)
{
    if (this->programId > 0 && count > 0 && count <= 4)
    {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
        if (location != -1)
        {
            dynamicUniforms[location] = DynamicUniformDetail{ GL_FLOAT, (void*)array, count, uniformName };
            return true;
        }
        else
        {
            fprintf(stderr, "Dynamic uniform named: %s was not found in shaders '%s' etc.\n", uniformName, this->vertexShaderPath ? this->vertexShaderPath : (this->fragmentShaderPath ? this->fragmentShaderPath : ""));
        }
    }
    return false;
}
/*
Sets the
Remembers a pointer to an array of upto 4 floats that will be updated everytime useProgram() is called on this Shaders object
@param uniformName The name of the uniform within the shader
@param array A pointer to the array of floats
@param count The number of floats provided in the array (a maximum of 4)
@returns false if the uniform name was not found or count is outside of the inclusive range 1-4
@note Even when false is returned, the value will be stored for reprocessing on shader reloads
*/
bool Shaders::addStaticUniform(char *uniformName, GLfloat *array, unsigned int count)
{
    staticUniforms.push_front(StaticUniformDetail{ GL_FLOAT, *(glm::ivec4 *)array, count, uniformName });
    if (this->programId > 0 && count > 0 && count <= 4)
    {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
        if (location != -1)
        {
            glUseProgram(this->programId);
            if (count == 1){
                GL_CALL(glUniform1fv(location, 1, array));
            }
            else if (count == 2){
                GL_CALL(glUniform2fv(location, 1, array));
            }
            else if (count == 3){
                GL_CALL(glUniform3fv(location, 1, array));
            }
            else if (count == 4){
                GL_CALL(glUniform4fv(location, 1, array));
            }
            glUseProgram(0);
            return true;
        }
        else
        {
            fprintf(stderr, "Static uniform named: %s was not found in shaders '%s' etc.\n", uniformName, this->vertexShaderPath ? this->vertexShaderPath : (this->fragmentShaderPath ? this->fragmentShaderPath : ""));
        }
    }
    return false;
}
/*
Remembers a pointer to an array of upto 4 integers that will be updated everytime useProgram() is called on this Shaders object
@param uniformName The name of the uniform within the shader
@param array A pointer to the array of integers
@param count The number of integers provided in the array (a maximum of 4)
@returns false if the uniform name was not found or count is outside of the inclusive range 1-4
@note Even when false is returned, the value will be stored for reprocessing on shader reloads
*/
bool Shaders::addStaticUniform(char *uniformName, GLint *array, unsigned int count)
{
    staticUniforms.push_front(StaticUniformDetail{ GL_INT, (glm::ivec4)*array, count, uniformName });
    if (this->programId > 0 && count > 0 && count <= 4)
    {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
        if (location != -1)
        {
            glUseProgram(this->programId);
            if (count == 1){
                GL_CALL(glUniform1iv(location, 1, array));
            }
            else if (count == 2){
                GL_CALL(glUniform2iv(location, 1, array));
            }
            else if (count == 3){
                GL_CALL(glUniform3iv(location, 1, array));
            }
            else if (count == 4){
                GL_CALL(glUniform4iv(location, 1, array));
            }
            glUseProgram(0);
            return true;
        }
        else
        {
            fprintf(stderr, "Static uniform named: %s was not found in shaders '%s' etc.\n", uniformName, this->vertexShaderPath ? this->vertexShaderPath : (this->fragmentShaderPath ? this->fragmentShaderPath:""));
        }
    }
    return false;
}
/*
Sets the color uniform
@param color The RGB value of the color
*/
void Shaders::setColor(glm::vec3 color)
{
    setColor(glm::vec4(color, 1.0f));
}
/*
Sets the color uniform
@param color The RGBA value of the color
*/
void Shaders::setColor(glm::vec4 color)
{
    //Set the color uniform is present
    if (this->programId>0 && this->colorUniformLocation >= 0)
    {//If projection matrix location and camera ptr are known
        glUseProgram(this->programId);
        if (this->colorUniformSize == 3)
        {
            GL_CALL(glUniform3fv(this->colorUniformLocation, 1, glm::value_ptr(color)));
        }
        else
        {
            GL_CALL(glUniform4fv(this->colorUniformLocation, 1, glm::value_ptr(color)));
        }
        glUseProgram(0);
    }
}