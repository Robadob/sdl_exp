#include "TowerCrane.h"
#include "model/SceneGraphJoint.h"

std::shared_ptr<TowerCrane> TowerCrane::make(glm::vec3 location, float scale)
{
    try
    {
        auto rtn = std::shared_ptr<TowerCrane>(new TowerCrane(location, scale));
        rtn->setupCraneSceneGraph();//Do this here, else we get a bad pointer exception as it's not yet a shared pointer
        return rtn;
    }
    catch (std::exception)
    {
        //Do nothing, return at end
    }
    return nullptr;
}
TowerCrane::TowerCrane(glm::vec3 location, float scale)
    : craneBase(new Entity(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
    , craneTower(new Entity(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
    , craneCounterWeight(new Entity(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
    , craneJib(new Entity(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
    , craneTrolley(new Entity(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
    , spinJoint(SGJoint::make())
    , trolleySlideJoint(SlideJoint::makeX())
    , scale(scale)
    , location(location)
    , jibRot(0)
    , trolleyOffset(0)
{
    if (!(craneBase&&craneTower&&craneCounterWeight&&craneJib&&craneTrolley))
        throw std::exception("Crane was unable to load all entities!");
}

void TowerCrane::setViewMatPtr(glm::mat4 const* viewMat)
{
    craneBase->setViewMatPtr(viewMat);
    craneTower->setViewMatPtr(viewMat);
    craneCounterWeight->setViewMatPtr(viewMat);
    craneJib->setViewMatPtr(viewMat);
    craneTrolley->setViewMatPtr(viewMat);
}

void TowerCrane::setProjectionMatPtr(glm::mat4 const* projectionMat)
{
    craneBase->setProjectionMatPtr(projectionMat);
    craneTower->setProjectionMatPtr(projectionMat);
    craneCounterWeight->setProjectionMatPtr(projectionMat);
    craneJib->setProjectionMatPtr(projectionMat);
    craneTrolley->setProjectionMatPtr(projectionMat);
}

const float BASE_HEIGHT = 4;
const float BASE_WIDTH = 10;
const float TOWER_HEIGHT = 25;
const float TOWER_WIDTH = 1;
const float JIB_LENGTH = 30;
const float JIB_WIDTH = TOWER_WIDTH;
const float JIB_HEIGHT = 0.5f;
const float TROLLEY_WIDTH = JIB_WIDTH;
const float TROLLEY_LENGTH = 3.0f;
const float TROLLEY_HEIGHT = 0.5f;
const float TROLLEY_TRACK_LENGTH = (JIB_LENGTH *0.75f) - TROLLEY_LENGTH - (0.5f*TOWER_WIDTH);
void TowerCrane::setupCraneSceneGraph()
{
    //Crane Base
    {
        glm::mat4 mm = glm::mat4(1);
        //Scale to size
        mm *= glm::scale(glm::vec3(BASE_WIDTH, BASE_HEIGHT, BASE_WIDTH));
        //Translate so bottom centre is at origin
        mm *= glm::translate(glm::vec3(0, 0.5, 0));
        //Setup
        craneBase->setModelMat(mm);
        //Add to Scene Graph (at root)
        attach(craneBase, "crane_base");
        craneBase->setMaterial(glm::vec3(0.1f), glm::vec3(1));
    }
    //Crane Tower
    {
        glm::mat4 mm = glm::mat4(1);
        //Scale to size
        mm *= glm::scale(glm::vec3(TOWER_WIDTH, TOWER_HEIGHT, TOWER_WIDTH));
        //Translate so bottom centre is at origin
        mm *= glm::translate(glm::vec3(0, 0.5, 0));
        //Setup
        craneTower->setModelMat(mm);
        //Add to Scene Graph (at top centre of base)
        craneBase->attach(craneTower, "crane_tower", glm::vec3(0, BASE_HEIGHT, 0));//This is in world space!
        craneTower->setMaterial(glm::vec3(0.095, 0.1, 0.025), glm::vec3(0.95, 1, 0.25));
    }
    //Crane Jib (arm)
    {
        glm::mat4 mm = glm::mat4(1);
        //Scale to size
        mm *= glm::scale(glm::vec3(JIB_LENGTH, JIB_HEIGHT, JIB_WIDTH));
        //Translate so bottom 25% along is origin
        mm *= glm::translate(glm::vec3(0.25, 0.5, 0));
        //Setup
        craneJib->setModelMat(mm);
        //Add Rotational Joint to scene graph
        craneTower->attach(spinJoint, "crane_spin_tower", glm::vec3(0, TOWER_HEIGHT, 0));
        //Atach Jib to rotational joint
        spinJoint->attach(craneJib, "crane_spin_jib", glm::vec3(0));
        craneJib->setMaterial(glm::vec3(0.085, 0.09, 0.025), glm::vec3(0.85, 0.9, 0.25));
    }
    //Counter-weight
    {
        glm::mat4 mm = glm::mat4(1);
        //Scale to size
        mm *= glm::scale(glm::vec3(JIB_LENGTH * 0.15, JIB_HEIGHT * 5, JIB_WIDTH));
        //Translate so top -x is at origin
        mm *= glm::translate(glm::vec3(0.5, -0.5, 0));
        //Setup
        craneCounterWeight->setModelMat(mm);
        //Add to scene graph
        craneJib->attach(craneCounterWeight, "crane_counter_weight", glm::vec3(-0.25*JIB_LENGTH, 0, 0));
        craneCounterWeight->setMaterial(glm::vec3(0.07, 0.07, 0.07), glm::vec3(0.7, 0.7, 0.7));
    }
    //Trolley
    {
        glm::mat4 mm = glm::mat4(1);
        //Scale to size
        mm *= glm::scale(glm::vec3(TROLLEY_LENGTH, TROLLEY_HEIGHT, TROLLEY_WIDTH));
        //Translate so top x is at origin
        mm *= glm::translate(glm::vec3(0.5, -0.5, 0));
        //Setup
        craneTrolley->setModelMat(mm);
        //Slider joint
        {
            //Set Limits
            trolleySlideJoint->setMax((JIB_LENGTH*0.75f) - TROLLEY_LENGTH);
            trolleySlideJoint->setMin(0.5);
            trolleySlideJoint->set(JIB_LENGTH);
            //Add to scene graph
            craneJib->attach((std::shared_ptr<SceneGraphJoint>)trolleySlideJoint, "crane_slide_jib", glm::vec3(0));
        }
        trolleySlideJoint->attach(craneTrolley, "crane_slide_trolley", glm::vec3(0));
        craneTrolley->setMaterial(glm::vec3(0.02, 0.04, 0.02), glm::vec3(0.2, 0.4, 0.2));
    }
    //Cable
    {
        
    }
    //Hook
    {
        
    }

}
void TowerCrane::update(unsigned int frameTime)
{
    //Jib Rot
    if (jibRot!=0)
    {
        const float FULL_SPIN_MS = 10000.0f;
        const float MAX_SPIN_FRAME  = 2 * (float)M_PI * frameTime / FULL_SPIN_MS;
        float jibRotFrame = glm::min(jibRot, MAX_SPIN_FRAME);
        spinJoint->rotate(jibRotFrame, glm::vec3(0, 1, 0));
        jibRot -= jibRotFrame;
    }
    //Trolley Slide
    if (trolleyOffset != 0)
    {
        const float FULL_SLIDE_MS = 10000.0f;
        const float MAX_SLIDE_FRAME = TROLLEY_TRACK_LENGTH * frameTime / FULL_SLIDE_MS;
        float trolleyOffsetFrame = glm::min(trolleyOffset, MAX_SLIDE_FRAME);
        trolleySlideJoint->move(trolleyOffsetFrame);
        trolleyOffset -= trolleyOffsetFrame;
    }
}
void TowerCrane::setJibRotation(const float &rads)
{
    //spintJoint->setModelMat(glm::rotate(rads, glm::vec3(0,1,0)));
}
void TowerCrane::rotateJib(const float &rads, const bool &limitSpeed)
{
    if (limitSpeed)
    {
        jibRot += rads;
    }
    else
    {
       spinJoint->rotate(rads, glm::vec3(0, 1, 0));
    }
}
void TowerCrane::setTrolleyPosition(const float &pos)
{
    trolleySlideJoint->set(pos);
}
void TowerCrane::slideTrolley(const float &offset, const bool &limitSpeed)
{

    if (limitSpeed)
    {
        trolleyOffset += offset;
    }
    else
    {
        trolleySlideJoint->move(offset);
    }
}
void TowerCrane::reload()
{
    craneBase->reload();
    craneTower->reload();
    craneCounterWeight->reload();
    craneJib->reload();
    craneTrolley->reload();
}
void TowerCrane::setLightsBuffer(const GLuint &bufferBindingPoint)
{
    craneBase->setLightsBuffer(bufferBindingPoint);
    craneTower->setLightsBuffer(bufferBindingPoint);
    craneCounterWeight->setLightsBuffer(bufferBindingPoint);
    craneJib->setLightsBuffer(bufferBindingPoint);
    craneTrolley->setLightsBuffer(bufferBindingPoint);
}
