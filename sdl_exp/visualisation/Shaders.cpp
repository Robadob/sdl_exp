//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "Shaders.h"

#include <stdio.h>
#include <stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "Entity.h"

const char *Shaders::MODELVIEW_MATRIX_UNIFORM_NAME = "_modelViewMat";
const char *Shaders::PROJECTION_MATRIX_UNIFORM_NAME = "_projectionMat";
const char *Shaders::MODELVIEWPROJECTION_MATRIX_UNIFORM_NAME = "_modelViewProjectionMat";
const char *Shaders::VERTEX_ATTRIBUTE_NAME = "_vertex";
const char *Shaders::NORMAL_ATTRIBUTE_NAME = "_normal";
const char *Shaders::COLOR_ATTRIBUTE_NAME = "_color";
const char *Shaders::TEXCOORD_ATTRIBUTE_NAME = "_texCoords";
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
	: ShaderCore(0)
	, vertexShaderPath(vertexShaderPath)
    , fragmentShaderPath(fragmentShaderPath)
    , geometryShaderPath(geometryShaderPath)
    , vertexShaderVersion(0)
    , fragmentShaderVersion(0)
    , geometryShaderVersion(0)
    , modelview()
    , projection()
    , modelviewprojection(-1)
    , positions(GL_FLOAT, 3, sizeof(float))
    , normals(GL_FLOAT, NORMALS_SIZE, sizeof(float))
    , colors(GL_FLOAT, 3, sizeof(float))
    , texcoords(GL_FLOAT, 2, sizeof(float))
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
bool Shaders::_compileShaders(const GLuint t_programId)
{
	if (vertexShaderFiles.size()>0)
		if (!compileShader(t_programId, GL_VERTEX_SHADER, vertexShaderFiles))
			return false;

	if (fragmentShaderFiles.size()>0)
		if (!compileShader(t_programId, GL_VERTEX_SHADER, fragmentShaderFiles))
			return false;

	if (geometryShaderFiles.size()>0)
		if (!compileShader(t_programId, GL_VERTEX_SHADER, geometryShaderFiles))
			return false;
	return true;
}
/*
Compiles the shader sources specified at the objects creation
Errors are reported directly to stderr, and will cause compileSuccessFlag to be set to false
Exiting the program on error can be configured by defining EXIT_ON_ERROR
*/
void Shaders::_setupBindings(){
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
	ShaderCore::useProgram();

	//Set the projection matrix (e.g. glFrustum, normally provided by the Visualisation)
	if (this->vertexShaderVersion <= 120 && proj)
	{//If old shaders where gl_ModelViewProjectionMatrix is available
		glMatrixMode(GL_PROJECTION);
		GL_CALL(glLoadMatrixf(glm::value_ptr(*proj)));
	}
	if (this->projection.location >= 0 && proj )
	{//If projection matrix location and camera ptr are known
		GL_CALL(glUniformMatrix4fv(this->projection.location, 1, GL_FALSE, glm::value_ptr(*proj)));
	}
	//Set the model view matrix (e.g. gluLookAt, normally provided by the Camera)
	if (this->vertexShaderVersion <= 120 && mv )
	{//If old shaders where gl_ModelViewMatrix is available
		glMatrixMode(GL_MODELVIEW);
		GL_CALL(glLoadMatrixf(glm::value_ptr(*mv)));
	}
	if (this->modelview.location >= 0 && mv)
	{//If modeview matrix location and camera ptr are known
		GL_CALL(glUniformMatrix4fv(this->modelview.location, 1, GL_FALSE, glm::value_ptr(*mv)));
	}
	//Set the model view projection matrix (e.g. projection * modelview)
	if (this->modelviewprojection >= 0 && mv && proj)
	{
		glm::mat4 mvp = *proj * *mv;
		GL_CALL(glUniformMatrix4fv(this->modelviewprojection, 1, GL_FALSE, glm::value_ptr(mvp)));
	}
}
/*
Used internally by useProgram() fns, to handle shader setup after matrix setup
*/
void Shaders::_useProgram()
{

    //Don't think we need to use (this is for generic vertex attrib buffers)
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
            activeVBO = this->texcoords.vbo;
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
void Shaders::_clearProgram(){
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
    if (this->getProgram()>0 && this->colorUniformLocation >= 0)
    {//If projection matrix location and camera ptr are known
		glUseProgram(this->getProgram());
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