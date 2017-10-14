#ifndef __Points_h__
#define __Points_h__
#include "Primitives.h"
#include "../interface/Renderable.h"
#include "../shader/Shaders.h"
#include <vector>
#define DEFAULT_POINT_SIZE 5.0f
class Points : public Renderable
{
public:
    Points(const Point &point, float pointSize = DEFAULT_POINT_SIZE);
    Points(const std::vector<Point> &points, float pointSize = DEFAULT_POINT_SIZE);
    Points(const glm::vec3 &vertex, const glm::vec4 &color, float pointSize = DEFAULT_POINT_SIZE);
    Points(const glm::vec3 &vertex, const glm::vec3 &color, float pointSize = DEFAULT_POINT_SIZE);
    Points(const std::initializer_list<glm::vec3> vertices, const glm::vec4 &color, float pointSize = DEFAULT_POINT_SIZE);
    Points(const std::initializer_list<glm::vec3> vertices, const glm::vec3 &color, float pointSize = DEFAULT_POINT_SIZE);
    ~Points();

    unsigned int addPoint(const Point &point);
    unsigned int addPoint(const glm::vec3 &vertex, const glm::vec4 &color);
    unsigned int addPoint(const glm::vec3 &vertex, const glm::vec3 &color);

    bool setPoint(const unsigned int &index, const Point &point);
    bool setPoint(const unsigned int &index, const glm::vec3 &vertex, const glm::vec4 &color);
    bool setPoint(const unsigned int &index, const glm::vec3 &vertex, const glm::vec3 &color);
    bool setPointLocation(const unsigned int &index, const glm::vec3 &vertex);
    bool setPointLocation(const unsigned int &index, const float &x, const float &y, const float &z);
    bool setPointColor(const unsigned int &index, const glm::vec4 &color);
    bool setPointColor(const unsigned int &index, const glm::vec3 &color);
    bool setPointColor(const unsigned int &index, const float &r, const float &g, const float &b, const float &a=1.0f);

    glm::vec3 getLocation(const unsigned int &index=0);

    void setPointSize(float pointSize);

    glm::mat4 render(const unsigned int &shaderIndex = 0, glm::mat4 transform = glm::mat4()) override;
    void reload() override;
    /**
    * Binds the provided modelview matrix to the internal shader
    * @param viewMat Ptr to modelview matrix
    * @note This is normally found within the Camera object
    */
    void setViewMatPtr(glm::mat4 const *viewMat) override;
    /**
    * Binds the provided projection matrix to the internal shader
    * @param projectionMat Ptr to model view matrix
    * @note This is normally found within the Viewport object
    */
    void setProjectionMatPtr(glm::mat4 const *projectionMat) override;
    void setModelMat(glm::mat4 const modelMat) override;
    glm::mat4 getModelMat() override;
    static const unsigned int POINT_SIZE;
    static const unsigned int POINT_INDEX_SIZE;
    static const unsigned int FLOATS_PER_POINT;
    static const unsigned int VERTEX_STRIDE;
    static const unsigned int POINT_STRIDE;
    static const unsigned int VERTEX_OFFSET;
    static const unsigned int COLOR_OFFSET;
    static const unsigned int VERTEX_FLOAT_OFFSET;
    static const unsigned int COLOR_FLOAT_OFFSET;
private:
    float pointSize;
    float *vertColor;//vec3,vec4 strided data
    unsigned short*indices;//uint16 index data

    unsigned int count;
    unsigned int maxCount;

    GLuint vbo, fvbo;
    static std::vector<Point> makeInitList(const std::initializer_list<glm::vec3> vertices, const glm::vec4 &color);
    static std::vector<Point> makeInitList(const std::initializer_list<glm::vec3> vertices, const glm::vec3 &color);

    Shaders::VertexAttributeDetail vertices, colors, faces;
    std::shared_ptr<Shaders> shaders;
    glm::mat4 modelMat;
};

#endif //__Points_h__