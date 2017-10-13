#ifndef __Lines_h__
#define __Lines_h__
#include "Primitives.h"
#include "../interface/Renderable.h"
#include "../shader/Shaders.h"
#include <vector>
class Lines : public Renderable
{
public:
	Lines(const Line &line);
	Lines(const std::vector<Line> &lines);
	Lines(const std::initializer_list<Point> &pointPairs);
	Lines(const Point &vertexA, const Point &vertexB);
	Lines(const glm::vec3 &vertexA, const glm::vec3 &vertexB, const glm::vec4 &color);
	Lines(const glm::vec3 &vertexA, const glm::vec3 &vertexB, const glm::vec3 &color);
	Lines(const std::initializer_list<glm::vec3> vertices, const glm::vec4 &color);
	Lines(const std::initializer_list<glm::vec3> vertices, const glm::vec3 &color);
	~Lines();

	unsigned int addLine(const Line &line);
	unsigned int addLine(const Point &pointA, const Point &pointB);
	unsigned int addLine(const glm::vec3 &vertexA, const glm::vec3 &vertexB, glm::vec4 color);
	unsigned int addLine(const glm::vec3 &vertexA, const glm::vec3 &vertexB, glm::vec3 color);

	bool setLine(const unsigned int &index, const Line &line);
	bool setLine(const unsigned int &index, const Point &pointA, Point &pointB);
	bool setLine(const unsigned int &index, const glm::vec3 &vertexA, const glm::vec3 &vertexB, glm::vec4 color);
	bool setLine(const unsigned int &index, const glm::vec3 &vertexA, const glm::vec3 &vertexB, glm::vec3 color);

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
	static const unsigned int LINE_SIZE;
	static const unsigned int LINE_INDICES_SIZE;
	static const unsigned int VERTEX_STRIDE;
	static const unsigned int VERTEX_A_OFFSET;
	static const unsigned int VERTEX_B_OFFSET;
	static const unsigned int COLOR_A_OFFSET;
	static const unsigned int COLOR_B_OFFSET;
private:
	float *vertColor;//vec3,vec4 strided data
	unsigned short*indices;//uint16 index data

	unsigned int count;
	unsigned int maxCount;

	GLuint vbo, fvbo;
	static std::vector<Line> makeInitList(const std::initializer_list<Point> &pointPairs);
	static std::vector<Line> makeInitList(const std::initializer_list<glm::vec3> vertices, const glm::vec4 &color);
	static std::vector<Line> makeInitList(const std::initializer_list<glm::vec3> vertices, const glm::vec3 &color);

	Shaders::VertexAttributeDetail vertices, colors, faces;
	std::shared_ptr<Shaders> shaders;
};

#endif //__Lines_h__