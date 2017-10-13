#ifndef __Primitives_h__
#define __Primitives_h__
#include <glm/glm.hpp>
#include <utility>
struct Point
{
	Point()
		: Point(glm::vec3(0), glm::vec4(1))
	{ }
	Point(const glm::vec3 &vertex, const glm::vec4 &color)
	: vertex(vertex)
	, color(color) 
	{ };
	Point(const glm::vec3 &vertex, const glm::vec3 &color) 
	: vertex(vertex)
	, color(color, 1.0f)
	{ };
	glm::vec3 vertex;
	glm::vec4 color;
};
struct Line
{
	Line()
		:Line({ glm::vec3(0), glm::vec4(1) }, { glm::vec3(1), glm::vec4(1) })
	{ }
	Line(Point a, Point b)
	{
		vertices[0] = a.vertex;
		vertices[1] = b.vertex;
		colors[0] = a.color;
		colors[1] = b.color;
	};
	Line(std::pair<Point, Point> ab);
	glm::vec3 vertices[2];
	glm::vec4 colors[2];
};
#endif //__Primitives_h__