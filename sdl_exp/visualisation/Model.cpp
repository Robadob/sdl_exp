#include "Model.h"
#include <assimp/types.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.inl>

Model::Model(const char *modelPath)
	: modelPath(modelPath)
	, root(nullptr)
	, data(nullptr)
	, positions(GL_FLOAT, 3, sizeof(float))
	, normals(GL_FLOAT, 3, sizeof(float))
	, colors(GL_FLOAT, 4, sizeof(float))
	, texcoords(GL_FLOAT, 3, sizeof(float))
{
	loadModel();
}
Model::~Model()
{
	freeModel();
}
void Model::freeModel()
{
	//Clear hierarchy
	root.reset();
	//Clear data
	data.reset();
	//Release VBOs
}


void Model::reload()
{
	freeModel();
	loadModel();
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
std::shared_ptr<ModelNode> Model::buildHierarchy(const struct aiScene* scene, const struct aiNode* nd, VFCcount &vfc) const
{
	static int qi = -1;
	qi++;
	printf("Node(#%d, %s, M%d, C%d)\n", qi, nd->mName.C_Str(), nd->mNumMeshes, nd->mNumChildren);
	//Copy transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	//aiTransposeMatrix4(&m);
#ifdef _DEBUG
	//aiMatrix4x4 is type float
	assert(sizeof(glm::mat4) == sizeof(aiMatrix4x4));
#endif
	data->transforms[vfc.c] = *reinterpret_cast<glm::mat4*>(&m);
	//Create the ModelNode for this component (and increment data offset)
	std::shared_ptr<ModelNode> rtn = ModelNode::make_shared(this->data, vfc.c++);

	//Fill it with meshes
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* aiMesh = scene->mMeshes[nd->mMeshes[n]];

		if (aiMesh->mNumFaces == 0) continue;

		//Create a new mesh
		GLenum faceType;
		unsigned int mNumFaceIndices = aiMesh->mFaces[0].mNumIndices;
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
		std::shared_ptr<Mesh> mesh = Mesh::make_shared(this->data, vfc.f*sizeof(unsigned int), aiMesh->mNumFaces*mNumFaceIndices, aiMesh->mMaterialIndex, faceType);

#ifdef _DEBUG
		//aiVector3D is type float
		assert(sizeof(glm::vec3) == sizeof(aiVector3D));
		//aiColor4D is type float
		assert(sizeof(glm::vec4) == sizeof(aiColor4D));
#endif
		//Copy data for each face
		//Vertex
		memcpy(data->vertices + vfc.v, aiMesh->mVertices, aiMesh->mNumVertices*sizeof(glm::vec3));
		//for (int j = 0; j < aiMesh->mNumVertices; j++)
		//{
		//	printf("(%.4f,%.4f,%.4f)\n", data->vertices[vfc.v + j].x, vfc.f + data->vertices[vfc.v + j].y, vfc.f + data->vertices[vfc.v + j].z);
		//}
		//Normal
		if (data->normals)
			memcpy(data->normals + vfc.v, aiMesh->mNormals, aiMesh->mNumVertices*sizeof(glm::vec3));
		//Color (just take the first set)
		if (data->colors)
			memcpy(data->colors + vfc.v, aiMesh->mColors[0], aiMesh->mNumVertices*sizeof(glm::vec4));
		//Texture Coordinates (just take the first set)
		if (data->texcoords)
			memcpy(data->texcoords + vfc.v, aiMesh->mTextureCoords[0], aiMesh->mNumVertices*sizeof(glm::vec3));

		//Map face indexes
		for (unsigned int t = 0; t < aiMesh->mNumFaces; ++t) {
			const struct aiFace* face = &aiMesh->mFaces[t];
			for (unsigned int i = 0; i < face->mNumIndices; ++i) {
				int index = face->mIndices[i];
				//printf("%d:%d\n", vfc.f + (t*mNumFaceIndices) + i, vfc.v + index);
				//printf("(%.4f,%.4f,%.4f)", data->vertices[vfc.v + index].x, data->vertices[vfc.v + index].y, data->vertices[vfc.v + index].z);
				data->faces[vfc.f + (t*mNumFaceIndices) + i] = vfc.v + index;
			}
			//printf("\n");
		}
		//printf("---------------------------------------\n");

		//Bones
		//if (data->bones)
		//	for (unsigned int t = 0; t<aiMesh->mNumBones;++t)
		//	{
		//		data->bones[vfc.b + t] = Bone();//aiMesh->mBones[]
		//		//Todo convert bones, (and vertex mapping)
		//	}

		//Increment data offsets
		vfc.v += aiMesh->mNumVertices;
		vfc.b += aiMesh->mNumBones;
		vfc.f += aiMesh->mNumFaces*mNumFaceIndices;

		//Link mesh to hierarchy
		rtn->addMesh(mesh);
		//mesh->setParent(rtn);
	}
	//Recursivly fill children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		std::shared_ptr<ModelNode> child = buildHierarchy(scene, nd->mChildren[n], vfc);
		rtn->addChild(child);
		//child->setParent(rtn);
	}
	return rtn;
}
/**
 * Loads a model from file
 * @TODO Also add support for importing bones, textures and materials
 */
