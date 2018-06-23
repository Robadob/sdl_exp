#ifndef __Animation_h__
#define __Animation_h__
#include <unordered_map>
#include <glm/gtx/quaternion.hpp>

class Animation
{
public:
	struct NodeAnimation;
	struct MeshAnimation;
	struct MeshMorphAnimation;
	typedef std::unordered_map<std::string, NodeAnimation *> NodeKeyMap;
	typedef std::unordered_map<std::string, MeshAnimation *> MeshKeyMap;
	typedef std::unordered_map<std::string, MeshMorphAnimation *> MeshMorphKeyMap;
	struct NodeAnimation
	{
		/**
		 * These are not currently handled, unsure of exact intention (repeat appears independent of constant/linear)
		 * Would have expected constant as post state means last frame remains unchanged after it's reached
		 * Need a test mesh which sets them to better understand
		 */
		enum Behaviour
		{
			/** The value from the default node transformation is taken*/
			DEFAULT = 0x0,

			/** The nearest key value is used without interpolation */
			CONSTANT = 0x1,

			/** The value of the nearest two keys is linearly
			*  extrapolated for the current time value.*/
			LINEAR = 0x2,

			/** The animation is repeated.
			*
			*  If the animation key go from n to m and the current
			*  time is t, use the value at (t-n) % (|m-n|).*/
			REPEAT = 0x3,
		};
		struct Vec3Key
		{
			float time;
			glm::vec3 vec3;
		};
		struct RotationKey
		{
			float time;
			glm::quat rotation;
		};
		unsigned int positionKeyCount = 0;
		Vec3Key *positionKeys = nullptr;
		unsigned int rotationKeyCount = 0;
		RotationKey *rotationKeys = nullptr;
		unsigned int scalingKeyCount = 0;
		Vec3Key *scalingKeys = nullptr;
		Behaviour preState = DEFAULT;
		Behaviour postState = DEFAULT;
		NodeAnimation(unsigned int p, unsigned int r, unsigned int s);
		~NodeAnimation();
		glm::vec3 calcInterpolatedScaling(float time) const;
		glm::quat calcInterpolatedRotation(float time) const;
		glm::vec3 calcInterpolatedTranslation(float time) const;
		glm::vec3 calcInterpolatedScalingTo(
			const unsigned int &iStart,
			const NodeAnimation *end,
			const unsigned int &iEnd,
			const float &factor) const;
		glm::quat calcInterpolatedRotationTo(
			const unsigned int &iStart,
			const NodeAnimation *end,
			const unsigned int &iEnd,
			const float &factor) const;
		glm::vec3 calcInterpolatedTranslationTo(
			const unsigned int &iStart,
			const NodeAnimation *end,
			const unsigned int &iEnd,
			const float &factor) const;
	};
	struct MeshAnimation
	{
		//You may think of an MeshAnimation as a patch for the host mesh, which replaces only certain vertex data streams at a particular time.
		struct Key
		{
			float time;
			/** Index into the aiMesh::mAnimMeshes array of the
			*  mesh coresponding to the #aiMeshAnim hosting this
			*  key frame. The referenced anim mesh is evaluated
			*  according to the rules defined in the docs for #aiAnimMesh.*/
			unsigned int animMeshId;
		};
		unsigned int keyCount = 0;
		Key *meshKeys = nullptr;
		MeshAnimation(unsigned int count);
		~MeshAnimation();
	};
	struct MeshMorphAnimation
	{
		struct Key
		{
			
			Key(unsigned int count, float time);
			~Key();
			float time;
			unsigned int count;
			unsigned int *values;
			float *weights;
		};
		unsigned int keyCount = 0;
		Key *meshKeys = nullptr;
		MeshMorphAnimation(unsigned int count);
		~MeshMorphAnimation();
	};
public:
	~Animation();
	std::string name;
	float duration;
	float ticksPerSecond;
	NodeKeyMap nodeAnims;
	MeshKeyMap meshAnims;
	MeshMorphKeyMap meshMorphAnims;
};

#endif //__Animation_h__