#ifndef __Camera_h__
#define __Camera_h__

#include "util/GLcheck.h"

#include <glm/glm.hpp>

class Camera
{
public:
	/**
	 * Initialises the camera located at (1,1,1) directed at (0,0,0)
	 */
    Camera();
	/**
	 * Initialises the camera located at eye, directed at (0,0,0)
	 * @param eye The coordinates the camera is located
	 */
    Camera(glm::vec3 eye);
	/**
	* Initialises the camera located at eye directed at target
	* @param eye The coordinates the camera is located
	* @param target The coordinates the camera is directed towards
    */
	Camera(glm::vec3 eye, glm::vec3 target);
	/**
	 * Default destructor
	 */
    ~Camera();

	/**
	 * Rotate look and right, yaw radians about up
	 * Rotate look and up, pitch radians about right
	 * @param yaw The number of radians to rotate the camera's direction about the up vector
	 * @param pitch The number of radians to rotate the camera's direction about the right vector
	 */
	void turn(float yaw, float pitch);
	/**
	 * Move eye specified distance along look
	 * @param distance The number of units to move the camera
	 */
	void move(float distance);
	/**
	 * Move eye specified distance along right
	 * @param distance The number of units to move the camera
	 */
	void strafe(float distance);
	/**
	 * Move eye specified distance along pureUp
	 * @param distance The number of units to move the camera
	 */
	void ascend(float distance);
	/**
	 * Rotate right and up, roll radians about look
	 * @param roll The number of radians to rotate the camera's direction about the look vector
	 */
	void roll(float roll);
	/**
	 * Sets whether the camera should be stabilised
	 * When the camera is stabilised, the up vector will not rotate about the cameras direction
	 * When the camera is not stabilsed, moving the mouse in a circular motion may cause the camera to roll
	 * @param stabilise Whether the camera should be stabilised
	 */
	void setStabilise(bool stabilise);
	/**
	 * Returns the projection matrix
	 * For use with shader uniforms or glLoadMatrixf() after calling glMatrixMode(GL_MODELVIEW)
	 * @return the modelview matrix as calculated by glm::lookAt(glm::vec3, glm::vec3, glm::vec3)
	 */
	glm::mat4 view() const;
	/**
	 * Calls gluLookAt()
	 * For people using fixed function pipeline
	 * @see view()
	 */
	void gluLookAt();
	/**
	 * Returns the projection matrix from the perspective required for rendering a skybox (direction only)
	 * For use with shader uniforms or glLoadMatrixf() after calling glMatrixMode(GL_MODELVIEW)
	 * @return the modelview matrix as calculated by glm::lookAt(glm::vec3, glm::vec3, glm::vec3)
	 */
	glm::mat4 skyboxView() const;
	/**
	 * Calls gluLookAt() from the perspective required for rendering a skybox (direction only)
	 * For people using fixed function pipeline, although manually setting the matrix with glLoadMatrixf() also works.
	 * @see skyboxView()
	 */
	void skyboxGluLookAt() const;
	/**
	 * Returns the cameras location
	 * @return The location of the camera in world space
	 */
	glm::vec3 getEye() const;
	/**
	 * Returns the cameras normalized direction vector
	 * @return The normalized direction of the camera
	 */
	glm::vec3 getLook() const;
	/**
	 * Returns the cameras normalized up vector
	 * @return The normalized direction the camera treats as upwards
	 */
	glm::vec3 getUp() const;
	/**
	 * Returns the value of pureUp
	 * This value is used by the stabilisation to prevent the camera rolling unintentionally
	 * @return The normalized direction the camera treats as the true up
	 */
	glm::vec3 getPureUp() const;
	/**
	 * Returns the cameras normalized right vector
	 * @return The normalized direction the camera treats as rightwards
	 */
	glm::vec3 getRight() const;
	/**
	 * Returns a constant pointer to the cameras modelview matrix
	 * This pointer can be used to continuously track the modelview matrix
	 * @return A pointer to the modelview matrix
	 */
	const glm::mat4 *Camera::getViewMatPtr() const;
	/**
	 * Returns a constant pointer to the cameras skybox modelview matrix
	 * This pointer can be used to continuously track the skybox modelview matrix
	 * @return A pointer to the modelview matrix
	 */
    const glm::mat4 *Camera::getSkyboxViewMatPtr() const;
private:
	/**
	 * Updates the view and skyboxView matrices
	 * Called whenever any internal camera variables are updated
	 */
    void updateViews();
    //View matrix
    glm::mat4 viewMat;
    //View matrix without camera position taken into consideration
    glm::mat4 skyboxViewMat;
    //Up vector used for stabilisation, only rotated when roll is called
    glm::vec3 pureUp;
    //Eyelocation
    glm::vec3 eye;
    //3 perpendicular vectors which represent the cameras direction and orientation
    glm::vec3 look;
    glm::vec3 right;
    glm::vec3 up;
    bool stabilise;
};

#endif //ifndef __Camera_h__