#include "Points.h"

#define RESIZE_FACTOR 2
#define INITIAL_MAX_COUNT 32

const unsigned int Points::POINT_SIZE = sizeof(glm::vec3)+sizeof(glm::vec4);
const unsigned int Points::POINT_INDEX_SIZE = sizeof(unsigned short) * 2;
const unsigned int Points::FLOATS_PER_POINT = 7;
const unsigned int Points::VERTEX_STRIDE = POINT_SIZE;
const unsigned int Points::POINT_STRIDE = VERTEX_STRIDE;
const unsigned int Points::VERTEX_OFFSET = 0;
const unsigned int Points::COLOR_OFFSET = sizeof(glm::vec3);
const unsigned int Points::VERTEX_FLOAT_OFFSET = 0;
const unsigned int Points::COLOR_FLOAT_OFFSET = 3;

Points::Points(const std::vector<Point> &points, float pointSize)
    : pointSize(pointSize>0 ? pointSize:DEFAULT_POINT_SIZE)
    , vertColor(nullptr)
    , indices(nullptr)
    , maxCount(INITIAL_MAX_COUNT)
    , vbo(0)
    , fvbo(0)
    , vertices(GL_FLOAT, 3, sizeof(float))
    , colors(GL_FLOAT, 4, sizeof(float))
    , faces(GL_UNSIGNED_SHORT, 1, sizeof(unsigned short))
    , shaders(std::make_shared<Shaders>(Stock::Shaders::COLOR))
{
    assert(pointSize > 0);
    //Create buffers
    GL_CALL(glGenBuffers(1, &vbo));
    GL_CALL(glGenBuffers(1, &fvbo));
    count = static_cast<unsigned int>(points.size());
    //Setup maxCount
    while (count>maxCount)
        maxCount *= RESIZE_FACTOR;
    //Allocate host memory
    vertColor = static_cast<float*>(malloc(maxCount * POINT_SIZE));
    indices = static_cast<unsigned short*>(malloc(maxCount * POINT_INDEX_SIZE));
    //Fill host memory
    unsigned int offset = 0;
    for (auto &&it = points.begin(); it != points.end(); ++it)
    {
        memcpy(&vertColor[offset + VERTEX_FLOAT_OFFSET], &it->vertex, sizeof(glm::vec3));
        memcpy(&vertColor[offset + COLOR_FLOAT_OFFSET], &it->color, sizeof(glm::vec4));
        offset += FLOATS_PER_POINT;
    }
    for (unsigned short i = 0; i < maxCount; ++i)
        indices[i] = i;
    //Fill device buffers
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, maxCount * POINT_SIZE, vertColor, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxCount * POINT_INDEX_SIZE, indices, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    //Configure shaders
    vertices.count = count;
    vertices.data = (void*)vertColor;
    vertices.vbo = vbo;
    vertices.stride = VERTEX_STRIDE;

    colors.count = count;
    colors.data = (void*)vertColor;
    colors.offset = COLOR_OFFSET;
    colors.vbo = vbo;
    colors.stride = VERTEX_STRIDE;

    faces.count = count;
    faces.data = (void*)indices;
    faces.vbo = fvbo;

    shaders->setPositionsAttributeDetail(vertices);
    shaders->setColorsAttributeDetail(colors);
    shaders->setModelMatPtr(&this->modelMat);
}
Points::Points(const Point &point, float pointSize)
    : Points(std::vector<Point>({ point }), pointSize)
{ }
Points::Points(const glm::vec3 &vertex, const glm::vec4 &color, float pointSize)
    : Points({ vertex, color }, pointSize)
{
    
}
Points::Points(const glm::vec3 &vertex, const glm::vec3 &color, float pointSize)
    : Points({ vertex, glm::vec4(color, 1.0f) }, pointSize)
{
    
}
Points::Points(const std::initializer_list<glm::vec3> vertices, const glm::vec4 &color, float pointSize)
    : Points(makeInitList(vertices, color), pointSize)
{ }
Points::Points(const std::initializer_list<glm::vec3> vertices, const glm::vec3 &color, float pointSize)
    : Points(makeInitList(vertices, color), pointSize)
{ }
std::vector<Point> Points::makeInitList(const std::initializer_list<glm::vec3> vertices, const glm::vec4 &color)
{
    std::vector<Point> rtn(vertices.size());
    unsigned int i = 0;
    for (auto &&it = vertices.begin(); it != vertices.end(); ++it)
    {
        rtn[i++] = { *it, color };
    }
    return rtn;
}
std::vector<Point> Points::makeInitList(const std::initializer_list<glm::vec3> vertices, const glm::vec3 &color)
{
    return makeInitList(vertices, glm::vec4(color, 1.0f));
}

Points::~Points()
{
    //Free host memory
    if (vertColor)
        free(vertColor);
    if (indices)
        free(indices);
    //Delete buffers
    GL_CALL(glDeleteBuffers(1, &vbo));
    GL_CALL(glDeleteBuffers(1, &fvbo));
}

