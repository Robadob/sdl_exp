#include "Draw.h"
#include "util/StringUtils.h"
#include "PortableDraw.h"

const unsigned int Draw::DEFAULT_INITIAL_VBO_LENGTH = 1024;
const float Draw::STORAGE_MUTLIPLIER = 2.0f;
Draw::DrawData::DrawData(const unsigned int &vboLength)
: shaders(std::make_shared<Shaders>(Stock::Shaders::COLOR_NOSHADE))
, vertices(GL_FLOAT, 3, sizeof(float))
, colors(GL_FLOAT, 4, sizeof(float))
{
    vertices.count = vboLength;
    vertices.data = nullptr;

    colors.count = vboLength;
    colors.data = nullptr;

    //Vertices vbo
    GL_CALL(glGenBuffers(1, &vertices.vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertices.vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, vboLength * sizeof(glm::vec3), nullptr, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Colors vbo
    GL_CALL(glGenBuffers(1, &colors.vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colors.vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, vboLength * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    shaders->setPositionsAttributeDetail(vertices, false);
    shaders->setColorsAttributeDetail(colors);
}

Draw::DrawData::~DrawData()
{
    GL_CALL(glDeleteBuffers(1, &vertices.vbo));
    GL_CALL(glDeleteBuffers(1, &colors.vbo));
}
Draw::Draw(const unsigned int &bufferLength, const glm::vec4 &initialColor, const float &initialWidth)
	: tName()
	, tColor(initialColor)
	, tWidth(initialWidth)
	, tType()
    , vboLen(bufferLength == 0 ? DEFAULT_INITIAL_VBO_LENGTH : bufferLength)
	, vboOffset(0)
	, requiredLength(0)
	, data(std::make_shared<DrawData>(vboLen))
{
	assert(STORAGE_MUTLIPLIER > 1.0f);
}
void Draw::begin(Type type, const std::string &name)
{
	if (isDrawing)
		throw std::runtime_error("Draw::begin() cannot be called whilst the draw state is already open.\n");
	isDrawing = true;
	tType = type;
	tName = name;
}
void Draw::end(bool replaceExisting)
{
	if (!isDrawing)
		throw std::runtime_error("Draw::end() cannot be called whilst the draw state is not open.\n");
	if (tName.empty())
	{
		State s = _save(true);
		render(s);
	}
	else
	{
		std::string stateName = tName;//Save will purge name
		save(replaceExisting); //Forward to named draw state end to ensure it is saved
		render(stateName);
	}
}
void Draw::save(bool replaceExisting)
{
	if (!isDrawing)
		throw std::runtime_error("Draw::save() cannot be called whilst the draw state is not open.\n");
	if (tName.empty())
		throw std::runtime_error("Draw::save() cannot be called to close anonymous draw states.\n");
	//Save draw state
	State tState = _save(false);
	//Store draw state in map
    auto f = data->stateDirectory.find(tName);
    if (f != data->stateDirectory.end())
	{
		if (!replaceExisting)
		{
			throw std::runtime_error("Overwriting draw states must be an explicit action, use Draw::save(true).\n");
		}
		else
		{
			vboGaps.push_back({ f->second.offset, f->second.count });
			requiredLength -= f->second.count;
            data->stateDirectory.erase(tName);
		}
	}
	requiredLength += tState.count;
    data->stateDirectory.insert({ tName, std::move(tState) });
}
Draw::State Draw::_save(bool isTemporary)
{
	if (tType == Lines && tVertices.size() % 2 != 0)
		throw std::runtime_error("Line drawings require an even number of vertices.\n");
	assert(tVertices.size() == tColors.size());
	//Select the smallest gap which is big enough
	unsigned int best = UINT_MAX;
	unsigned int bestCt = UINT_MAX;
	for (unsigned int i = 0; i < vboGaps.size(); ++i)
	{
		unsigned int count = std::get<1>(vboGaps[i]);
		if (count>tVertices.size())
		{
			if (bestCt < count)
				continue;
			best = i;
			bestCt = count;
		}
	}
	//If we found a suitable gap
	unsigned int bufferPos;
	if (best<vboGaps.size())
	{//We shall use a gap, so remove it from gap list
		unsigned int offset = std::get<0>(vboGaps[best]);
		bufferPos = offset;
		if (!isTemporary)
		{
			unsigned int count = std::get<1>(vboGaps[best]);
			vboGaps.erase(vboGaps.begin() + best);
			if (count >tVertices.size())
			{//Add remainder of gap back to gap list
				vboGaps.push_back({ offset + (unsigned int)tVertices.size(), count - (unsigned int)tVertices.size() });
			}
		}
	}
	else
	{//No suitable gap, resize buffer
		if (vboOffset + tVertices.size()>vboLen)
		{
			//Resize buffer
			unsigned int newLen = vboLen;
			while (requiredLength + tVertices.size()>newLen)
			{
				newLen = (unsigned int)(newLen * STORAGE_MUTLIPLIER);
			}
			resize(newLen);
		}
		bufferPos = vboOffset;
		vboOffset += (unsigned int)tVertices.size();
	}
	//Close and package draw state
	State rtn;
	rtn.count = (unsigned int)tVertices.size();
	rtn.offset = bufferPos;
	rtn.mType = tType;
	rtn.mWidth = tWidth;
	//Fill VBO
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, data->vertices.vbo));
	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, bufferPos*sizeof(glm::vec3), rtn.count*sizeof(glm::vec3), tVertices.data()));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, data->colors.vbo));
	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, bufferPos*sizeof(glm::vec4), rtn.count*sizeof(glm::vec4), tColors.data()));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	//Clear temporary structures
	tVertices.clear();
	tColors.clear();
	isDrawing = false;
	//Return
	return rtn;
}
std::shared_ptr<PortableDraw> Draw::makePortable(const std::string &name)
{
    return std::shared_ptr<PortableDraw>(new PortableDraw(name, data));
}
void Draw::render(const std::string &name)
{
	const auto &&f = data->stateDirectory.find(name);
    if (f == data->stateDirectory.end())
	{
		throw std::runtime_error(su::format("Draw state '%s' was not found for render().\n", name.c_str()));
	}
	render(f->second);
}
void Draw::render(const State &state) const
{
	setWidth(state.mType, state.mWidth);
    data->shaders->useProgram();
	GL_CALL(glDrawArrays(toGL(state.mType), state.offset, state.count));
    data->shaders->clearProgram();
	clearWidth(state.mType);
}
GLenum Draw::toGL(const Type &t)
{
	if (t == Lines)
	{
		return GL_LINES;
	}
	else if (t == Polyline)
	{
		return GL_LINE_STRIP;
	}
	else if (t == Points)
	{
		return GL_POINTS;
	}
	throw std::runtime_error("Unexpected Type pased to Draw::toGL()\n");
}
void Draw::setWidth(const Type &t, const float &w)
{
	if (t == Lines)
	{
		GL_CALL(glLineWidth(w));
	}
	else if (t == Polyline)
	{
		GL_CALL(glLineWidth(w));
	}
	else if (t == Points)
	{
		GL_CALL(glPointSize(w));
	}
	else
		throw std::runtime_error("Unexpected Type passed to Draw::setWidth()\n");
}
void Draw::clearWidth(const Type &t)
{
	if (t == Lines)
	{
		GL_CALL(glLineWidth(1.0f));
	}
	else if (t == Polyline)
	{
		GL_CALL(glLineWidth(1.0f));
	}
	else if (t == Points)
	{
		GL_CALL(glPointSize(1.0f));
	}
	else
		throw std::runtime_error("Unexpected Type passed to Draw::clearWidth()\n");
}
void Draw::resize(unsigned int newLength)
{
    assert(newLength > requiredLength);
	/**
	* Allocate new vbos of double size
	*/
	//Vertices vbo
	GLuint _vbo = 0, _cvbo = 0;
	unsigned int vboSize = newLength * sizeof(glm::vec3);
	GL_CALL(glGenBuffers(1, &_vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, vboSize, nullptr, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	//Colors vbo
	unsigned int cvboSize = newLength * sizeof(glm::vec4);
	GL_CALL(glGenBuffers(1, &_cvbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _cvbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, cvboSize, nullptr, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	/**
	* Copy back data
	*/
	unsigned int _vboSize = vboLen * sizeof(glm::vec3);
	unsigned int _cvboSize = vboLen * sizeof(glm::vec4);
	glm::vec3 *_vertices = (glm::vec3 *)malloc(_vboSize);
	glm::vec4 *_colors = (glm::vec4 *)malloc(_cvboSize);
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
	GL_CALL(glGetBufferSubData(GL_ARRAY_BUFFER, 0, _vboSize, _vertices));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _cvbo));
	GL_CALL(glGetBufferSubData(GL_ARRAY_BUFFER, 0, _cvboSize, _colors));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	unsigned int _offset = 0;
	/**
	* Defragment from old into new
	*/
	for (auto &a : data->stateDirectory)
	{
		//Copy back to vbo
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, _offset*sizeof(glm::vec3), a.second.count*sizeof(glm::vec3), &_vertices[a.second.offset]));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _cvbo));
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, _offset*sizeof(glm::vec4), a.second.count*sizeof(glm::vec4), &_colors[a.second.offset]));
		a.second.offset = _offset;
		_offset += a.second.count;
	}
	assert(_offset == requiredLength);
	vboOffset = _offset;
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	free(_vertices);
	free(_colors);
	vboGaps.clear();
	/**
	 * Delete and replace old vbos
	 */
    GL_CALL(glDeleteBuffers(1, &data->vertices.vbo));
    GL_CALL(glDeleteBuffers(1, &data->colors.vbo));
    data->vertices.vbo = _vbo;
    data->colors.vbo = _cvbo;
    data->shaders->setPositionsAttributeDetail(data->vertices, false);
    data->shaders->setColorsAttributeDetail(data->colors);
	vboLen = newLength;
}
void Draw::reload()
{
    data->shaders->reload();
}
void Draw::setViewMatPtr(glm::mat4 const *viewMat)
{
    data->shaders->setViewMatPtr(viewMat);
}
void Draw::setProjectionMatPtr(glm::mat4 const *projectionMat)
{
    data->shaders->setProjectionMatPtr(projectionMat);
}
void Draw::setLightsBuffer(const GLuint &bufferBindingPoint)
{
    data->shaders->setLightsBuffer(bufferBindingPoint);
}