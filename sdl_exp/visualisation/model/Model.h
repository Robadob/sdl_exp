#ifndef __Model_h__
#define __Model_h__
#include "glm/glm.hpp"
#include <memory>
#include "ModelNode.h"
#include "../shader/Shaders.h"
#include "Material.h"
#include "BoundingBox.h"
#include "Animation.h"
#include "../shader/buffer/UniformBuffer.h"
#include <assimp/config.h>
#include "../shader/ShadersVec.h"
#include "../Draw.h"

struct VFCcount
{
	VFCcount(int i = 1) :v(0), f(0), c(i), b(0){}
	unsigned int v;//vertices
	unsigned int f;//faces
	unsigned int c;//components??
	unsigned int b;//bones
	VFCcount &operator +=(const VFCcount &rhs)
	{
		v += rhs.v;
		f += rhs.f;
		c += rhs.c;
		b += rhs.b;
		return *this;
	}
};

struct VertexBoneData
{
	VertexBoneData()
	{
		memset(weights, 0, sizeof(weights));//0
		memset(boneIds, 0, sizeof(boneIds));//0
	}
	void add(const float &weight, const unsigned int &boneId)
	{
		for (unsigned int i = 0; i < AI_LMW_MAX_WEIGHTS; ++i)
		{
			if (this->weights[i] == 0.0f)
			{
				this->weights[i] = weight;
				this->boneIds[i] = boneId;
				return;
			}
		}
		assert(false);//Too many bones attached to vert
	}
public:
	const float *Weights() const { return this->weights; }
	const unsigned int *BoneIds() const { return this->boneIds; }
	static const unsigned char COUNT = AI_LMW_MAX_WEIGHTS;
private:
	float weights[AI_LMW_MAX_WEIGHTS];
	unsigned int boneIds[AI_LMW_MAX_WEIGHTS];
};
struct ModelData
{
	ModelData(
		size_t vertices,
		size_t normals,
		size_t colors,
		size_t texcoords,
		size_t bones,
		size_t materials,
		size_t faces,
		size_t transforms
		)
        : vertices(nullptr)
        , normals(nullptr)
        , colors(nullptr)
        , texcoords(nullptr)
		, boneMatrices(nullptr)
        , materials(nullptr)
        , faces(nullptr)
		, transforms(nullptr)
		, _transforms(nullptr)
		, animations()
        , verticesSize(vertices)
		, normalsSize(normals)
		, colorsSize(colors)
		, texcoordsSize(texcoords)
		, bonesSize(bones)
		, materialsSize(materials)
		, facesSize(faces)
		, transformsSize(transforms)
		, boneData(new VertexBoneData[vertices]())
		, boneMapping()
		, meshDirectory()

	{
		this->vertices = static_cast<glm::vec3 *>(malloc(vertices * sizeof(glm::vec3)));
		if (normals)
			this->normals = static_cast<glm::vec3 *>(malloc(normals * sizeof(glm::vec3)));
		if (colors)
			this->colors = static_cast<glm::vec4 *>(malloc(colors * sizeof(glm::vec4)));
		if (texcoords)
			this->texcoords = static_cast<glm::vec3 *>(malloc(texcoords * sizeof(glm::vec3)));

		if (bones)
		{
			this->boneMatrices = static_cast<glm::mat4 *>(malloc(bones * sizeof(glm::mat4)));
			memset(this->boneMatrices, 0, bones * sizeof(glm::mat4));
			this->computedTransforms = static_cast<glm::mat4 *>(malloc(bones * sizeof(glm::mat4)));
			for (unsigned int i = 0; i < bones; ++i)
				this->computedTransforms[i] = glm::mat4(1);
		}

		if (materials)
        {
            this->materials = static_cast<std::shared_ptr<Material> *>(malloc(materials * sizeof(std::shared_ptr<Material>)));
            for (unsigned int i = 0; i < materials;++i)
                new(&this->materials[i]) std::shared_ptr<Material>();
		}

		this->faces = static_cast<unsigned int *>(malloc(faces * sizeof(unsigned int)));
		if (transforms)
		{
			this->transforms = static_cast<glm::mat4 *>(malloc(transforms * sizeof(glm::mat4)));
			memset(this->transforms, 0, transforms * sizeof(glm::mat4));
			this->_transforms = static_cast<glm::mat4 *>(malloc(transforms * sizeof(glm::mat4)));
			memset(this->_transforms, 0, transforms * sizeof(glm::mat4));
		}
	}
	~ModelData()
	{
		//Free memory
		free(vertices);
		free(normals);
		free(colors);
		free(texcoords);
		free(boneMatrices);
		free(computedTransforms);
        //Call destructor on materials
        for (unsigned int i = 0; i < materialsSize;++i)
            this->materials[i].~shared_ptr();
		free(materials);
		free(faces);
		free(transforms);
		free(_transforms);
		for (auto &a : animations)
			delete a;
		animations.clear();
		delete[]boneData;
	}
	//Vertex attributes
	glm::vec3 *vertices;
	glm::vec3 *normals;
	glm::vec4 *colors;
	glm::vec3 *texcoords;