void Model::loadModel()
{
	//Import model with assimp
	const aiScene* scene = aiImportFile(modelPath, aiProcessPreset_TargetRealtime_MaxQuality);

	if (!scene)
		fprintf(stderr, "Err Model not found: %s\n", modelPath);

	//Calculate total number of vertices, faces, components and bones within hierarchy
	this->vfc = countVertices(scene, scene->mRootNode);

	//Allocate memory
	data = std::make_shared<ModelData>(
		vfc.v,
		scene->mMeshes[0]->HasNormals() ? vfc.v : 0,
		scene->mMeshes[0]->mColors[0] != nullptr ? vfc.v : 0,
		scene->mMeshes[0]->HasTextureCoords(0) ? vfc.v : 0,
		scene->mMeshes[0]->HasBones() ? vfc.b : 0,
		scene->mNumMaterials,
		vfc.f,
		vfc.c
		);

	//Store count in VADs
	positions.count = this->vfc.v;
	normals.count = normals.data ? this->vfc.v : 0;
	colors.count = colors.data?this->vfc.v:0;
	texcoords.count = texcoords.data ? this->vfc.v : 0;
	positions.data = data->vertices;
	normals.data = data->normals;
	colors.data = data->colors;
	texcoords.data = data->texcoords;

	//Copy materials
	//for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	//{
	//	//data.materials[i] = scene->mMaterials[i];
	//  //TODO
	//}

	//Convert assimp hierarchy to our custom hierarchy (ignoring bones for the time being)
	VFCcount rootCount = VFCcount(0);
	root = buildHierarchy(scene, scene->mRootNode, rootCount);


	//Free assimp model
	aiReleaseImport(scene);

	//Calc VBO size
	size_t vboSize = 0;
	vboSize += this->vfc.v*sizeof(glm::vec3);
	if (data->normals)
		vboSize += this->vfc.v*sizeof(glm::vec3);
	if (data->colors)
		vboSize += this->vfc.v*sizeof(glm::vec4);
	if (data->texcoords)
		vboSize += this->vfc.v*sizeof(glm::vec3);

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
		normals.offset = vboSize;
		vboSize += this->vfc.v*sizeof(glm::vec3);
	}
	if (data->colors)
	{
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, vboSize, this->vfc.v*sizeof(glm::vec4), data->colors));
		colors.offset = vboSize;
		vboSize += this->vfc.v*sizeof(glm::vec4);
	}
	if (data->texcoords)
	{
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, vboSize, this->vfc.v*sizeof(glm::vec3), data->texcoords));
		texcoords.offset = vboSize;
		vboSize += this->vfc.v*sizeof(glm::vec3);
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
	
	//Build FBO
	GL_CALL(glGenBuffers(1, &fbo));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->vfc.f*sizeof(unsigned int), data->faces, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Model::render(std::shared_ptr<Shaders> shaders)
{
	static int i = 0;
	//Configure shader
	if (shaders != nullptr)
	{

		if (i==0)
		{
			shaders->setPositionsAttributeDetail(positions);
			shaders->setNormalsAttributeDetail(normals);
			shaders->setColorsAttributeDetail(colors);
			shaders->setTexCoordsAttributeDetail(texcoords);
			i++;
		}
	}
	//Bind the faces to be rendered
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo));
	//Recursive render
	glm::mat4 modelMat;

	//Apply world transforms
	//Check we actually have a rotation (providing no axis == error)
	if ((this->rotation.x != 0 || this->rotation.y != 0 || this->rotation.z != 0) && this->rotation.w != 0)
		modelMat = glm::rotate(modelMat, glm::radians(this->rotation.w), glm::vec3(this->rotation));
	modelMat = glm::translate(modelMat, this->location);

	//GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	//glGetBufferSubData(GL_ARRAY_BUFFER,0, this->vfc.v*sizeof(glm::vec3), data->vertices);
	//GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	//GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo));
	//glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, this->vfc.f*sizeof(unsigned int), data->faces);
	//GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	//Trigger recursive render
	root->render(shaders, modelMat);
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	shaders->clearProgram();
}