unsigned int Points::addPoint(const Point &point)
{
    if (count == maxCount)
    {
        //Resize host memory
        maxCount *= RESIZE_FACTOR;
        {//VertColor
            float *t = static_cast<float*>(malloc(maxCount * POINT_SIZE));
            memcpy(t, vertColor, sizeof(count*POINT_SIZE));
            free(vertColor);
            vertColor = t;
        }
        {//Faces
            unsigned short *t = static_cast<unsigned short*>(malloc(maxCount * POINT_INDEX_SIZE));
            memcpy(t, indices, sizeof(count*POINT_INDEX_SIZE));
            free(indices);
            indices = t;
        }
        //Extend face vbo
        for (unsigned short i = count; i < maxCount; ++i)
            indices[i] = i;
        //Add Line
        auto offset = count++*FLOATS_PER_POINT;;
        memcpy(&vertColor[offset + VERTEX_FLOAT_OFFSET], &point.vertex, sizeof(glm::vec3));
        memcpy(&vertColor[offset + COLOR_FLOAT_OFFSET], &point.color, sizeof(glm::vec4));
        //Resize device buffers
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GL_CALL(glBufferData(GL_ARRAY_BUFFER, maxCount * POINT_SIZE, vertColor, GL_STATIC_DRAW));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
        GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxCount * POINT_INDEX_SIZE, indices, GL_STATIC_DRAW));
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }
    else
    {
        setPoint(count++, point);
    }
    //Update Vertex Attribute Data
    vertices.count = count;
    colors.count = count;
    faces.count = count;
    return count - 1;
}
unsigned int Points::addPoint(const glm::vec3 &vertex, const glm::vec4 &color)
{
    return addPoint({ vertex, color });
}
unsigned int Points::addPoint(const glm::vec3 &vertex, const glm::vec3 &color)
{
    return addPoint({ vertex, glm::vec4(color, 1.0f) });
}

bool Points::setPoint(const unsigned int &index, const Point &point)
{
    if (index >= this->count)
        return false;
    auto offset = index*FLOATS_PER_POINT;
    memcpy(&vertColor[offset + VERTEX_FLOAT_OFFSET], &point.vertex, sizeof(glm::vec3));
    memcpy(&vertColor[offset + COLOR_FLOAT_OFFSET], &point.color, sizeof(glm::vec4));
    //Add to device buffer
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, index*POINT_SIZE, POINT_SIZE, &vertColor[offset]));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    return true;
}
bool Points::setPoint(const unsigned int &index, const glm::vec3 &vertex, const glm::vec4 &color)
{
    return setPoint(index, {vertex, color});
}
bool Points::setPoint(const unsigned int &index, const glm::vec3 &vertex, const glm::vec3 &color)
{
    return setPoint(index, { vertex, glm::vec4(color, 1.0f) });
}
bool Points::setPointLocation(const unsigned int &index, const glm::vec3 &vertex)
{
    if (index >= this->count)
        return false;
    auto offset = index*FLOATS_PER_POINT;
    memcpy(&vertColor[offset + VERTEX_FLOAT_OFFSET], &vertex, sizeof(glm::vec3));
    //Add to device buffer
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, (index*POINT_SIZE) + VERTEX_OFFSET, sizeof(glm::vec3), &vertColor[offset + VERTEX_FLOAT_OFFSET]));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    return true;
}
bool Points::setPointLocation(const unsigned int &index, const float &x, const float &y, const float &z)
{
    return setPointLocation(index, { x, y, z });
}
bool Points::setPointColor(const unsigned int &index, const glm::vec4 &color)
{
    if (index >= this->count)
        return false;
    auto offset = index*FLOATS_PER_POINT;
    memcpy(&vertColor[offset + COLOR_FLOAT_OFFSET], &color, sizeof(glm::vec4));
    //Add to device buffer
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, (index*POINT_SIZE) + COLOR_OFFSET, sizeof(glm::vec4), &vertColor[offset + COLOR_FLOAT_OFFSET]));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    return true;
}
bool Points::setPointColor(const unsigned int &index, const glm::vec3 &color)
{
    return setPointColor(index, glm::vec4(color, 1.0f));
}
bool Points::setPointColor(const unsigned int &index, const float &r, const float &g, const float &b, const float &a)
{
    return setPointColor(index, { r, g, b, a });
}

void Points::setPointSize(float pointSize)
{
    this->pointSize = pointSize > 0 ? pointSize : pointSize;
    assert(pointSize > 0);
}


glm::vec3 Points::getLocation(const unsigned int &index)
{
    assert(index < count);
    auto offset = index*FLOATS_PER_POINT;
    return *(glm::vec3*)&vertColor[offset + VERTEX_FLOAT_OFFSET];
}

glm::mat4 Points::render(const unsigned int &shaderIndex, glm::mat4 transform)
{
    //Grab current point size
    float _pointSize = pointSize;
    GL_CALL(glGetFloatv(GL_POINT_SIZE, &_pointSize));

#ifdef _DEBUG
    assert(shaderIndex == 0);
#endif
    shaders->useProgram();
    transform = shaders->overrideModelMat(&transform);
    GL_CALL(glPointSize(pointSize));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces.vbo));
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_POINT));
    GL_CALL(glDrawElements(GL_POINTS, faces.count * faces.components, faces.componentType, nullptr));
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GL_CALL(glDisable(GL_BLEND));
    shaders->clearProgram();
    //Reset point size
    GL_CALL(glPointSize(_pointSize));
    return transform;
}
void Points::reload()
{
    shaders->reload();
}
void Points::setViewMatPtr(glm::mat4 const *viewMat)
{
    shaders->setViewMatPtr(viewMat);
}
void Points::setProjectionMatPtr(glm::mat4 const *projectionMat)
{
    shaders->setProjectionMatPtr(projectionMat);
}
void Points::setModelMat(glm::mat4 const modelMat)
{
    this->modelMat = modelMat;
}
glm::mat4 Points::getModelMat()
{
    return modelMat;
}