	//Bones
	glm::mat4 *boneMatrices;//Bone offset matricies
	std::unordered_multimap<std::string, unsigned int> boneMapping;
	VertexBoneData *boneData;
	glm::mat4 *computedTransforms;

	std::vector<Animation *>animations;
	std::unordered_map<std::string, unsigned int> animationDirectory;

	//Materials
    std::shared_ptr<Material> *materials;

	//Component attributes
	unsigned int *faces;
	glm::mat4 *transforms;//bind pose transform
	glm::mat4 *_transforms;//last animation pose transform
	std::vector<unsigned int> rootChain;
	glm::mat4 inverseRootTransform;

    //Sizes
    size_t verticesSize;
    size_t normalsSize;
    size_t colorsSize;
    size_t texcoordsSize;
    size_t bonesSize;
    size_t materialsSize;
    size_t facesSize;
	size_t transformsSize;
	Animation &newAnimation(){ animations.push_back(new Animation()); return *animations[animations.size() - 1]; }
	std::unordered_map<std::string, std::weak_ptr<Mesh>> meshDirectory;
	std::unordered_map<std::string, std::weak_ptr<ModelNode>> nodeDirectory;
};
class Model : public RenderableAdv
{
public:
	/**
	* @param modelPath Path to the model desired to be loaded
	* @param scale The model will be uniformly scaled so that the largest dimension matches this value
	* @param scale Values <= 0.0 will leave the model at it's native scale.
	* @param setAllMeshesVisible Sets the default value of mesh visibility
	* @param setAllMeshesVisible This is useful for models which contain more mesh aliases than desired meshes
	*/
	Model(const char *modelPath, float scale = -1.0f, bool setAllMeshesVisible = true, std::vector<std::shared_ptr<Shaders>> shaders = std::vector<std::shared_ptr<Shaders>>());
	explicit Model(const char *modelPath, float scale, bool setAllMeshesVisible, std::initializer_list<const Stock::Shaders::ShaderSet> ss = {});
	~Model();
	/**
	* Reloads the model from file, rewriting GPU buffers
	*/
	void reload() override;
	//Rendering methods
	void update(float time);
	void render(const unsigned int &shaderIndex = UINT_MAX) override;
	void renderSkeleton();
    void setLocation(glm::vec3 location){ this->location = location; }
    void setRotation(glm::vec4 rotation){ this->rotation = rotation; }
    glm::vec3 getLocation() const override{ return location; }
    glm::vec4 getRotation() const{ return rotation; }
	BoundingBox3D getBoundingBox() const { return boundingBox; }
	std::unique_ptr<ShadersVec> getShaders(unsigned int shaderIndex = 0) const;
	std::shared_ptr<Material> getMaterial(unsigned int materialIndex = 0) const;
	unsigned int loadExternalAnimation(const std::string &path);
	/**
	 * Attempts to load animations from all files in the named directory
	 * Optionally pass the 2nd argument to specify the file extension required
	 * @param directory The directory used
	 * @param extension Optional file extension to filter for
	 */
	unsigned int loadExternalAnimations(const std::string &directory, const std::string extension = "");
	float getDistanceTravelled() const { return mDistanceTravelled;  };
	const char *getModelPath() const { return modelPath.c_str(); }
	/**
	 * Returns whether the named mesh exists within the model.
	 * @param meshName Name of the mesh to be checked (value must be exact, case sensitive)
	 * @return True if mesh exists within model
	 * @see setMeshVisible(const std::string &, const bool &)
	 * @see getMeshVisible(const std::string &)
	 * @note Nameless meshes cannot be targeted with this function
	 */
	bool hasMesh(const std::string &meshName);
	/**
	 * Sets the visibility flag for the named mesh
	 * @param meshName Name of the mesh to be checked (value must be exact, case sensitive)
	 * @param isVisible The new visibility flag, false hides the mesh during render.
	 * @note This method will fail silently if the named mesh does not exist
	 * @see hasMesh()
	 * @see getMeshVisible(const std::string &)
	 * @note Nameless meshes cannot be targeted with this function
	 */
	void setMeshVisible(const std::string &meshName, const bool &isVisible);
	/**
	 * Returns the visibility flag for the named mesh
	 * @param meshName Name of the mesh to be checked (value must be exact, case sensitive)
	 * @return The visbility flag of the named mesh, false hides the mesh during render.
	 * @note This method will fail silently if the named mesh does not exist
	 * @see hasMesh()
	 * @see setMeshVisible(const std::string &, const bool &)
	 * @note Nameless meshes cannot be targeted with this function
	 */
	bool getMeshVisible(const std::string &meshName);
	void nextAnimation(float transitionDuration = DEFAULT_KEYFRAME_TRANSITION_DURATION);
	void setAnimation(unsigned int index, float transitionDuration = DEFAULT_KEYFRAME_TRANSITION_DURATION);
	void setAnimation(const std::string &name, float transitionDuration = DEFAULT_KEYFRAME_TRANSITION_DURATION);

