#include "Model.h"
#include "../util/StringUtils.h"
#include "Model_assimpUtils.h"
#include <glm/gtc/epsilon.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/hash.hpp>
#include <filesystem>
#include "../texture/Texture2D.h"


const float Model::DEFAULT_KEYFRAME_TRANSITION_DURATION = 0.4f;//seconds
const unsigned int Model::DEFAULT_TICKS_PER_SECOND = 25;

Model::Model(const char *modelPath, float scale, bool setAllMeshesVisible, std::vector<std::shared_ptr<Shaders>> shaders)
	: skeletonIsValid(false)
	, root(nullptr)
	, data(nullptr)
	, modelPath(modelPath)
	, loadScale(scale)
	, positions(GL_FLOAT, 3, sizeof(float))
	, normals(GL_FLOAT, 3, sizeof(float))
	, colors(GL_FLOAT, 4, sizeof(float))
	, texcoords(GL_FLOAT, 3, sizeof(float))
	, boneIDs(GL_UNSIGNED_INT, 4, sizeof(unsigned int))
	, boneWeights(GL_FLOAT, 4, sizeof(float))
	, boneBuffer(nullptr)
	, shaders(shaders)
	, viewMatPtr(nullptr)
	, projMatPtr(nullptr)
	, lightsBufferBindPt(-1)
{
	loadModel();
	if (data)
	{
		for (auto &a : data->meshDirectory)
		{
			if (auto b = a.second.lock())
			{
				b->setVisible(setAllMeshesVisible);
			}
		}
		//Init mTransitonKeyFrame, create an item for each node
		for (auto &a : data->nodeDirectory)
		{
			mTransitionKeyFrame[a.first] = new Animation::NodeAnimation(1, 1, 1);
		}
	}
}
Model::Model(const char *modelPath, float scale, bool setAllMeshesVisible, std::initializer_list<const Stock::Shaders::ShaderSet> ss)
	: Model(modelPath, scale, setAllMeshesVisible, convertToShader(ss))
{ }
Model::~Model()
{
	freeModel();
	//Purge mTransitonKeyFrame
	for (auto &a : mTransitionKeyFrame)
	{
		delete a.second;
	}
}
void Model::freeModel()
{
    //Clear hierarchy
    root.reset();
    //Clear data
    data.reset();
    //Release VBOs
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &fbo);
}
void Model::reload()
{
	//Cache old mesh directory values
	std::unordered_map<std::string, bool> oldMeshDirectory;
	for (auto &a : data->meshDirectory)
		if (auto b = a.second.lock())
			oldMeshDirectory.insert({ a.first, b->getVisible() });
	//Reload model
    freeModel();
    loadModel();
	//Reconfigure new mesh directory to match old
	if (data)
	{
		for (auto &a : data->meshDirectory)
		{
			auto b = oldMeshDirectory.find(a.first);
			if (b != oldMeshDirectory.end())
				if (auto c = a.second.lock())
					c->setVisible(b->second);
		}
		//Can't refresh viewMat, projMat here, breaks custom shaders which have alts
		//Would have to bind those alts as dynamicMat4 instead
		//Only update default shader for each material
		for (unsigned int i = 0; i < data->materialsSize; ++i)
		{
			auto &&m = data->materials[i]->getShaders();
			m->setProjectionMatPtr(projMatPtr);
			m->setViewMatPtr(viewMatPtr);
			if (lightsBufferBindPt >= 0)
				m->setLightsBuffer(lightsBufferBindPt);
			if (environmentMap)
				data->materials[i]->setEnvironmentMap(environmentMap);
		}
	}
	//Need to update external textures?
	//e.g. environment map
}
VFCcount countVertices(const struct aiScene* scene, const struct aiNode* nd)
{
    VFCcount vfc;
    for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		vfc.v += mesh->mNumVertices;
		vfc.b += mesh->mNumBones;
        for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
            vfc.f += mesh->mFaces[t].mNumIndices;
#ifdef _DEBUG
            //All faces per mesh are of the same type
            assert(mesh->mFaces[0].mNumIndices == mesh->mFaces[t].mNumIndices);
#endif
		}
    }

    for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		vfc += countVertices(scene, nd->mChildren[n]);
	}
    return vfc;
}
void Model::updateBoundingBox()
{
	boundingBox.reset();
	if (this->root)
	{
		boundingBox.include(this->root->calculateBoundingBox());
	}
}
//Loading
unsigned int Model::loadAnimationsFromScene(const struct aiScene *scene, const std::string &filePath)
{
	for (unsigned int i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation *b = scene->mAnimations[i];
		{//Validate animation matches model
			unsigned int badChannels = 0;
			for (unsigned int j = 0; j < b->mNumChannels; ++j)
			{
				if (data->nodeDirectory.find(b->mChannels[j]->mNodeName.data) == data->nodeDirectory.end())
				{//Channel name not found in mesh!
					badChannels++;
				}
			}
			if (badChannels)
			{
				fprintf(stderr, "Cannot load animation '%s', contains %d/%d node channels not found in model!\n", b->mName.data, badChannels, b->mNumChannels);
				continue;
			}
			badChannels = 0;
			for (unsigned int j = 0; j < b->mNumMeshChannels; ++j)
			{
				if (data->meshDirectory.find(b->mMeshChannels[j]->mName.data) == data->meshDirectory.end())
				{//Channel name not found in mesh!
					badChannels++;
				}
			}
			if (badChannels)
			{
				fprintf(stderr, "Cannot load animation '%s', contains %d/%d mesh channels not found in model!\n", b->mName.data, badChannels, b->mNumMeshChannels);
				continue;
			}
			badChannels = 0;
			for (unsigned int j = 0; j < b->mNumMorphMeshChannels; ++j)
			{
				if (data->meshDirectory.find(b->mMorphMeshChannels[j]->mName.data) == data->meshDirectory.end())
				{//Channel name not found in mesh!
					badChannels++;
				}
			}
			if (badChannels)
			{
				fprintf(stderr, "Cannot load animation '%s', contains %d/%d mesh morph channels not found in model!\n", b->mName.data, badChannels, b->mNumMorphMeshChannels);
				continue;
			}
		}
		Animation &a = data->newAnimation();
		a.name = std::string(b->mName.data);
		a.duration = (float)b->mDuration;
		a.ticksPerSecond = (float)b->mTicksPerSecond;
		for (unsigned int j = 0; j < b->mNumChannels; ++j)
		{
			aiNodeAnim *c = b->mChannels[j];
			Animation::NodeAnimation *na = new Animation::NodeAnimation(c->mNumPositionKeys, c->mNumRotationKeys, c->mNumScalingKeys);
			for (unsigned int k = 0; k < c->mNumPositionKeys; ++k)
			{
				na->positionKeys[k].time = (float)c->mPositionKeys[k].mTime;
				na->positionKeys[k].vec3 = *reinterpret_cast<glm::vec3*>(&c->mPositionKeys[k].mValue);
			}
			for (unsigned int k = 0; k < c->mNumRotationKeys; ++k)
			{
				na->rotationKeys[k].time = (float)c->mRotationKeys[k].mTime;
				auto &q = c->mRotationKeys[k].mValue;//aiQuat storage begins w, glm storage ends w
				na->rotationKeys[k].rotation = glm::quat((float)q.w, (float)q.x, (float)q.y, (float)q.z);
			}
			for (unsigned int k = 0; k < c->mNumScalingKeys; ++k)
			{
				na->scalingKeys[k].time = (float)c->mScalingKeys[k].mTime;
				na->scalingKeys[k].vec3 = *reinterpret_cast<glm::vec3*>(&c->mScalingKeys[k].mValue);
			}
			na->preState = Animation::NodeAnimation::Behaviour((int)c->mPreState);
			na->postState = Animation::NodeAnimation::Behaviour((int)c->mPostState);
			a.nodeAnims[std::string(c->mNodeName.data)] = na;
		}
		if (b->mNumMeshChannels)
			fprintf(stderr, "Animation '%s' contains mesh channels, these are not currently supported.\n", b->mName.data);
		for (unsigned int j = 0; j < b->mNumMeshChannels; ++j)
		{
			aiMeshAnim *c = b->mMeshChannels[j];
			Animation::MeshAnimation *ma = new Animation::MeshAnimation(c->mNumKeys);
			for (unsigned int k = 0; k < c->mNumKeys; ++k)
			{
				ma->meshKeys[k].time = (float)c->mKeys[k].mTime;
				ma->meshKeys[k].animMeshId = c->mKeys[k].mValue;
			}
			a.meshAnims[std::string(c->mName.data)] = ma;
		}
		if (b->mNumMorphMeshChannels)
			fprintf(stderr, "Animation '%s' contains morph mesh channels, these are not currently supported.\n", b->mName.data);
		for (unsigned int j = 0; j < b->mNumMorphMeshChannels; ++j)
		{
			aiMeshMorphAnim *c = b->mMorphMeshChannels[j];
			Animation::MeshMorphAnimation *ma = new Animation::MeshMorphAnimation(c->mNumKeys);
			for (unsigned int k = 0; k < c->mNumKeys; ++k)
			{
				Animation::MeshMorphAnimation::Key* mak = new(c->mKeys + k)Animation::MeshMorphAnimation::Key(c->mKeys[k].mNumValuesAndWeights, (float)c->mKeys[k].mTime);
				for (unsigned int _k = 0; _k < mak->count; ++_k)
				{
					mak->values[_k] = c->mKeys[k].mValues[_k];
					mak->weights[_k] = (float)c->mKeys[k].mWeights[_k];
				}
			}
			a.meshMorphAnims[std::string(c->mName.data)] = ma;
		}
		//Add animation to directory
		if (scene->mNumAnimations==1)
		{//name is filename sans extension
			data->animationDirectory[su::removeFileExt(su::getFilenameFromPath(filePath))] = (unsigned int)data->animations.size() - 1;
		}
		else
		{//name is filename sans extension.animation name (some animations are just names 'take_001', so loading many from seperate files would collide)
			std::string animName = su::format("%s_%s", su::removeFileExt(su::getFilenameFromPath(filePath)).c_str(), b->mName.data);
			data->animationDirectory[animName] = (unsigned int)data->animations.size() - 1;
		}
	}
	return scene->mNumAnimations;
}
std::shared_ptr<ModelNode> Model::buildHierarchy(const struct aiScene* scene, const struct aiNode* nd, VFCcount &vfc) const
{
    //Copy transformation matrix
    aiMatrix4x4 m = nd->mTransformation;
#ifdef _DEBUG
    //aiMatrix4x4 is type float
    assert(sizeof(glm::mat4) == sizeof(aiMatrix4x4));
#endif
    //Assimp row major -> GLM column major 
	data->transforms[vfc.c] = transpose(*reinterpret_cast<glm::mat4*>(&m));
	
    //Create the ModelNode for this component, add to nodeDirectory (and increment data offset)
    std::shared_ptr<ModelNode> rtn = ModelNode::make_shared(this->data, vfc.c++, nd->mName.C_Str());
	data->nodeDirectory[rtn->getName()] = rtn;
    //Fill it with meshes
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* _aiMesh = scene->mMeshes[nd->mMeshes[n]];    		
		if (_aiMesh->mNumFaces == 0) continue;
		//Create a new mesh
		GLenum faceType;
		unsigned int mNumFaceIndices = _aiMesh->mFaces[0].mNumIndices;
		switch (mNumFaceIndices) {
		case 1: faceType = GL_POINTS; break;
		case 2: faceType = GL_LINES; break;
		case 3: faceType = GL_TRIANGLES; break;
		case 4: faceType = GL_QUADS;
			fprintf(stderr, "Warning: GL_QUADS is deprecated, this should be replaced with GL_PATCH and tesselation shader.\n");
			break;
		default: faceType = GL_POLYGON;
			fprintf(stderr, "Warning: GL_POLYGON is deprecated, this should be replaced with GL_PATCH and tesselation shader.\n");
			break;
		}

		std::shared_ptr<Mesh> mesh = Mesh::make_shared(
			_aiMesh->mName.C_Str(),
			this->data,
			vfc.f*sizeof(unsigned int),
			_aiMesh->mNumFaces*mNumFaceIndices,
			_aiMesh->mMaterialIndex,
			faceType,
			vfc.b,//Bone offset
			_aiMesh->mNumBones
			);

#ifdef _DEBUG
		//aiVector3D is type float
		assert(sizeof(glm::vec3) == sizeof(aiVector3D));
		//aiColor4D is type float
		assert(sizeof(glm::vec4) == sizeof(aiColor4D));
#endif
		//Copy data for each face
		//Vertex
		memcpy(data->vertices + vfc.v, _aiMesh->mVertices, _aiMesh->mNumVertices*sizeof(glm::vec3));
		//Normal
		if (data->normals)
			memcpy(data->normals + vfc.v, _aiMesh->mNormals, _aiMesh->mNumVertices*sizeof(glm::vec3));
		//Color (just take the first set)
		if (data->colors)
			memcpy(data->colors + vfc.v, _aiMesh->mColors[0], _aiMesh->mNumVertices*sizeof(glm::vec4));
		//Texture Coordinates (just take the first set)
		if (data->texcoords)
			memcpy(data->texcoords + vfc.v, _aiMesh->mTextureCoords[0], _aiMesh->mNumVertices*sizeof(glm::vec3));
		//Map face indexes
		for (unsigned int t = 0; t < _aiMesh->mNumFaces; ++t) {
			const struct aiFace* face = &_aiMesh->mFaces[t];
			for (unsigned int i = 0; i < face->mNumIndices; ++i) {
				int index = face->mIndices[i];
				data->faces[vfc.f + (t*mNumFaceIndices) + i] = vfc.v + index;
			}
		}

		//Bones
		//Assign each bone an index in the array, ignoring those with duplicate names
		for (unsigned int t = 0; t<_aiMesh->mNumBones; ++t)
		{
			const aiBone *bn = _aiMesh->mBones[t];
			unsigned int id = (unsigned int)data->boneMapping.size();
			data->boneMapping.insert({ std::string(bn->mName.data), id });
			//Store the bone's index and weight in a vertex array
			for (unsigned int i = 0; i < bn->mNumWeights; ++i)
			{
				data->boneData[vfc.v + bn->mWeights[i].mVertexId].add(bn->mWeights[i].mWeight, id);
			}
			//Assimp row major -> GLM column major
			if (data->boneMatrices[id] == glm::mat4(0))
				data->boneMatrices[id] = transpose(*reinterpret_cast<const glm::mat4*>(&bn->mOffsetMatrix));
		}
		//Increment data offsets
		vfc.v += _aiMesh->mNumVertices;
		vfc.b += _aiMesh->mNumBones;
		vfc.f += _aiMesh->mNumFaces*mNumFaceIndices;

		//Link mesh to hierarchy
		rtn->addMesh(mesh);
		if (!mesh->getName().empty())
		{//Ignore nameless meshes, they are unlikely to be aliased
			if (data->meshDirectory.find(mesh->getName()) == data->meshDirectory.end())
			{
				data->meshDirectory.insert({ mesh->getName(), std::weak_ptr<Mesh>(mesh) });
			}
			else
			{
				fprintf(stderr, "Warning: Multiple meshes in model '%s' share name '%s'. Mesh visibility toggle may fail.\n", this->modelPath.c_str(), mesh->getName().c_str());
			}
		}
	}
    //Recursivly fill children
    for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
        std::shared_ptr<ModelNode> child = buildHierarchy(scene, nd->mChildren[n], vfc);
        rtn->addChild(child);
        //child->setParent(rtn);
    }
    return rtn;
}
void Model::loadModel()
{
	printf("\rLoading Model: %s ", su::getFilenameFromPath(modelPath).c_str());
    //Import model with assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelPath, aiProcessPreset_TargetRealtime_MaxQuality);

	if (!scene)
	{
#ifdef _DEBUG
		fprintf(stderr, "\rError Model load failed '%s'\n%s\n", modelPath.c_str(), importer.GetErrorString());
#else
		throw std::runtime_error(su::format("\rError Model load failed '%s'\n%s\n", modelPath.c_str(), importer.GetErrorString()).c_str());
#endif
	}

	printf("\rLoading Model: %s [Parsing Assimp Model Data]      ", su::getFilenameFromPath(modelPath).c_str());
    //Calculate total number of vertices, faces, components and bones within hierarchy
    {
		this->vfc = countVertices(scene, scene->mRootNode);
		//Allocate memory
		this->data = std::make_shared<ModelData>(
			vfc.v,
			scene->mMeshes[0]->HasNormals() ? vfc.v : 0,
			scene->mMeshes[0]->mColors[0] != nullptr ? vfc.v : 0,
			scene->mMeshes[0]->HasTextureCoords(0) ? vfc.v : 0,
			vfc.b,
			scene->mNumMaterials,
			vfc.f,
			vfc.c
		);
    }

    //Store count in VADs
    positions.count = this->vfc.v;
    normals.count = normals.data ? this->vfc.v : 0;
    colors.count = colors.data ? this->vfc.v : 0;
	texcoords.count = texcoords.data ? this->vfc.v : 0;
	boneIDs.count = data->bonesSize ? this->vfc.v : 0;
	boneWeights.count = data->bonesSize ? this->vfc.v : 0;
    positions.data = data->vertices;
    normals.data = data->normals;
    colors.data = data->colors;
	texcoords.data = data->texcoords;
	boneIDs.data = nullptr;//Not store in right format, swap before create buffer
	boneWeights.data = nullptr;//Not store in right format, swap before create buffer

	printf("\rLoading Model: %s [Parsing Assimp Material Data]      ", su::getFilenameFromPath(modelPath).c_str());
    if (scene->HasTextures()){ fprintf(stderr, "Model '%s' has embedded textures, these are currently unsupported.\n", modelPath.c_str()); }
	std::string modelFolder = su::getFolderFromPath(modelPath);
    //Copy materials
	materialBuffer = std::make_shared<UniformBuffer>(sizeof(MaterialProperties)*data->materialsSize);
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
		//Create new blank material
		data->materials[i] = std::make_shared<Material>(materialBuffer, i, "", boneWeights.count!=0);
		//Setup basic material properties
		au::getMaterialProps(data->materials[i], scene->mMaterials[i]);
		//Textures
		for (unsigned int tType = 0; tType < AI_TEXTURE_TYPE_MAX; ++tType)
		{
			const aiTextureType textureType = aiTextureType(tType);
			unsigned int texCount = scene->mMaterials[i]->GetTextureCount(textureType);
			for (unsigned int texIndex = 0; texIndex < texCount; texIndex++)
			{
				Material::TextureFrame frame = au::getTextureProps(textureType, scene->mMaterials[i], texIndex, modelFolder.c_str());
				if (frame.texture)//If texture was loaded correctly
					data->materials[i]->addTexture(frame, au::toTexType_internal(textureType));
			}
		}
		data->materials[i]->bake();
    }

    //Convert assimp hierarchy to our custom hierarchy
    VFCcount rootCount = VFCcount(0);    
	root = buildHierarchy(scene, scene->mRootNode, rootCount);
	data->inverseRootTransform = glm::inverse(data->transforms[0]);//Default Inverse Root	
	this->root->constructRootChain(data->rootChain);

	printf("\rLoading Model: %s [Parsing Assimp Animation Data]       ", su::getFilenameFromPath(modelPath).c_str());
	loadAnimationsFromScene(scene, modelPath);
	
    //Calculate model scale
    updateBoundingBox();

    //Scale model
	if (loadScale>0)
	{
		const float maxDim = compMax(boundingBox.size());
		this->scaleFactor = 1.0f / (maxDim / loadScale);
	}

	printf("\rLoading Model: %s [Parsing Assimp Filling Buffers]       ", su::getFilenameFromPath(modelPath).c_str());
    //Calc VBO size
    size_t vboSize = 0;
    vboSize += this->vfc.v*sizeof(glm::vec3);
    if (data->normals)
        vboSize += this->vfc.v*sizeof(glm::vec3);
    if (data->colors)
        vboSize += this->vfc.v*sizeof(glm::vec4);
    if (data->texcoords)
        vboSize += this->vfc.v*sizeof(glm::vec3);
	if (data->bonesSize)
	{
		vboSize += this->vfc.v*sizeof(glm::uvec4);
		vboSize += this->vfc.v*sizeof(glm::vec4);
	}

    //Build VBO from data
    GL_CALL(glGenBuffers(1, &vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, vboSize, nullptr, GL_STATIC_DRAW));
    //Copy each vertex attrib to subBuffer location
    vboSize = 0;
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, vboSize, this->vfc.v*sizeof(glm::vec3), data->vertices));
    vboSize += this->vfc.v*sizeof(glm::vec3);
    if (data->normals)
    {
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, vboSize, this->vfc.v*sizeof(glm::vec3), data->normals));
        normals.offset = (unsigned int)vboSize;
        vboSize += this->vfc.v*sizeof(glm::vec3);
    }
    if (data->colors)
    {
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, vboSize, this->vfc.v*sizeof(glm::vec4), data->colors));
		colors.offset = (unsigned int)vboSize;
        vboSize += this->vfc.v*sizeof(glm::vec4);
    }
    if (data->texcoords)
    {
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, vboSize, this->vfc.v*sizeof(glm::vec3), data->texcoords));
		texcoords.offset = (unsigned int)vboSize;
        vboSize += this->vfc.v*sizeof(glm::vec3);
	}
	if (data->bonesSize)
	{
		assert(VertexBoneData::COUNT == 4);//Required for this shader config
		glm::uvec4 *u_buff = (glm::uvec4 *)malloc(this->vfc.v*sizeof(glm::uvec4));
		glm::vec4 *v_buff = (glm::vec4 *)malloc(this->vfc.v*sizeof(glm::vec4));
		for (unsigned int i = 0; i < this->vfc.v;++i)
		{
			u_buff[i] = *reinterpret_cast<const glm::uvec4*>(data->boneData[i].BoneIds());
			v_buff[i] = *reinterpret_cast<const glm::vec4*>(data->boneData[i].Weights());
		}
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, vboSize, this->vfc.v*sizeof(glm::uvec4), u_buff));
		boneIDs.offset = (unsigned int)vboSize;
		vboSize += this->vfc.v*sizeof(glm::uvec4);
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, vboSize, this->vfc.v*sizeof(glm::vec4), v_buff));
		boneWeights.offset = (unsigned int)vboSize;
		vboSize += this->vfc.v*sizeof(glm::vec4);
		free(u_buff);
		free(v_buff);
		boneBuffer = std::make_shared<UniformBuffer>(sizeof(glm::mat4)*this->vfc.b, data->computedTransforms);
	}
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Store VBO is VADs
    positions.vbo = vbo;
    if (data->normals)
        normals.vbo = vbo;
    if (data->colors)
        colors.vbo = vbo;
    if (data->texcoords)
        texcoords.vbo = vbo;
	if (data->bonesSize)
	{
		boneIDs.vbo = vbo;
		boneWeights.vbo = vbo;
	}
    //Build FBO
    GL_CALL(glGenBuffers(1, &fbo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->vfc.f*sizeof(unsigned int), data->faces, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	//Check vertex weights make sense
	for (unsigned int i = 0; i < data->verticesSize;++i)
	{
		float c = 0.0f;
		for (unsigned int j = 0; j < data->boneData->COUNT;++j)
		{
			c += data->boneData[i].Weights()[j];
		}
		assert(glm::epsilonEqual(c, 1.0f, 0.001f) || glm::epsilonEqual(c, 0.0f, 0.001f));
	}

	printf("\rLoading Model: %s [Performing First Animation]           ", su::getFilenameFromPath(modelPath).c_str());
	//Fbx needs special handling...
	//Handle missing inverseRootTransform
	updateBoneTransforms(0);
	if (su::endsWith(modelPath, ".fbx", false))
		computeInverseRootTransform();

	//printf("\rLoading Model: %s [Configuring Shaders]           ", su::getFilenameFromPath(modelPath).c_str());
	printf("\rLoading Model: %s [Complete!]                           \n", su::getFilenameFromPath(modelPath).c_str());
	//Report complete early because shader setup can write to console.
	//Inform shaders
	for (auto &s : shaders)
	{
		s->setPositionsAttributeDetail(positions);
		s->setNormalsAttributeDetail(normals);
		s->setColorsAttributeDetail(colors);
		s->setTexCoordsAttributeDetail(texcoords);
		s->setFaceVBO(fbo);
		//Set bone weights
		if (data->bonesSize)
		{
			s->addGenericAttributeDetail("_boneIDs", boneIDs);
			s->addGenericAttributeDetail("_boneWeights", boneWeights);
			s->addBuffer("_bones", boneBuffer);
		}
		s->setMaterialBuffer(materialBuffer);//Redundant (Material re-sets this when we set custom shaders).
	}
	for (unsigned int i = 0; i < data->materialsSize; ++i)
	{
		auto s = data->materials[i]->getShaders();
		s->setPositionsAttributeDetail(positions);
		s->setNormalsAttributeDetail(normals);
		s->setColorsAttributeDetail(colors);
		s->setTexCoordsAttributeDetail(texcoords);
		s->setFaceVBO(fbo);
		//Set bone weights
		if (data->bonesSize)
		{
			s->addGenericAttributeDetail("_boneIDs", boneIDs);
			s->addGenericAttributeDetail("_boneWeights", boneWeights);
			s->addBuffer("_bones", boneBuffer);
		}
		data->materials[i]->setCustomShaders(shaders);//Clone shaders into every material, so they can bind their own textures etc
	}
}
unsigned int Model::loadExternalAnimation(const std::string &path)
{
	if (!this->root)
		return 0;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (!scene)
	{
		fprintf(stderr, "Error: Animation load failed '%s'\n%s\n", path.c_str(), importer.GetErrorString());
		return 0;
	}
	return loadAnimationsFromScene(scene, path);
}
unsigned int Model::loadExternalAnimations(const std::string &directory, const std::string extension)
{
	if (!this->root)
		return 0;
	unsigned int count = 0;
#ifdef _MSC_VER
	using namespace std::tr2::sys;
	for(directory_iterator next(path(directory.c_str())), end;next!=end; ++next)
	{
		std::string filePath = next->path().string();
#else
	for (auto & p : std::filesystem::directory_iterator(directory))
	{
		std::string filePath = p;
#endif
		if(extension.empty() || su::endsWith(filePath, extension, false))
		{
			count += loadExternalAnimation(filePath);
		}
	}
	return count;
}
//Rendering
void Model::update(float time)
{
#if _DEBUG
	if (!this->root)
	{
		return;
	}
#endif
	updateBoneTransforms(time);
}
void Model::render(const unsigned int &shaderIndex)
{
	if (!mVisible)
		return;
#if _DEBUG
    static bool aborted = false;
    if (!this->root)
    {
        if (!aborted)
            fprintf(stderr, "Model '%s' was not found, render aborted!\n", modelPath.c_str());
        aborted = true;
        return;
    }
#endif

	for (unsigned int i = 0; i < data->materialsSize; ++i)
		data->materials[i]->prepare(shaderIndex);

    Material::clearActive();

    //Trigger recursive render
	root->render(getModelMat(), shaderIndex);

	//Clear shaders (only need to do this once, due to shared vao)
	for (unsigned int i = 0; i < data->materialsSize && i < 1; ++i)
		data->materials[i]->clear(shaderIndex);
}
void Model::renderSkeleton()
{
	if (!mVisible)
		return;
#if _DEBUG
	if (!this->root)
	{
		return;
	}
#endif 
	if (!skeletonIsValid)
	{
		skeletonPen.begin(Draw::Lines, "skeleton");
		//Trigger recursive render
		root->renderSkeleton(skeletonPen, getModelMat());
		skeletonPen.save(true);
		skeletonIsValid = true;
	}
	skeletonPen.render("skeleton");
}

//HasMatrices overrides
void Model::setViewMatPtr(const glm::mat4 *viewMat)
{
	viewMatPtr = viewMat;
	if (data)
	{
		for (unsigned int i = 0; i < data->materialsSize; ++i)
			data->materials[i]->setViewMatPtr(viewMat);
		for (auto &s : shaders)
			s->setViewMatPtr(viewMat);
		skeletonPen.setViewMatPtr(viewMat);
	}
}
void Model::setProjectionMatPtr(const glm::mat4 *projectionMat)
{
	projMatPtr = projectionMat;
	if (data)
	{
		for (unsigned int i = 0; i < data->materialsSize; ++i)
			data->materials[i]->setProjectionMatPtr(projectionMat);
		for (auto &s : shaders)
			s->setProjectionMatPtr(projectionMat);
		skeletonPen.setProjectionMatPtr(projectionMat);
	}
}
void Model::setLightsBuffer(GLuint bufferBindingPoint)
{
	lightsBufferBindPt = bufferBindingPoint;
	if (data)
	{
		for (unsigned int i = 0; i < data->materialsSize; ++i)
			data->materials[i]->setLightsBuffer(bufferBindingPoint);
		for (auto &s : shaders)
			s->setLightsBuffer(bufferBindingPoint);
		skeletonPen.setLightsBuffer(bufferBindingPoint);
	}
}
std::unique_ptr<ShadersVec> Model::getShaders(unsigned int shaderIndex) const
{
	std::unique_ptr<ShadersVec> s = std::make_unique<ShadersVec>();
	//Add local copy
	if (shaderIndex<shaders.size())
		s->add(shaders[shaderIndex]);
	for (unsigned int i = 0; i < data->materialsSize; ++i)
		s->add(data->materials[i]->getShaders(shaderIndex));
	return s;
}
std::shared_ptr<Material> Model::getMaterial(unsigned int materialIndex) const
{
	if (data)
	{
		if (data->materialsSize>materialIndex)
			return data->materials[materialIndex];
	}
	return nullptr;
}
//Animation stuff
void Model::nextAnimation(float transitionDuration)
{
	if (!data)
		return;  
	setAnimation(mActiveAnim + 1 < data->animations.size() ? mActiveAnim + 1 : 0, transitionDuration);
}
void Model::setAnimation(unsigned int index, float transitionDuration)
{
	assert(transitionDuration >= 0.0f);
	if (index<data->animations.size())
	{
		mTransitionDuration = transitionDuration;
		//Start transition to next animation
		mTransitioningKeyframes = true;
		//Find ticks
		float ticksPerSecond = data->animations[mActiveAnim]->ticksPerSecond != 0 ? data->animations[mActiveAnim]->ticksPerSecond : DEFAULT_TICKS_PER_SECOND;
		float timeInTicks = (mLastAnimationTime * ticksPerSecond) + mAnimationTickOffset;
		if (timeInTicks<0)
		{
			timeInTicks += (ceil(-timeInTicks / data->animations[mActiveAnim]->duration) + 1) * data->animations[mActiveAnim]->duration;
		}
		float animTime = fmod(timeInTicks, data->animations[mActiveAnim]->duration);
		//Store current pose into a temporary keyframe for each node
		auto &anim = *data->animations[mActiveAnim];
		//V2: Clear and rebuild transition keyframe to only include required nodes
		//Purge existing transition frame
		for (auto &a : mTransitionKeyFrame)
		{
			delete a.second;
		}
		mTransitionKeyFrame.clear();
		mTransitionKeyFrame.reserve(anim.nodeAnims.size());
		//For each nodeAnim in our source anim
		for (auto &a:anim.nodeAnims)
		{
			auto b = new Animation::NodeAnimation(1, 1, 1);
			b->scalingKeys[0].vec3 = a.second->calcInterpolatedScaling(animTime);
			b->rotationKeys[0].rotation = a.second->calcInterpolatedRotation(animTime);
			b->positionKeys[0].vec3 = a.second->calcInterpolatedTranslation(animTime);
			mTransitionKeyFrame[a.first] = b;
		}
		//V1: Alternate version where we always have a NodeAnim for every node, and set missing ones to bind pose
		//Not really sure which I prefer
		//for (auto &a : mTransitionKeyFrame)
		//{
		//	auto nodeIt = anim.nodeAnims.find(a.first);
		//	if (nodeIt != anim.nodeAnims.end())
		//	{//Animation, so calculate inverse skinning transformation
		//		a.second->scalingKeys[0].vec3 = nodeIt->second->calcInterpolatedScaling(animTime);
		//		a.second->rotationKeys[0].rotation = nodeIt->second->calcInterpolatedRotation(animTime);
		//		a.second->positionKeys[0].vec3 = nodeIt->second->calcInterpolatedTranslation(animTime);
		//	}
		//	else
		//	{//No animation, so load inverse bind pose transformation
		//		if (auto nd = data->nodeDirectory[a.first].lock())
		//		{
		//			glm::vec3 _t1; glm::vec4 _t2;
		//			assert(glm::decompose(data->transforms[nd->transformOffset],
		//				a.second->scalingKeys[0].vec3,
		//				a.second->rotationKeys[0].rotation,
		//				a.second->positionKeys[0].vec3,
		//				_t1, _t2));
		//			//Unsure why x,y,z need to be inverted, but otherwise rotation is flipped
		//			a.second->rotationKeys[0].rotation = -a.second->rotationKeys[0].rotation;
		//			a.second->rotationKeys[0].rotation.w = -a.second->rotationKeys[0].rotation.w;
		//		}
		//	}
		//}
		//Swap animation flag to next animation
		mActiveAnim = index;
		//Reset mAnimationTickOffset
		mAnimationTickOffset = 0;
	}
	else
	{
		fprintf(stderr, "Animation with id '%d' was not found.\n", index);
	}
}
void Model::setAnimation(const std::string &name, float transitionDuration)
{
	auto f = data->animationDirectory.find(name);
	if (f != data->animationDirectory.end())
	{
		setAnimation(f->second, transitionDuration);
	}
	else
	{
		fprintf(stderr, "Animation '%s' was not found.\n", name.c_str());
	}
}

void Model::updateBoneTransforms(float seconds)
{
	mLastAnimationTime = seconds;
	assert(mActiveAnim < data->animations.size());
	if(!mTransitioningKeyframes)
	{//Perform regular animation
		float ticksPerSecond = data->animations[mActiveAnim]->ticksPerSecond != 0 ? data->animations[mActiveAnim]->ticksPerSecond : DEFAULT_TICKS_PER_SECOND;
		float timeInTicks = (seconds * ticksPerSecond) + mAnimationTickOffset;
		if (timeInTicks<0)
		{
			timeInTicks += (ceil(-timeInTicks / data->animations[mActiveAnim]->duration) + 1) * data->animations[mActiveAnim]->duration;
		}
		float animTime = fmod(timeInTicks, data->animations[mActiveAnim]->duration);

		root->propagateAnimation(animTime, mActiveAnim);
	}
	else
	{//Interpolate between first frame from mTransitionKeyFrame and data->animations[mActiveAnim] over KEYFRAME_TRANSITION_DURATION*DEFAULT_TICKS_PER_SECOND) steps
		auto &start = mTransitionKeyFrame;
		auto &end = data->animations[mActiveAnim]->nodeAnims;
		float factor = mAnimationTickOffset / (mTransitionDuration*DEFAULT_TICKS_PER_SECOND);
		root->propagateKeyframeInterpolation(start, 0, end, 0, factor);

		mAnimationTickOffset++;
		//If transition has been completed
		if (mAnimationTickOffset>mTransitionDuration*DEFAULT_TICKS_PER_SECOND)
		{
			float timeInTicks = seconds * data->animations[mActiveAnim]->ticksPerSecond;
			if (timeInTicks<0)
			{
				timeInTicks += (ceil(-timeInTicks / timeInTicks) + 1) * timeInTicks;
			}
			float animTime = fmod(timeInTicks, data->animations[mActiveAnim]->duration);
			mAnimationTickOffset = -animTime;
			mTransitioningKeyframes = false;
		}
	}
	//Update bones on GPU
	boneBuffer->setData(data->computedTransforms, sizeof(glm::mat4)*data->bonesSize);
	{
		glm::mat4 rootTransform = glm::mat4(1);
		for (auto a = data->rootChain.begin(); a != data->rootChain.end(); ++a)
		{
			rootTransform *= data->_transforms[*a];
		}
		glm::vec3 newLocation = glm::vec3(rootTransform * glm::vec4(0, 0, 0, 1))*scaleFactor;
		this->mDistanceTravelled = distance(mPreviousLocation, newLocation);//When animation loops, this value goes wrong
		if (this->mDisableAnimationTravel)
		{
			this->mAnimationLocationOffset += mPreviousLocation;
			this->mAnimationLocationOffset -= newLocation;
		}
		mPreviousLocation = newLocation;
	}
	skeletonIsValid = false;
}
void Model::disableAnimationTravel(bool disable)
{
	this->mDisableAnimationTravel = disable;
	this->mAnimationLocationOffset = glm::vec3(0);
}

//Mesh management
bool Model::hasMesh(const std::string &meshName)
{
	if (this->data)
	{
		return this->data->meshDirectory.find(meshName) != this->data->meshDirectory.end();
	}
	return false;
}
void Model::setMeshVisible(const std::string &meshName, const bool &isVisible)
{
	if (this->data)
	{
		auto a = this->data->meshDirectory.find(meshName);
		if (a != this->data->meshDirectory.end())
			if (auto b = a->second.lock())
			{
				b->setVisible(isVisible);
			}
#ifdef _DEBUG
			else
			{
				fprintf(stderr, "Model::setVisible(): Model '%s' does not contain mesh '%s'.\n", modelPath.c_str(), meshName.c_str());
			}
#endif
	}
}
bool Model::getMeshVisible(const std::string &meshName)
{
	if (this->data)
	{
		auto a = this->data->meshDirectory.find(meshName);
		if (a != this->data->meshDirectory.end())
			if (auto b = a->second.lock())
			{
				return b->getVisible();
			}
#ifdef _DEBUG
			else
			{
				fprintf(stderr, "Model::getVisible(): Model '%s' does not contain mesh '%s'.\n", modelPath.c_str(), meshName.c_str());
			}
#endif
	}
	return false;
}
void Model::setEnvironmentMap(std::shared_ptr<const TextureCubeMap> cubeMap)
{
	environmentMap = cubeMap;
	for (unsigned int i = 0; i < data->materialsSize; ++i)
	{
		data->materials[i]->setEnvironmentMap(cubeMap);
	}
}
void Model::computeInverseRootTransform()
{
	std::unordered_map<glm::quat, unsigned int> quatCount;
	//Iterate all vertices
	for (unsigned int i = 0; i<data->verticesSize; ++i)
	{
		glm::vec3 &v = data->vertices[i];
		auto &bd = data->boneData[i];
		glm::mat4  boneTransform = glm::mat4(0);
		//Calculate vertices weighted bone transform
		for (unsigned int j = 0; j < bd.COUNT; ++j)
		{
			boneTransform += data->computedTransforms[bd.BoneIds()[j]] * bd.Weights()[j];
		}
		//Decompose computed bone transform to find orientation
		glm::vec3 scale, translation, skew;
		glm::quat orientation;
		glm::vec4 perspective;
		decompose(boneTransform, scale, orientation, translation, skew, perspective);
		//Insert orientation into a map, counting how many match
		if (quatCount.find(orientation) != quatCount.end())
			quatCount[orientation]++;
		else
		{
			quatCount[orientation] = 1;
		}
	}
	//Find the most common quaternion
	unsigned int max = 0;
	glm::quat best = glm::quat();
	for (auto &b : quatCount)
	{
		if (b.second>max)
		{
			max = b.second;
			best = b.first;
		}
	}
	if (best != glm::quat())
		data->inverseRootTransform = glm::toMat4(best);
}

glm::mat4 Model::getModelMat() const
{
	//Apply world transforms (in reverse order that we wish for them to be applied)
	glm::mat4 modelMat = glm::translate(glm::mat4(1), this->location + this->mAnimationLocationOffset);

	//Check we actually have a rotation (providing no axis == error)
	if ((this->rotation.x != 0 || this->rotation.y != 0 || this->rotation.z != 0) && this->rotation.w != 0)
		modelMat = glm::rotate(modelMat, glm::radians(this->rotation.w), glm::vec3(this->rotation));

	//Only bother scaling if we were asked to
	if (this->scaleFactor != 1.0f)
		modelMat = glm::scale(modelMat, glm::vec3(this->scaleFactor));
	return modelMat;
}