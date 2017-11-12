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
	: craneBase(Entity::load(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
	, craneTower(Entity::load(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
	, craneCounterWeight(Entity::load(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
	, craneJib(Entity::load(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
	, craneTrolley(Entity::load(Stock::Models::CUBE, 1.0f, Stock::Shaders::FLAT))
	, spinJoint(SGJoint::make())
	, trolleySlideJoint(SGJoint::make())
	, scale(scale)
	, location(location)
{
	if (!(craneBase&&craneTower&&craneCounterWeight&&craneJib&&craneTrolley))
		throw std::exception("Crane was unable to load all entities!");
	craneBase->setColor(glm::vec3(1));
	craneTower->setColor(glm::vec3(0.95, 1, 0.25));
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

void TowerCrane::setupCraneSceneGraph()
{
	const float BASE_HEIGHT = 4;
	const float BASE_WIDTH = 10;
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
	}
	//Crane Tower
	{
		glm::mat4 mm = glm::mat4(1);
		//Scale to size
		mm *= glm::scale(glm::vec3(1.0f / BASE_WIDTH, 20 / BASE_HEIGHT, 1.0f / BASE_WIDTH));
		//Translate so bottom centre is at origin
		mm *= glm::translate(glm::vec3(0, 0.5, 0));
		//Setup
		craneTower->setModelMat(mm);
		//Add to Scene Graph (at top centre of base)
		craneBase->attach(craneTower, "crane_tower", glm::vec3(0, 1 ,0));
	}

}