	void disableAnimationTravel(bool disable);

	void setEnvironmentMap(std::shared_ptr<const TextureCubeMap> cubeMap) override;
private:
	Draw skeletonPen;
	bool skeletonIsValid;
	static std::vector<std::shared_ptr<Shaders>> convertToShader(std::initializer_list<const Stock::Shaders::ShaderSet> ss)
	{
		std::vector<std::shared_ptr<Shaders>> rtn;
		for (auto&& s : ss)
			rtn.push_back(std::make_shared<Shaders>(s.vertex, s.fragment, s.geometry));
		return rtn;
	}
	glm::vec3 mPreviousLocation = glm::vec3(0);
	float mDistanceTravelled = 0.0f;
	unsigned int mActiveAnim = 0;
	float mAnimationTickOffset = 0;
	bool mTransitioningKeyframes = false;
	static const float DEFAULT_KEYFRAME_TRANSITION_DURATION;//seconds
	static const unsigned int DEFAULT_TICKS_PER_SECOND;
	Animation::NodeKeyMap mTransitionKeyFrame;
	float mTransitionDuration = DEFAULT_KEYFRAME_TRANSITION_DURATION;
	bool mDisableAnimationTravel = true;
	glm::vec3 mAnimationLocationOffset = glm::vec3(0);
	float mLastAnimationTime = 0.0f;
	/**
	 * This is triggered at the end of load for FBX models
	 * FBX models lack the inverseRootTransform as they don't officially have a root node
	 * This method iterates vertex bone transforms of the default animation to find the most popular and treats that as inverse root
	 */
	void computeInverseRootTransform();
	void updateBoneTransforms(float seconds);
    void updateBoundingBox();
	std::shared_ptr<ModelNode> buildHierarchy(const struct aiScene* scene, const struct aiNode* nd, VFCcount &vfc) const;
	/**
	 * @param filePath is used for naming animations
	 */
	unsigned int loadAnimationsFromScene(const struct aiScene *scene, const std::string &filePath);
	/**
	* Loads a model from file
	* @TODO Also add support for importing textures and materials
	*/
	void loadModel();
	void freeModel();

    BoundingBox3D boundingBox;

	std::shared_ptr<ModelNode> root;
	std::shared_ptr<ModelData> data;
	VFCcount vfc;
	const std::string modelPath;
    const float loadScale;//Scale that vertices are scaled to at model load
	//VBOs
	GLuint vbo;
	GLuint fbo;
	glm::vec3 location = glm::vec3(0);
	glm::vec4 rotation = glm::vec4(0);
	float scaleFactor = 1.0f;

	/**
	* Holds information for binding the vertex positions attribute
	*/
	Shaders::VertexAttributeDetail positions;
	/**
	* Holds information for binding the vertex normals attribute
	*/
	Shaders::VertexAttributeDetail normals;
	/**
	* Holds information for binding the vertex colours attribute
	*/
	Shaders::VertexAttributeDetail colors;
	/**
	* Holds information for binding the vertex texture coordinates attribute
	*/
	Shaders::VertexAttributeDetail texcoords;
	/**
	* Holds information for binding the bone id's attribute
	*/
	Shaders::VertexAttributeDetail boneIDs;
	/**
	* Holds information for binding the bone weights attribute
	*/
	Shaders::VertexAttributeDetail boneWeights;
	std::shared_ptr<UniformBuffer> boneBuffer;
	std::shared_ptr<UniformBuffer> materialBuffer;
	/**
	 * Custom shaders, shared by all materials
	 * e.g. for shadow etc
	 */
	std::vector<std::shared_ptr<Shaders>> shaders;
	//HasMatrices overrides
	const glm::mat4 *viewMatPtr;
	const glm::mat4 *projMatPtr;
	GLuint lightsBufferBindPt;
public:
	glm::mat4 getModelMat() const;
	/**
	* Sets the pointer from which the View matrix should be loaded from
	* @param viewMat A pointer to the viewMatrix to be tracked
	* @note This pointer is likely provided by a Camera subclass
	*/
	void setViewMatPtr(const glm::mat4  *viewMat) override;
	/**
	* Sets the pointer from which the Projection matrix should be loaded from
	* @param projectionMat A pointer to the projectionMatrix to be tracked
	* @note This pointer is likely provided by the Visualisation object
	*/
	void setProjectionMatPtr(const glm::mat4 *projectionMat) override;
	/**
	* Provides lights buffer to the shader
	* @param bufferBindingPoint Set the buffer binding point to be used for rendering
	*/
	void setLightsBuffer(GLuint bufferBindingPoint) override;
};

#endif //__Model_h__