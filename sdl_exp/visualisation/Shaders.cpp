#include "Shaders.h"

#include <stdio.h>
#include <stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

const char *Shaders::MODELVIEW_MATRIX_UNIFORM_NAME = "_modelViewMat";
const char *Shaders::PROJECTION_MATRIX_UNIFORM_NAME = "_projectionMat";
const char *Shaders::MODELVIEWPROJECTION_MATRIX_UNIFORM_NAME = "_modelViewProjectionMat";
const char *Shaders::VERTEX_ATTRIBUTE_NAME = "_vertex";
const char *Shaders::NORMAL_ATTRIBUTE_NAME = "_normal";
const char *Shaders::COLOR_ATTRIBUTE_NAME = "_color";
const char *Shaders::TEXCOORD_ATTRIBUTE_NAME = "_texCoords";

Shaders::Shaders(Stock::Shaders::ShaderSet set)
    :Shaders(set.vertex, set.fragment, set.geometry){}
Shaders::Shaders(const char *vertexShaderPath, const char *fragmentShaderPath, const char *geometryShaderPath)
	: Shaders(
		vertexShaderPath ? std::initializer_list <const char *>{vertexShaderPath } : std::initializer_list <const char *>{}, 
		fragmentShaderPath ? std::initializer_list <const char *>{fragmentShaderPath } : std::initializer_list <const char *>{}, 
		geometryShaderPath ? std::initializer_list <const char *>{geometryShaderPath } : std::initializer_list <const char *>{}
	)
{ }
Shaders::Shaders(std::initializer_list <const char *> vertexShaderPath, std::initializer_list <const char *> fragmentShaderPath, std::initializer_list <const char *> geometryShaderPath)
    : ShaderCore()
	, modelview()
    , projection()
    , modelviewprojection(-1)
	, rotationPtr(nullptr)
	, translationPtr(nullptr)
    , positions(GL_FLOAT, 3, sizeof(float))
    , normals(GL_FLOAT, NORMALS_SIZE, sizeof(float))
    , colors(GL_FLOAT, 3, sizeof(float))
    , texcoords(GL_FLOAT, 2, sizeof(float))
    , colorUniformValue(1,0,0,1)//Red
	, vertexShaderFiles(buildFileVector(vertexShaderPath))
	, fragmentShaderFiles(buildFileVector(fragmentShaderPath))
	, geometryShaderFiles(buildFileVector(geometryShaderPath))
    , vertexShaderVersion(-1)
    , fragmentShaderVersion(-1)
	, geometryShaderVersion(-1)
{
	reload();
}
Shaders::~Shaders(){
    this->destroyProgram();
	delete vertexShaderFiles;
	delete fragmentShaderFiles;
	delete geometryShaderFiles;
}
bool Shaders::hasVertexShader() const
{
    return this->vertexShaderFiles->size()>0;
}
bool Shaders::hasFragmentShader() const
{
	return this->fragmentShaderFiles->size()>0;
}
bool Shaders::hasGeometryShader() const
{
	return this->geometryShaderFiles->size()>0;
}
bool Shaders::_compileShaders(const GLuint t_programId)
{
	if (vertexShaderFiles->size() > 0)
	{
		this->vertexShaderVersion = compileShader(t_programId, GL_VERTEX_SHADER, vertexShaderFiles);
		if (this->vertexShaderVersion < 0)
			return false;
	}
	if (fragmentShaderFiles->size()>0)
	{
		this->fragmentShaderVersion = compileShader(t_programId, GL_FRAGMENT_SHADER, fragmentShaderFiles);
		if (this->fragmentShaderVersion < 0)
		{
			this->vertexShaderVersion = -1;
			return false;
		}
	}
	if (geometryShaderFiles->size() > 0)
	{
		this->geometryShaderVersion = compileShader(t_programId, GL_GEOMETRY_SHADER, geometryShaderFiles);
		if (this->geometryShaderVersion < 0)
		{
			this->vertexShaderVersion = -1;
			this->fragmentShaderVersion = -1;
			return false;
		}
	}
	return true;
}
void Shaders::_setupBindings(){
    //Locate the modelView matrix uniform
	std::pair<int, GLenum> u_MV = findUniform(MODELVIEW_MATRIX_UNIFORM_NAME, this->getProgram());
    if (u_MV.first >= 0 && u_MV.second == GL_FLOAT_MAT4)
        this->modelview.location = u_MV.first;
    else
        this->modelview.location = -1;
    //Locate the projection matrix uniform
	std::pair<int, GLenum> u_PM = findUniform(PROJECTION_MATRIX_UNIFORM_NAME, this->getProgram());
    if (u_PM.first >= 0 && u_PM.second == GL_FLOAT_MAT4)
        this->projection.location = u_PM.first;
    else
        this->projection.location = -1;
    //Locate the modelViewProjection matrix uniform
	std::pair<int, GLenum> u_MVP = findUniform(MODELVIEWPROJECTION_MATRIX_UNIFORM_NAME, this->getProgram());
    if (u_MVP.first >= 0 && u_MVP.second == GL_FLOAT_MAT4)
        this->modelviewprojection = u_MVP.first;
    else
        this->modelviewprojection = -1;
    //Locate the vertexPosition attribute
    std::pair<int, GLenum> a_V = findAttribute(VERTEX_ATTRIBUTE_NAME, this->getProgram());
    if (a_V.first >= 0 && (a_V.second == GL_FLOAT_VEC3 || a_V.second == GL_FLOAT_VEC4))
        this->positions.location = a_V.first;
    else
        this->positions.location = -1;
    //Locate the vertexNormal attribute
	std::pair<int, GLenum> a_N = findAttribute(NORMAL_ATTRIBUTE_NAME, this->getProgram());
    if (a_N.first >= 0 && (a_N.second == GL_FLOAT_VEC3 || a_N.second == GL_FLOAT_VEC4))
        this->normals.location = a_N.first;
    else
        this->normals.location = -1;
    //Locate the vertexColor attribute
	std::pair<int, GLenum> a_C = findAttribute(COLOR_ATTRIBUTE_NAME, this->getProgram());
    if (a_C.first >= 0 && (a_C.second == GL_FLOAT_VEC3 || a_C.second == GL_FLOAT_VEC4))
        this->colors.location = a_C.first;
    else
        this->colors.location = -1;
    //Locate the vertexTexCoords attribute
	std::pair<int, GLenum> a_T = findAttribute(TEXCOORD_ATTRIBUTE_NAME, this->getProgram());
    if (a_T.first >= 0 && (a_T.second == GL_FLOAT_VEC2 || a_T.second == GL_FLOAT_VEC3))
        this->texcoords.location = a_T.first;
    else
        this->texcoords.location = -1;
    //Locate the color uniform   
	std::pair<int, GLenum> u_C = findUniform(COLOR_ATTRIBUTE_NAME, this->getProgram());
    if (u_C.first >= 0 && (u_C.second == GL_FLOAT_VEC3 || u_C.second == GL_FLOAT_VEC4))
	{
		this->colorUniformLocation = u_C.first;
		this->colorUniformSize = u_C.second == GL_FLOAT_VEC3 ? 3 : 4;
		setColor(this->colorUniformValue);
    }
    else
	{
		this->colorUniformLocation = -1;
		this->colorUniformSize = u_C.second == GL_FLOAT_VEC3 ? 3 : 4;
	}
	//Refresh generic vertex attribs
	std::list<GenericVAD> t_gvad;
	t_gvad.splice(t_gvad.end(), lostGvads);
	t_gvad.splice(t_gvad.end(), gvads);
	for (GenericVAD gvad : t_gvad)
	{
		std::pair<int, GLenum> a_G = findAttribute(gvad.attributeName, this->getProgram());
		if (a_G.first >= 0)
		{
			gvad.location = a_G.first;
			gvads.push_back(gvad);
		}
		else
		{
			lostGvads.push_front(gvad);
			fprintf(stderr, "%s: Generic vertex attrib named: %s could not be located on shader reload.\n", this->getShaderTag(), gvad.attributeName);
		}
	}
}
//Overrides
void Shaders::_useProgram()
{
	//Set the projection matrix (e.g. glFrustum, normally provided by the Visualisation)
	if (this->vertexShaderVersion <= 120 && this->projection.matrixPtr > nullptr)
	{//If old shaders where gl_ModelViewProjectionMatrix is available
		GL_CALL(glMatrixMode(GL_PROJECTION));
		GL_CALL(glLoadMatrixf(glm::value_ptr(*this->projection.matrixPtr)));
	}
	if (this->projection.location >= 0 && this->projection.matrixPtr > nullptr)
	{//If projection matrix location and camera ptr are known
		GL_CALL(glUniformMatrix4fv(this->projection.location, 1, GL_FALSE, glm::value_ptr(*this->projection.matrixPtr)));
	}
	//Calculate modelview matrix transformations
	glm::mat4 mv;
	if (this->modelview.matrixPtr)
	{
		mv = modelMat * *this->modelview.matrixPtr;
		if (this->rotationPtr)
		{
			//Check we actually have a rotation (providing no axis == error)
			if ((this->rotationPtr->x != 0 || this->rotationPtr->y != 0 || this->rotationPtr->z != 0) && this->rotationPtr->w != 0)
				mv = glm::rotate(mv, glm::radians(this->rotationPtr->w), glm::vec3(*this->rotationPtr));
		}
		if (this->translationPtr)
		{
			mv = glm::translate(mv, *this->translationPtr);
		}
	}
	//Set the model view matrix (e.g. gluLookAt, normally provided by the Camera)
	if (this->vertexShaderVersion <= 120 && this->modelview.matrixPtr > nullptr)
	{//If old shaders where gl_ModelViewMatrix is available
		GL_CALL(glMatrixMode(GL_MODELVIEW));
		GL_CALL(glLoadMatrixf(glm::value_ptr(mv)));
	}
	if (this->modelview.location >= 0 && this->modelview.matrixPtr > nullptr)
	{//If modeview matrix location and camera ptr are known
		GL_CALL(glUniformMatrix4fv(this->modelview.location, 1, GL_FALSE, glm::value_ptr(mv)));
	}
	//Set the model view projection matrix (e.g. projection * modelview)
	if (this->modelviewprojection >= 0 && this->modelview.matrixPtr && this->projection.matrixPtr)
	{
		glm::mat4 mvp = *this->projection.matrixPtr * mv;
		GL_CALL(glUniformMatrix4fv(this->modelviewprojection, 1, GL_FALSE, glm::value_ptr(mvp)));
	}

    GLuint activeVBO = 0;
    //Set the vertex (location) attribute
    if (this->vertexShaderVersion <= 120 && this->positions.vbo > 0)
    {//If old shaders where gl_Vertex is available
		GL_CALL(glEnableClientState(GL_VERTEX_ARRAY));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->positions.vbo));
		GL_CALL(glVertexPointer(this->positions.components, this->positions.componentType, this->positions.stride, static_cast<char *>(nullptr) + this->positions.offset));
        activeVBO = this->positions.vbo;
    }
    if (this->positions.location >= 0 && this->positions.vbo > 0)
    {//If vertex attribute location and vbo are known
		GL_CALL(glEnableVertexAttribArray(this->positions.location));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->positions.vbo));
		if (this->positions.componentType == GL_FLOAT || this->positions.componentType == GL_HALF_FLOAT)
		{
			GL_CALL(glVertexAttribPointer(this->positions.location, this->positions.components, this->positions.componentType, GL_FALSE, this->positions.stride, static_cast<char *>(nullptr) + this->positions.offset));
		}
		else if (this->positions.componentType == GL_DOUBLE)
		{
			GL_CALL(glVertexAttribLPointer(this->positions.location, this->positions.components, this->positions.componentType, this->positions.stride, static_cast<char *>(nullptr) + this->positions.offset));
		}
		else
		{
			GL_CALL(glVertexAttribIPointer(this->positions.location, this->positions.components, this->positions.componentType, this->positions.stride, static_cast<char *>(nullptr) + this->positions.offset));
		}
        activeVBO = this->positions.vbo;
    }

    //Set the vertex normal attribute
    if (this->vertexShaderVersion <= 120 && this->normals.vbo > 0)
    {//If old shaders where gl_Vertex is available
		GL_CALL(glEnableClientState(GL_NORMAL_ARRAY));
        if (activeVBO != this->normals.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->normals.vbo));
            activeVBO = this->normals.vbo;
        }
        glNormalPointer(this->normals.componentType, this->normals.stride, static_cast<char *>(nullptr) + this->normals.offset);
    }
    if (this->normals.location >= 0 && this->normals.vbo > 0)
    {//If vertex attribute location and vbo are known
		GL_CALL(glEnableVertexAttribArray(this->normals.location));
        if (activeVBO != this->normals.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->normals.vbo));
            activeVBO = this->normals.vbo;
		}
		if (this->normals.componentType == GL_FLOAT || this->normals.componentType == GL_HALF_FLOAT)
		{
			GL_CALL(glVertexAttribPointer(this->normals.location, this->normals.components, this->positions.componentType, GL_FALSE, this->normals.stride, static_cast<char *>(nullptr) + this->normals.offset));
		}
		else if (this->normals.componentType == GL_DOUBLE)
		{
			GL_CALL(glVertexAttribLPointer(this->normals.location, this->normals.components, this->positions.componentType, this->normals.stride, static_cast<char *>(nullptr) + this->normals.offset));
		}
		else
		{
			GL_CALL(glVertexAttribIPointer(this->normals.location, this->normals.components, this->positions.componentType, this->normals.stride, static_cast<char *>(nullptr) + this->normals.offset));
		}
    }
    //Set the vertex color attributes
    if (this->vertexShaderVersion <= 120 && this->colors.vbo > 0)
    {//If old shaders where gl_Color is available
		GL_CALL(glEnableClientState(GL_COLOR_ARRAY));
        if (activeVBO != this->colors.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->colors.vbo));
            activeVBO = this->colors.vbo;
        }
        glColorPointer(this->colors.components, this->colors.componentType, this->colors.stride, static_cast<char *>(nullptr) + this->colors.offset);
    }
    if (this->colors.location >= 0 && this->colors.vbo > 0)
    {//If color attribute location and vbo are known
		GL_CALL(glEnableVertexAttribArray(this->colors.location));
        if (activeVBO != this->colors.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->colors.vbo));
            activeVBO = this->colors.vbo;
		}
		if (this->colors.componentType == GL_FLOAT || this->colors.componentType == GL_HALF_FLOAT)
		{
			GL_CALL(glVertexAttribPointer(this->colors.location, this->colors.components, this->colors.componentType, GL_FALSE, this->colors.stride, static_cast<char *>(nullptr) + this->colors.offset));
		}
		else if (this->colors.componentType == GL_DOUBLE)
		{
			GL_CALL(glVertexAttribLPointer(this->colors.location, this->colors.components, this->colors.componentType, this->colors.stride, static_cast<char *>(nullptr) + this->colors.offset));
		}
		else
		{
			GL_CALL(glVertexAttribIPointer(this->colors.location, this->colors.components, this->colors.componentType, this->colors.stride, static_cast<char *>(nullptr) + this->colors.offset));
		}
    }

    //Set the vertex texture coord attributes
    if (this->vertexShaderVersion <= 140 && this->texcoords.vbo > 0)
    {//If old shaders where gl_TexCoord is available
		GL_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
        if (activeVBO != this->texcoords.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->texcoords.vbo));
            activeVBO = this->texcoords.vbo;
        }
        glTexCoordPointer(this->texcoords.components, this->texcoords.componentType, this->texcoords.stride, static_cast<char *>(nullptr) + this->texcoords.offset);
    }
    if (this->texcoords.location >= 0 && this->texcoords.vbo > 0)
    {//If texture attribute location and vbo are known
		GL_CALL(glEnableVertexAttribArray(this->texcoords.location));
        if (activeVBO != this->texcoords.vbo)
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->texcoords.vbo));
            activeVBO = this->texcoords.vbo;
		}
		if (this->texcoords.componentType == GL_FLOAT || this->texcoords.componentType == GL_HALF_FLOAT)
		{
			GL_CALL(glVertexAttribPointer(this->texcoords.location, this->texcoords.components, this->texcoords.componentType, GL_FALSE, this->texcoords.stride, static_cast<char *>(nullptr) + this->texcoords.offset));
		}
		else if (this->texcoords.componentType == GL_DOUBLE)
		{
			GL_CALL(glVertexAttribLPointer(this->texcoords.location, this->texcoords.components, this->texcoords.componentType, this->texcoords.stride, static_cast<char *>(nullptr) + this->texcoords.offset));
		}
		else
		{
			GL_CALL(glVertexAttribIPointer(this->texcoords.location, this->texcoords.components, this->texcoords.componentType, this->texcoords.stride, static_cast<char *>(nullptr) + this->texcoords.offset));
		}
    }

	//Generics
	for (GenericVAD const &a : gvads)
	{
		//Set the generic vertex attributes
		if (a.location >= 0 && a.vbo > 0)
		{//If generic vertex attribute location and vbo are known
			GL_CALL(glEnableVertexAttribArray(a.location));
			if (activeVBO != a.vbo)
			{
				GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, a.vbo));
				activeVBO = a.vbo;
			}
			if (a.componentType == GL_FLOAT || a.componentType == GL_HALF_FLOAT)
			{
				GL_CALL(glVertexAttribPointer(a.location, a.components, a.componentType, GL_FALSE, a.stride, static_cast<char *>(nullptr) + a.offset));
			}
			else if (a.componentType == GL_DOUBLE)
			{
				GL_CALL(glVertexAttribLPointer(a.location, a.components, a.componentType, a.stride, static_cast<char *>(nullptr) + a.offset));
			}
			else
			{
				GL_CALL(glVertexAttribIPointer(a.location, a.components, a.componentType, a.stride, static_cast<char *>(nullptr) + a.offset));
			}
		}
	}
}
void Shaders::_clearProgram(){
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
    //Tex Coord
    if (this->vertexShaderVersion <= 140 && this->texcoords.vbo > 0)
    {//If old shaders where gl_Color is available
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (this->texcoords.location >= 0 && this->texcoords.vbo > 0)
    {//If vertex attribute location and vbo are known
        glDisableVertexAttribArray(this->texcoords.location);
    }
	for (GenericVAD const &gvad : gvads)
	{
		glDisableVertexAttribArray(gvad.location);
	}
}
//Bindings
void Shaders::setPositionsAttributeDetail(VertexAttributeDetail vad)
{
    vad.location = this->positions.location;
    this->positions = vad;
}
void Shaders::setNormalsAttributeDetail(VertexAttributeDetail vad)
{
    vad.location = this->normals.location;
    this->normals = vad;
}
bool Shaders::addGenericAttributeDetail(const char* attributeName, VertexAttributeDetail vad)
{
	GenericVAD gvad(vad, attributeName);
	gvad.location = -1;
	//Purge any existing gvad which matches
	removeGenericAttributeDetail(attributeName);
	if (this->getProgram() > 0)
	{
		//Locate the vertexNormal attribute
		std::pair<int, GLenum> a_N = findAttribute(attributeName, this->getProgram());
		if (a_N.first >= 0)
		{
			gvad.location = a_N.first;
			gvads.push_back(gvad);
			return true;
		}
		fprintf(stderr, "%s: Generic vertex attrib named: %s was not found.\n", this->getShaderTag(), gvad.attributeName);
	}
	lostGvads.push_back(gvad);
	return false;
}
bool Shaders::removeGenericAttributeDetail(const char* attributeName)
{
	bool rtn = false;
	for (auto a = lostGvads.begin(); a != lostGvads.end();)
	{
		if (std::string((*a).attributeName) == std::string(attributeName))
		{
			a = lostGvads.erase(a);
			--a;
			rtn = true;
		}
		else
			++a;
	}
	for (auto a = gvads.begin(); a != gvads.end();)
	{
		if (std::string((*a).attributeName) == std::string(attributeName))
		{
			a = gvads.erase(a);
			--a;
			rtn = true;
		}
		else
			++a;
	}
	return rtn;
}
void Shaders::setColorsAttributeDetail(VertexAttributeDetail vad)
{
    vad.location = this->colors.location;
    this->colors = vad;
}
void Shaders::setTexCoordsAttributeDetail(VertexAttributeDetail vad)
{
    vad.location = this->texcoords.location;
    this->texcoords = vad;
}
void Shaders::setColor(glm::vec3 color)
{
    setColor(glm::vec4(color, 1.0f));
}
void Shaders::setColor(glm::vec4 color)
{
	colorUniformValue = color;
    //Set the color uniform if present
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