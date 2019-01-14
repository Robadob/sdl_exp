#include "PointLightModel.h"
#include <assimp/types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../../util/StringUtils.h"
#include <glm/gtc/matrix_transform.hpp>

const char PointLightModel::NFF_SPHERE[] = "s 1.0 1.0 1.0 0.5";
PointLightModel::PointLightModel()
	: vertices(nullptr)
	, normals(nullptr)
	, faces(nullptr)
	, vCount(0)
	, fCount(0)
	, vbo(0)
	, ibo(0)
	, shader(std::make_unique<Shaders>(Stock::Shaders::FULLBRIGHT))
	, rootTransform(1)
{
	{//Load model data
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFileFromMemory((void*)NFF_SPHERE, sizeof(NFF_SPHERE), aiProcessPreset_TargetRealtime_MaxQuality, "nff");
		if (!scene)
			throw std::runtime_error("Assimp failed to load NFF unit sphere.\n");
		if (scene->mNumMeshes != 1)
			throw std::runtime_error(su::format("Expected 1 mesh from NFF load, got %d.\n", scene->mNumMeshes));
		if (!scene->mMeshes[0]->HasNormals())
			throw std::runtime_error("Expected NFF mesh to have normals.\n");
		if (scene->mMeshes[0]->mFaces[0].mNumIndices != 3)
			throw std::runtime_error("Expected NFF mesh faces to be triangles.\n");
		vCount = scene->mMeshes[0]->mNumVertices;
		fCount = scene->mMeshes[0]->mNumFaces;
		vertices = (glm::vec3*)malloc(vCount*sizeof(glm::vec3)*2);
		normals = vertices + vCount;
		faces = (unsigned int*)malloc(fCount*scene->mMeshes[0]->mFaces[0].mNumIndices*sizeof(unsigned int));
		//Copy data from assimp data-structure.
		memcpy(vertices, scene->mMeshes[0]->mVertices, vCount*sizeof(glm::vec3));
		memcpy(normals, scene->mMeshes[0]->mNormals, vCount*sizeof(glm::vec3));
		for (unsigned int t = 0; t < fCount; ++t) {
			const struct aiFace* face = &scene->mMeshes[0]->mFaces[t];
			assert(face->mNumIndices == 3);
			for (unsigned int i = 0; i < face->mNumIndices; ++i) {
				faces[(t*face->mNumIndices) + i] = face->mIndices[i];
			}
		}
		rootTransform = glm::transpose(*(glm::mat4*)&scene->mRootNode->mTransformation);
		rootTransform *= glm::transpose(*(glm::mat4*)&scene->mRootNode->mChildren[0]->mTransformation);
	}
	{//Construct VBOs
		GL_CALL(glGenBuffers(1, &vbo));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, vCount*sizeof(glm::vec3) * 2, vertices, GL_STATIC_DRAW));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_CALL(glGenBuffers(1, &ibo));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
		GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, fCount*3*sizeof(unsigned int), faces, GL_STATIC_DRAW));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));		
	}
	{//Setup shader
		Shaders::VertexAttributeDetail vDetail = Shaders::VertexAttributeDetail(GL_FLOAT,3, sizeof(float));
		{
			vDetail.count = vCount;
			vDetail.data = vertices;
			vDetail.vbo = vbo;
			vDetail.offset = 0;
			vDetail.stride = 0;
		}
		shader->setPositionsAttributeDetail(vDetail, false);
		Shaders::VertexAttributeDetail nDetail = Shaders::VertexAttributeDetail(GL_FLOAT, 3, sizeof(float));
		{
			nDetail.count = vCount;
			nDetail.data = normals;
			nDetail.vbo = vbo;
			nDetail.offset = vCount*sizeof(glm::vec3);
			nDetail.stride = 0;
		}
		shader->setNormalsAttributeDetail(nDetail, false);
		shader->setFaceVBO(ibo);
	}
}
PointLightModel::~PointLightModel()
{
	GL_CALL(glDeleteBuffers(1, &vbo));
	GL_CALL(glDeleteBuffers(1, &ibo));
	free(vertices);
	free(faces);
}
void PointLightModel::setViewMatPtr(const glm::mat4 *viewMat)
{
	shader->setViewMatPtr(viewMat);
}
void PointLightModel::setProjectionMatPtr(const glm::mat4 *projectionMat)
{
	shader->setProjectionMatPtr(projectionMat);
}
void PointLightModel::setLightsBuffer(const GLuint &bufferBindingPoint)
{
	//Do nothing, lights will be rendered full bright
}
void PointLightModel::render(const glm::vec3 &location, const glm::vec3 &color) const
{
	glm::mat4 modelMat = glm::scale(glm::translate(glm::mat4(1), location), glm::vec3(scale));
	modelMat *= rootTransform;
	shader->setColor(glm::vec4(color, 0.8));
	shader->useProgram(true);
	shader->overrideModelMat(&modelMat);

	GL_CALL(glEnable(GL_BLEND));
	GL_CALL(glDrawElements(GL_TRIANGLES, fCount * 3, GL_UNSIGNED_INT, 0));

	shader->clearProgram();
}
void PointLightModel::reload()
{
	shader->reload();
};