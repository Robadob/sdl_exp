#define _CRT_SECURE_NO_WARNINGS
#include "Entity2.h"
#include <vector>
#include <map>
#include <unordered_map>
#include "shader/Material.h"
#include <glm/gtc/type_ptr.hpp>

Entity2::Entity2(const char *modelPath)
    : modelPath(modelPath)
    , vertices(GL_FLOAT, 3, sizeof(float))//Components may be increased at runtime
    , normals(GL_FLOAT, 3, sizeof(float))
    , colors(GL_FLOAT, 3, sizeof(float))
    , texcoords(GL_FLOAT, 2, sizeof(float))//Components may be increased at runtime
    , faces(GL_UNSIGNED_INT, 3, sizeof(unsigned int))//Components may be increased at runtime
    , shaders(std::make_shared<Shaders>(Stock::Shaders::FLAT))
    , rotation(0)
    , location(0)
{
    shaders->setColor(glm::vec3(1));
	loadModel();
	shaders->setModelMatPtr(&modelMat);
}

Entity2::Entity2(
    const float *vertices, unsigned int vComponents,
    const float *normals, unsigned int nComponents,
    const float *colors, unsigned int cComponents,
    const float *tCoords, unsigned int tcComponents,
    unsigned int count,
    const void *faces, size_t fSize, unsigned int fComponents, unsigned int fCount,
    std::shared_ptr<Texture2D> texture)
    : modelPath()
    , vertices(GL_FLOAT, vComponents, sizeof(float))//Components may be increased at runtime
    , normals(GL_FLOAT, nComponents, sizeof(float))
    , colors(GL_FLOAT, cComponents, sizeof(float))
    , texcoords(GL_FLOAT, tcComponents, sizeof(float))//Components may be increased at runtime
    , faces(fSize==16?GL_UNSIGNED_SHORT:GL_UNSIGNED_INT, fComponents, (unsigned int)fSize)//Components may be increased at runtime
{
    assert(vertices);
    assert(faces);
    //Copy vertex attribute data
    this->vertices.data = malloc(sizeof(float)*vComponents*count);
    memcpy(this->vertices.data, vertices, sizeof(float)*vComponents*count);
    this->vertices.count = count;
    if (normals)
    {
        this->normals.data = malloc(sizeof(float)*nComponents*count);
        memcpy(this->normals.data, normals, sizeof(float)*nComponents*count);
        this->normals.count = count;
    }
    if (colors)
    {
        this->colors.data = malloc(sizeof(float)*cComponents*count);
        memcpy(this->colors.data, colors, sizeof(float)*cComponents*count);
        this->colors.count = count;
    }
    if (tCoords)
    {
        this->texcoords.data = malloc(sizeof(float)*tcComponents*count);
        memcpy(this->texcoords.data, tCoords, sizeof(float)*tcComponents*count);
        this->texcoords.count = count;
    }
    this->faces.data = malloc(fSize*fCount*fComponents);
    memcpy(this->faces.data, faces, fSize*fCount*fComponents);
    this->faces.count = fCount;
    //Copy data to VBOs
    generateVertexBufferObjects();
    //Configure rendergroups
    renderGroup.push_back(RenderGroup(-1, 0, fComponents));
    renderGroup[0].faceIndexCount = fComponents*fCount;
    //Create Shader
    if (texture)
    {
        shaders = std::make_shared<Shaders>(Stock::Shaders::TEXTURE);
        texture->bindToShader(shaders);
    }
    else
    {
        shaders = std::make_shared<Shaders>(Stock::Shaders::FLAT);
    }
    //bind attribs
    if (shaders)
    {
        shaders->setPositionsAttributeDetail(this->vertices);
        shaders->setNormalsAttributeDetail(this->normals);
        shaders->setColorsAttributeDetail(this->colors);
        shaders->setTexCoordsAttributeDetail(this->texcoords);
        shaders->setRotationPtr(&this->rotation);
        shaders->setTranslationPtr(&this->location);
	}
	shaders->setModelMatPtr(&modelMat);
}
Entity2::~Entity2()
{
    if (vertices.vbo)
        GL_CALL(glDeleteBuffers(1, &vertices.vbo));
    if (faces.vbo)
        GL_CALL(glDeleteBuffers(1, &faces.vbo));
    if (vertices.data)
        free(vertices.data);
    if (normals.data)
        free(normals.data);
    if (colors.data)
        free(colors.data);
    if (texcoords.data)
        free(texcoords.data);
    if (faces.data)
        free(faces.data);
}
static char* parseRow(char* buf, char* bufEnd, char* row, int len)
{
	bool start = true;
	bool done = false;
	int n = 0;
	while (!done && buf < bufEnd)
	{
		char c = *buf;
        buf++;
		// multirow
		switch (c)
		{
		case '\\':
			break;
		case '\n':
			if (start) break;
			done = true;
			break;
		case '\r':
			break;
		case '\t':
		case ' ':
			if (start) break;
		default:
			start = false;
			row[n++] = c;
			if (n >= len - 1)
				done = true;
			break;
		}
	}
	row[n] = '\0';
	return buf;
}
static void parseRowFromFile(FILE** file, char* buf, const int bufLen, int &bufIndex, char* row, const int rowLen)
{
	if (!*file)
	{
		row[0] = '\0';
		return;
	}
		bool start = true;
		bool done = false;
		int n = 0;
		while (!done)
		{
			if (bufIndex>=bufLen)
			{//If we've reached end of buffer, refill it
                size_t readLen = fread(buf, 1, bufLen, *file);
				bufIndex = 0;
				if (readLen < bufLen)
					buf[readLen] = '\0';
			}
			char c = buf[bufIndex++];
			// multirow
			switch (c)
			{
			case '\\':
				break;
			case '\n':
				if (start) break;
				done = true;
				break;
			case '\r':
                break;
            case '\0':
                fclose(*file);
                *file = nullptr;
                done = true;
                break;
			case '\t':
			case ' ':
				if (start) break;
			default:
				start = false;
				row[n++] = c;
				if (n >= rowLen - 1)
					done = true;
				break;
			}
		}
		row[n] = '\0';
}
static bool parseVert(char* row, float* vdata, unsigned int*cOffset)
{
	////Reset optional vars
	//int vCount = 0;
	////While end of line is unreached
	//while (*row != '\0')
	//{
	//	// Skip whitespace chars
	//	while (*row != '\0' && (*row == ' ' || *row == '\t'))
	//		row++;
	//	if (*row == '\0')//End of line
	//		break;
	//	char* s = row;
	//	//Find delimiter
	//	while (*row != '\0' && *row != ' ' && *row != '\t')
	//	{
	//		row++;
	//	}
	//	vdata[vCount++] = atof(s);
	//}
	int vCount = sscanf(row, "%f %f %f %f %f %f %f", vdata, vdata + 1, vdata + 2, vdata + 3, vdata + 4, vdata + 5, vdata + 6);
	//Work out what type of vertex we had
	if (vCount == 3)
	{//3 verts
		*cOffset = 3;
		return false;
	}
	else if (vCount == 6)
	{//3 verts, 3 colours
		*cOffset = 3;
		return true;
	}
	else if (vCount == 7)
	{//4 verts, 3 colours
		*cOffset = 4;
		return true;
	}
	else if (vCount == 4)
	{//4 verts
		*cOffset = 4;
		return false;
	}
	else
	{
		assert(false);//Vert line invalid
		return false;
	}
}
static unsigned int parseFace(char * row, unsigned int (&faceData)[3][4], unsigned int (&faceDataCount)[3])
{
	unsigned int vertCount = 0;
	unsigned int facePart = 0;//0=vert, 1=tex coord, 2=normal
	faceDataCount[0] = 0;//Reset
	faceDataCount[1] = 0;
	faceDataCount[2] = 0;
	bool inSpace = true;
	for (; *row != '\0';)
	{//While we have char
		if (*row == '/')
		{//Move to next face part
			facePart++;
			inSpace = false;
		}
		else if (inSpace==false&&(*row == ' ' || *row == '\t'))
		{//Move to next vertex
			inSpace = true;
			vertCount++;
			facePart = 0;
		}
		else
		{
			inSpace = false;
			faceData[facePart][vertCount] = (unsigned int)strtol(row, &row, 10);
            faceDataCount[facePart]++;
            continue;
		}
        row++;
	}
	if (!inSpace)
		vertCount++;
	return vertCount;
}
#define INITIAL_ALLOC 1024
#define REALLOC_MULTIPLIER 2
static void addVertex(float *vdata, unsigned int& cap, unsigned int &components, float *&verts, unsigned int &vertCount)
{
	//Double memory if we haven't enough
	if (vertCount + 1 > cap)
	{
		cap = !cap ? INITIAL_ALLOC : cap * REALLOC_MULTIPLIER;
        float* nv = new float[cap * components];
        std::fill(nv, nv + (cap * components), 12.0f);
		if (vertCount)
            memcpy(nv, verts, vertCount * components * sizeof(float));
		delete[] verts;
		verts = nv;
	}
	//Copy vert into array
    memcpy(verts + (vertCount++ * components), vdata, components * sizeof(float));
}
static void addColor(float *cdata, unsigned int& cap, float *&colors, unsigned int &colorCount)
{
	//Double memory if we haven't enough
	if (colorCount + 1 > cap)
	{
		cap = !cap ? INITIAL_ALLOC : cap * REALLOC_MULTIPLIER;
		float* nc = new float[cap * 3];
		if (colorCount)
			memcpy(nc, colors, colorCount * 3 * sizeof(float));
		delete[] colors;
		colors = nc;
	}
	//Copy vert into array
	memcpy(colors + (colorCount++ * 3), cdata, 3 * sizeof(float));
}
static void addNormal(glm::vec3 *ndata, unsigned int& cap, float *&normals, unsigned int &normalCount)
{
	//Double memory if we haven't enough
	if (normalCount + 1 > cap)
	{
		cap = !cap ? INITIAL_ALLOC : cap * REALLOC_MULTIPLIER;
        float* nn = new float[cap * 3];
        std::fill(nn, nn + (cap * 3), 12.0f);
		if (normalCount)
			memcpy(nn, normals, normalCount * sizeof(glm::vec3));
		delete[] normals;
		normals = nn;
	}
	//Copy vert into array
	memcpy(normals + (normalCount++ * 3), ndata, sizeof(glm::vec3));
}
static void addTexCoord(glm::vec3 *ndata, unsigned int& cap, unsigned int& tcComps, float *&texCoords, unsigned int &tcCount)
{
	//Double memory if we haven't enough
	if (tcCount + 1 > cap)
	{
		cap = !cap ? INITIAL_ALLOC : cap * REALLOC_MULTIPLIER;
		float* ntc = new float[cap * tcComps];
        std::fill(ntc, ntc + (cap * tcComps), 12.0f);
		if (tcCount)
			memcpy(ntc, texCoords, tcCount * tcComps* sizeof(float));
		delete[] texCoords;
        texCoords = ntc;
	}
	//Copy vert into array
	memcpy(texCoords + (tcCount++ * tcComps), ndata, tcComps* sizeof(float));
}
void Entity2::addFace(const unsigned int(&faceData)[3][4], const unsigned int(&faceDataCount)[3], unsigned int &faceCap,
    const float *t_verts, const float*t_colors, const float *t_texCoords, const float *t_normals,
    const RenderGroup &renderGroup)
{
    //Extend memory if required
    if (faceDataCount[0] + faces.count > faceCap)
    {
        faceCap = !faceCap ? INITIAL_ALLOC : faceCap * REALLOC_MULTIPLIER;
        //Extend face index buffer
        {
            unsigned int* _faceIndices = new unsigned int[faceCap];
            if (faces.data)
                memcpy(_faceIndices, faces.data, faces.count * sizeof(unsigned int));
            delete[] (unsigned int*)faces.data;
            faces.data = _faceIndices;
            for (unsigned int i = faces.count; i < faceCap; ++i)
                ((unsigned int*)faces.data)[i] = i;
        }
        //Extend vertex buffer
        {
            float *_verts = new float[faceCap * vertices.components];
            vertices.count = faceCap;
            if (vertices.data)
            {
                memcpy(_verts, vertices.data, faces.count * vertices.components * sizeof(float));
                delete[] (float*)vertices.data;
            }
            vertices.data = _verts;
        }
        //Extend color buffer (always 3 components)
        if (renderGroup.hasColors || colors.data)
        {
            float *_colors = new float[faceCap * colors.components];
            colors.count = faceCap;
            if (colors.data)
            {
                memcpy(_colors, colors.data, faces.count * colors.components * sizeof(float));
                delete[](float*)colors.data;
            }
            colors.data = _colors;
        }
        //Extend normal buffer (always 3 components)
        if (renderGroup.hasNormals || normals.data)
        {
            float *_normals = new float[faceCap * normals.components];
            normals.count = faceCap;
            if (normals.data)
            {
                memcpy(_normals, normals.data, faces.count * normals.components * sizeof(float));
                delete[](float*)normals.data;
            }
            normals.data = _normals;
        }
        //Extend tc buffer
        if (renderGroup.hasTexCoords || texcoords.data)
        {
            float *_texCoords = new float[faceCap * texcoords.components];
            texcoords.count = faceCap;
            if (texcoords.data)
                memcpy(_texCoords, texcoords.data, faces.count * texcoords.components * sizeof(float));
            delete[](float*)texcoords.data;
            texcoords.data = _texCoords;
        }
    }
    if (!colors.data && renderGroup.hasColors)
    {
        colors.data = new float[faceCap * colors.components];
        colors.count = faceCap;
    }
    if (!normals.data && renderGroup.hasNormals)
    {
        normals.data = new float[faceCap * normals.components];
        normals.count = faceCap;
    }
    if (!texcoords.data && renderGroup.hasTexCoords)
    {
        texcoords.data = new float[faceCap * texcoords.components];
        texcoords.count = faceCap;
    }
    //Copy vertices
    {
        for (unsigned int i = 0; i < faceDataCount[0]; ++i)
            for (unsigned int j = 0; j < vertices.components; ++j)
                ((float*)vertices.data)[((faces.count + i)*vertices.components) + j] = t_verts[((faceData[0][i]-1) * vertices.components) + j];
    }
    //Copy colors if present
    if (renderGroup.hasColors)
    {
        for (unsigned int i = 0; i < faceDataCount[0]; ++i)
            for (unsigned int j = 0; j < 3; ++j)
                ((float*)colors.data)[((faces.count + i) * colors.components) + j] = t_colors[((faceData[0][i] - 1) * colors.components) + j];
    }
    //Copy normals if present (it is assumed if one RG has norms/tex coords, all do)
    if (renderGroup.hasNormals)//Redundant?
    {
        if (faceDataCount[2])//if current face/rendergroup has normals
        {
            for (unsigned int i = 0; i < faceDataCount[2]; ++i)
                for (unsigned int j = 0; j < 3; ++j)
                    ((float*)normals.data)[((faces.count + i) * normals.components) + j] = t_normals[((faceData[2][i] - 1) * normals.components) + j];
        }
    }
    //Copy Texcoords if present
    if (renderGroup.hasTexCoords)//Redundant?
    {
        if (faceDataCount[1])//if current face/rendergroup has texcoords
        {
            for (unsigned int i = 0; i < faceDataCount[1]; ++i)
                for (unsigned int j = 0; j < texcoords.components; ++j)
                    ((float*)texcoords.data)[((faces.count + i)*texcoords.components) + j] = t_texCoords[((faceData[1][i] - 1) * texcoords.components) + j];
        }
    }

    faces.count += faceDataCount[0];
}
	
void Entity2::loadModel()
{
    //char* buf = nullptr;
    if (modelPath.empty())
        return;
    FILE* fp = fopen(modelPath.c_str(), "rb");
    if (!fp)
    {
        fprintf(stderr, "Error: Model '%s' could not be read.", modelPath.c_str());
        return;
    }
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        fprintf(stderr, "Error: Model '%s' could not be read. Failed to seek file end.", modelPath.c_str());
        return;
    }
    long bufSize = ftell(fp);
    if (bufSize < 0)
    {
        fclose(fp);
        fprintf(stderr, "Error: Model '%s' could not be read. Failed to measure file length.", modelPath.c_str());
        return;
    }
    if (fseek(fp, 0, SEEK_SET) != 0)
    {
        fclose(fp);
        fprintf(stderr, "Error: Model '%s' could not be read. Failed to return seek to start of file.", modelPath.c_str());
        return;
    }
    //Allocate memory equal to file size
    //buf = new char[bufSize];
    //Read file into memory
    //size_t readLen = fread(buf, bufSize, 1, fp);
    //fclose(fp);
    //if (readLen != 1)
    //{
     //   delete[] buf;
     //   fprintf(stderr, "Error: Model '%s' could not be read. Failed to read entire model file to memory.", modelPath.c_str());
    //    return;
   // }

    //char* src = buf;					//Pointer to current position in buffer
    //char* srcEnd = buf + bufSize;		//Pointer to end of buffer
	char row[512];						//Buffer to copy single line of file into
	char buf[1024];						//Buffer to cache file reads into
	int bufIndex = INT_MAX;
    unsigned int face[3][4];						//Buffer to copy face data into
    unsigned int faceDataCount[3];				//Buffer to copy face data into
    float vdata[7];
    glm::vec3 nData;
    unsigned int _vComps = 0;
    unsigned int _tcComps = 0;
    bool hasColor;
    unsigned int nv;								            //Number of vertex in facedata
    unsigned int vcap = 0; unsigned int t_vertCount = 0;     //Current number of vertex we have space for
    unsigned int ncap = 0; unsigned int t_normalCount = 0;	//Current number of normal we have space for
    unsigned int ccap = 0; unsigned int t_colorCount = 0;	//Current number of color we have space for
    unsigned int tccap = 0; unsigned int t_tcCount = 0;		//Current number of tex coords indices we have space for
    unsigned int faceCap = 0;		                        //Current number of face indices we have space for
    float *t_verts = nullptr;                       //Temporary buffers for data storage
    float *t_colors = nullptr;
    float *t_texCoords = nullptr;
    float *t_normals = nullptr;
    std::vector<std::string> matFiles;
    std::map<std::string, unsigned int> materialIds;
    renderGroup.clear();
    renderGroup.push_back({ -1, 0 });		//Initial group with null mat
    vertices.count = 0;
    texcoords.count = 0;
    colors.count = 0;
    //while (src < srcEnd)
	row[0] = '1';
	while (row[0]!='\0')
    {
        // Parse one row
        //src = parseRow(src, srcEnd, row, sizeof(row) / sizeof(char));
		parseRowFromFile(&fp, buf, 1024, bufIndex, row, 512);
        // Skip comments
        if (row[0] == '#') continue;
        if (row[0] == 'v')
        {
            if (row[1] == 'w')
            {//vp u v (w[1.0f])
                fprintf(stderr, "Warning: Model '%s' contains 'vw' tags representing free-form geometry, these are unsupported.", modelPath.c_str());
            }
            else if (row[1] == 'n')
            {//vn i j k
                //Vertex normal
                _tcComps = sscanf(row + 2, "%f %f %f", &nData.x, &nData.y, &nData.z);
                assert(_tcComps == 3);//Norm must be 3 elements
                addNormal(&nData, ncap, t_normals, t_normalCount);
                if (nData.x < -10000)
                    printf("%s", row);
            }
            else if (row[1] == 't')
            {//vt u (v[1.0f]) (w[1.0f])
                //Texture coord
                _tcComps = sscanf(row + 2, "%f %f %f", &nData.x, &nData.y, &nData.z);
                if (nData.x < -500000)
                    printf("%s", row);
                if (texcoords.components != _tcComps)
                {
                    if (texcoords.components < _tcComps)
                    {//If component count has decreased, add default v&w=1.0f
                        for (unsigned int u = texcoords.components; u < _tcComps; ++u)
                            nData[u] = 1.0f;
                        texcoords.components = _tcComps;
                    }
                    else
                    {
                        if (_tcComps != 0)
                            assert(false);//tc element count has increased
                        _tcComps = texcoords.components;
                    }
                }
                addTexCoord(&nData, tccap, texcoords.components, t_texCoords, t_tcCount);
            }
            else
            {//v x y z (w[1.0f]) (r g b)
                // Vertex pos (+ colour)
                hasColor = parseVert(row + 1, vdata, &_vComps);
                if (vdata[0] < -500000)
                    printf("%s", row);
                if (hasColor)
                    addColor(vdata + vertices.components, ccap, t_colors, t_colorCount);
                if (vertices.components != _vComps)
                {
                    if (vertices.components == 3 && _vComps == 4)
                    {//If component count has decreased, add default W=1.0f
                        vdata[3] = 1.0f;
                        vertices.components = 4;
                    }
                    else
                    {
                        if (_vComps != 0)
                            assert(false);//Vertex element count has increased
                        _vComps = vertices.components;
                    }
                }
                addVertex(vdata, vcap, vertices.components, t_verts, t_vertCount);
            }
        }
        else if (row[0] == 'f')
        {//f v1(/(t1)/(n1)) v2(/(t2)/(n2)) v3(/(t3)/(n3)) (v4(/(t4)/(n4)))
            // Faces
            nv = parseFace(row + 1, face, faceDataCount);
            if (nv != renderGroup[renderGroup.size() - 1].faceComponentCount)
            {//If vertex per face count doesn't match current render group
                if (renderGroup[renderGroup.size() - 1].faceIndexStart == faces.count)
                {//If renderGroup is fresh, just update componentCount
                    renderGroup[renderGroup.size() - 1].faceComponentCount = nv;
                }
                else
                {//If renderGroup is old, make new group
                    renderGroup[renderGroup.size() - 1].faceIndexCount = faces.count - renderGroup[renderGroup.size() - 1].faceIndexStart;
                    renderGroup.push_back({ renderGroup[renderGroup.size() - 1].materialId, faces.count, nv });
                    renderGroup[renderGroup.size() - 1] = renderGroup[renderGroup.size() - 1];
                }
            }
            if (renderGroup[renderGroup.size() - 1].faceIndexStart == faces.count)
            {//If fresh RG, set group settings
                renderGroup[renderGroup.size() - 1].hasColors = colors.count > 0;
                renderGroup[renderGroup.size() - 1].hasNormals = faceDataCount[2] > 0;
                renderGroup[renderGroup.size() - 1].hasTexCoords = faceDataCount[1] > 0;
            }
            addFace(face, faceDataCount, faceCap, t_verts, t_colors, t_texCoords, t_normals, renderGroup[renderGroup.size() - 1]);
        }
        else if (row[0] == 'g')
        {
            //Smoothing groups not supported
        }
        else if (row[0] == 's')
        {
            //Smoothing groups not supported
        }
        else if (strncmp(row, "usemtl ", 7) == 0)
        {//usemtl <materialname>
            //End previous group
            renderGroup[renderGroup.size() - 1].faceIndexCount = faces.count - renderGroup[renderGroup.size() - 1].faceIndexStart;
            //Set current material
            char *token = strtok(row + 7, " ");
            if (token)
            {//Collect material file names, we will process them at the end
                std::string matName = std::string(token);
                std::map<std::string, unsigned int>::iterator it = materialIds.find(matName);
                if (it != materialIds.end())
                {//Repeated material
                    renderGroup.push_back({ (int)it->second, faces.count });
                }
                else
                {//New material
                    renderGroup.push_back({ (int)materialIds.size(), faces.count });
                    materialIds[matName] = (unsigned int)materialIds.size();
                }
            }
            else
            {//Null material				
                renderGroup.push_back({ -1, faces.count });
            }
        }
        else if (strncmp(row, "mtllib ", 7) == 0)
        {//mtllib <filename1> <filename2> ...
            char *token = strtok(row + 7, " ");
            while (token)
            {//Collect material file names, we will process them at the end
                matFiles.push_back(std::string(token));
                token = strtok(nullptr, " ");
            }
        }
        else if (row[0] != '\0')
        {
            fprintf(stderr, "Warning: Model '%s' contains unrecognised line:\n'%s'\n", modelPath.c_str(), row);
        }
    }
    //End final render group
    renderGroup[renderGroup.size() - 1].faceIndexCount = faces.count - renderGroup[renderGroup.size() - 1].faceIndexStart;
    //Cleanup buffers
    //delete[] buf;
    if (t_verts)
        delete[] t_verts;
    if (t_colors)
        delete[] t_colors;
    if (t_texCoords)
        delete[] t_texCoords;
    if (t_normals)
        delete[] t_normals;
    //Reduces counts to prevent excess data being handled
    vertices.count = faces.count;
    if (normals.data)
        normals.count = faces.count;
    if (colors.data)
        colors.count = faces.count;
    if (texcoords.data)
        texcoords.count = faces.count;
    //Merge render groups which share materials?
	{
        unsigned int matMax = 0;
        for (const auto &kv : materialIds)
        {
            matMax = glm::max(matMax, kv.second);
        }
        float *_vertices = nullptr;
        float *_normals = nullptr;
        float *_colors = nullptr;
        float *_texcoords = nullptr;
        std::vector<RenderGroup> renderGroup2;
         _vertices = (float*)malloc(vertices.count*vertices.components*vertices.componentSize);
        if (normals.data)
            _normals = (float*)malloc(normals.count*normals.components*normals.componentSize);
        if (colors.data)
            _colors = (float*)malloc(colors.count*colors.components*colors.componentSize);
        if (texcoords.data)
            _texcoords = (float*)malloc(texcoords.count*texcoords.components*texcoords.componentSize);
        unsigned int firstIndex = 0;
        for (int i = -1; i <= (int)matMax;++i)
        {
            RenderGroup rg(i, firstIndex, vertices.components);
            for (const auto &_rg : renderGroup)
            {//Copy all render groups into contiguous block
                if (_rg.materialId==i)
                {
                    memcpy(&_vertices[firstIndex*vertices.components], &((float*)vertices.data)[_rg.faceIndexStart*vertices.components], _rg.faceIndexCount*vertices.components*sizeof(float));
                    if (_normals)
                        memcpy(&_normals[firstIndex*normals.components], &((float*)normals.data)[_rg.faceIndexStart*normals.components], _rg.faceIndexCount*normals.components*sizeof(float));
                    if (_colors)
                        memcpy(&_colors[firstIndex*colors.components], &((float*)colors.data)[_rg.faceIndexStart*colors.components], _rg.faceIndexCount*colors.components*sizeof(float));
                    if (_texcoords)
                        memcpy(&_texcoords[firstIndex*texcoords.components], &((float*)texcoords.data)[_rg.faceIndexStart*texcoords.components], _rg.faceIndexCount*texcoords.components*sizeof(float));
                    rg.hasNormals = _rg.hasNormals;
                    rg.hasColors = _rg.hasColors;
                    rg.hasTexCoords = _rg.hasTexCoords;
                    rg.faceIndexCount += _rg.faceIndexCount;
                    firstIndex += _rg.faceIndexCount;
                }
            }
            if (rg.faceIndexCount>0)
            {
                assert(rg.faceIndexCount%3==0);
                renderGroup2.push_back(rg);
            }
        }
        renderGroup.clear();
        renderGroup = renderGroup2;
        free(vertices.data);
        vertices.data = _vertices;
        if (normals.data)
        {
            free(normals.data);
            normals.data = _normals;
        }
        if (colors.data)
        {
            free(colors.data);
            colors.data = _colors;
        }
        if (texcoords.data)
        {
            free(texcoords.data);
            texcoords.data = _texcoords;
        }
	}

    //Read all material files
    std::unordered_map<std::string, MaterialFile> t_materials;
    const size_t sepIndex = modelPath.find_last_of("\\/");
    for (const auto &matFile : matFiles)
    {
        std::string relativeMatFile = modelPath.substr(0,sepIndex+1)+matFile;
        fp = fopen(relativeMatFile.c_str(), "rb");
        if (!fp)
        {
            fprintf(stderr, "Error: MatLib '%s' could not be read.", relativeMatFile.c_str());
            continue;
        }
        if (fseek(fp, 0, SEEK_END) != 0)
        {
            fclose(fp);
            fprintf(stderr, "Error: MatLib '%s' could not be read. Failed to seek file end.", relativeMatFile.c_str());
            continue;
        }
        bufSize = ftell(fp);
        if (bufSize < 0)
        {
            fclose(fp);
            fprintf(stderr, "Error: MatLib '%s' could not be read. Failed to measure file length.", relativeMatFile.c_str());
            continue;
        }
        if (fseek(fp, 0, SEEK_SET) != 0)
        {
            fclose(fp);
            fprintf(stderr, "Error: MatLib '%s' could not be read. Failed to return seek to start of file.", relativeMatFile.c_str());
            continue;
        }
        //buf = new char[bufSize];
        //Read file into memory
        //readLen = fread(buf, bufSize, 1, fp);
        //fclose(fp);
        //src = buf;					//Pointer to current position in buffer
        //srcEnd = buf + bufSize;		//Pointer to end of buffer
        MaterialFile *currentMat = nullptr;
        bufIndex = INT_MAX;
        //while (src < srcEnd)
        row[0] = '1';
        while (row[0] != '\0')
        {
            // Parse one row
            //row[0] = '\0';
            //src = parseRow(src, srcEnd, row, sizeof(row) / sizeof(char));
            parseRowFromFile(&fp, buf, 1024, bufIndex, row, 512);
            if (strncmp(row, "newmtl ", 7) == 0)
            {//Start new material
                char *token = strtok(row + 7, " ");
                if (token)
                {//Collect material file names, we will process them at the end
                    std::string matName = std::string(token);
					t_materials[matName] = MaterialFile();
                    currentMat = &t_materials[matName];
                }                
            }
            else if (currentMat)
            {
                if (row[0] == 'K' || row[0]=='k')
                {
                    if (row[1] == 'a' || row[1] == 'A')
                    {//Ambient r g b
                        sscanf(row + 2, "%f %f %f", &currentMat->ambient.x, &currentMat->ambient.y, &currentMat->ambient.z);
                        currentMat->ambient = clamp(currentMat->ambient, glm::vec3(0), glm::vec3(1));
                    }
                    else if (row[1] == 'd' || row[1] == 'D')
                    {//Diffuse r g b
                        sscanf(row + 2, "%f %f %f", &currentMat->diffuse.x, &currentMat->diffuse.y, &currentMat->diffuse.z);
                        currentMat->diffuse = clamp(currentMat->diffuse, glm::vec3(0), glm::vec3(1));
                    }
                    else if (row[1] == 's' || row[1] == 'S')
                    {//Specular r g b
                        sscanf(row + 2, "%f %f %f", &currentMat->specular.x, &currentMat->specular.y, &currentMat->specular.z);
                        currentMat->specular = clamp(currentMat->specular, glm::vec3(0), glm::vec3(1));
                    }
                }
                else if (row[0] == 'N' || row[0] == 'n')
                {
                    if (row[1] == 's' || row[1] == 'S')
                    {//Specular exponent f
                        sscanf(row + 2, "%f", &currentMat->shininess);
                        currentMat->shininess = glm::clamp(currentMat->shininess, 0.0f, 1000.0f);
                    }
                }
                else if (row[0] == 'd' || row[0] == 'D')
                {//Opacity f
                    sscanf(row + 1, "%f", &currentMat->opacity);
                    currentMat->opacity = glm::clamp(currentMat->opacity, 0.0f, 1.0f);
                }
                else if (row[0] == 'T' || row[0] == 't')
                {
                    if (row[1] == 'r' || row[1] == 'R')
                    {//Inverted opacity 1-f
                        sscanf(row + 2, "%f", &currentMat->opacity);
                        currentMat->opacity = 1-glm::clamp(currentMat->opacity, 0.0f, 1.0f);

                    }
                }
                else if (strncmp(row, "illum ", 6) == 0)
                {//Illumination model
					sscanf(row + 2, "%d", &currentMat->illuminationModel);
                    //0. Color on and Ambient off
                    //1. Color on and Ambient on
                    //2. Highlight on
                    //3. Reflection on and Ray trace on
                    //4. Transparency: Glass on, Reflection : Ray trace on
                    //5. Reflection : Fresnel on and Ray trace on
                    //6. Transparency : Refraction on, Reflection : Fresnel off and Ray trace on
                    //7. Transparency : Refraction on, Reflection : Fresnel on and Ray trace on
                    //8. Reflection on and Ray trace off
                    //9. Transparency : Glass on, Reflection : Ray trace off
                    //10. Casts shadows onto invisible surfaces
                    static bool illum = false;
                    if (!illum)
                    {
                        illum = true;
                        fprintf(stderr, "Illuimination models currently unused by Entity2.cpp\n");
                    }
                }
				else if (strncmp(row, "map_", 4) == 0)
				{
					if (row[7] == '-')//Rough estimate
						fprintf(stderr, "material tex map parameters aren't supported");
					//-blendu on | off
					//- blendv on | off
					//- cc on | off
					//- clamp on | off
					//- mm base gain
					//- o u v w
					//- s u v w
					//- t u v w
					//- texres value
					if (row[4] == 'K' || row[4] == 'k')
					{
						char *token = strtok(row + 7, " ");
						if (token)
						{//Collect texture file names
							std::string texFile = std::string(token);
							if (row[5] == 'a' || row[5] == 'A')
							{//ambient texture map
								currentMat->tex_ambient = texFile;
							}
							else if (row[5] == 'd' || row[5] == 'D')
							{//Diffuse diffuse texture map (most of the time, it will be the same as ambient)
								currentMat->tex_diffuse = texFile;
							}
							else if (row[5] == 's' || row[5] == 'S')
							{//specular color texture map
								currentMat->tex_specular = texFile;
							}
						}
					}
					else if (row[4] == 'N' || row[4] == 'n')
					{
						char *token = strtok(row + 7, " ");
						if (token)
						{//Collect texture file names
							std::string texFile = std::string(token);
							if (row[5] == 's' || row[5] == 'S')
							{//specular highlight component
								currentMat->tex_specular = texFile;
							}
						}
					}
					else if (row[4] == 'd' || row[4] == 'D')
					{//alpha texture map
						char *token = strtok(row + 6, " ");
						if (token)
						{//Collect texture file names
							currentMat->tex_alpha = std::string(token);
						}

					}
					else if (strncmp(row + 4, "bump ", 5) == 0)
					{//bump map (which by default uses luminance channel of the image)
						char *token = strtok(row + 9, " ");
						if (token)
						{//Collect texture file names
							currentMat->tex_bump = std::string(token);
						}
					}
				}
				else if (strncmp(row, "bump ", 5) == 0)
				{//bump map (which by default uses luminance channel of the image)
					char *token = strtok(row + 5, " ");
					if (token)
					{//Collect texture file names
						currentMat->tex_bump = std::string(token);
					}					
				}
				else if (strncmp(row, "disp ", 5) == 0)
				{//displacement map
					char *token = strtok(row + 5, " ");
					if (token)
					{//Collect texture file names
						currentMat->tex_displacement = std::string(token);
					}
				}
				else if (strncmp(row, "decal ", 6) == 0)
				{//stencil decal texture (defaults to 'matte' channel of the image)
					char *token = strtok(row + 6, " ");
					if (token)
					{//Collect texture file names
						currentMat->tex_decal = std::string(token);
					}
				}
				else if (strncmp(row, "refl ", 5) == 0)
				{//reflection map
					char *token = strtok(row + 5, " ");
					if (token)
					{//Collect texture file names
						currentMat->tex_reflection = std::string(token);
					}
				}
            }
        }
        //delete[] buf;
    }
	//Copy t_materials into our material vec materials
    unsigned int missingMats = 0;
    {
        unsigned int matMax = 0;
        for (const auto &kv : materialIds)
        {
            matMax = glm::max(matMax, kv.second);
        }
        unsigned int i = 0;
        materials.clear();
        while (materials.size()<=matMax)
        {
            for (const auto &kv : materialIds)
            {
                if (kv.second == i)
                {
                    if (t_materials.find(kv.first) != t_materials.end())
                    {
                        materials.push_back(t_materials.at(kv.first));
                        goto continue_outer;
                    }
                    fprintf(stderr, "Material '%s' was not loaded.\n", kv.first.c_str());
                }
            }
            materials.push_back(MaterialFile());//Appears this material is missing
            missingMats++;
        continue_outer:;
            i++;
        }
	}
    //Correct face count (rather than it being index count)
    faces.count /= faces.components;
    //Copy data to VBOs
    generateVertexBufferObjects();
    //bind attribs
    if (shaders)
    {
        shaders->setPositionsAttributeDetail(vertices);
        shaders->setNormalsAttributeDetail(normals);
        shaders->setColorsAttributeDetail(colors);
        shaders->setTexCoordsAttributeDetail(texcoords);
        shaders->setRotationPtr(&this->rotation);
        shaders->setTranslationPtr(&this->location);
    }
    printf("Model : %s was loaded.\n%dv, %dvn, %dvc, %dvt, %df, %d/%dmat, %drg\n", modelPath.c_str(), vertices.count, normals.count, colors.count, texcoords.count, faces.count/faces.components, materials.size() - missingMats, materials.size(), renderGroup.size());
}
void Entity2::reload()
{
	//Reload Model from file
	loadModel();
	//Reload textures from file

	//Reload shaders from file
    shaders->reload();
}

void Entity2::setViewMatPtr(glm::mat4 const* viewMat)
{
	//Pass to all shaders
    shaders->setViewMatPtr(viewMat);
}

void Entity2::setProjectionMatPtr(glm::mat4 const* projectionMat)
{
	//Pass to all shaders
    shaders->setProjectionMatPtr(projectionMat);
}

/*
Creates the necessary vertex buffer objects, and fills them with the relevant instance var data.
*/
void Entity2::generateVertexBufferObjects()
{
    if (vertices.vbo)
        GL_CALL(glDeleteBuffers(1, &vertices.vbo));
    if (faces.vbo)
        GL_CALL(glDeleteBuffers(1, &faces.vbo));
    unsigned int bufferSize = 0;
    bufferSize += vertices.count * vertices.components * vertices.componentSize;
    bufferSize += normals.count *   normals.components *   normals.componentSize;
    bufferSize += colors.count *    colors.components *    colors.componentSize;
    bufferSize += texcoords.count * texcoords.components * texcoords.componentSize;
    GL_CALL(glGenBuffers(1, &vertices.vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertices.vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW));
    unsigned int offset = vertices.count*vertices.components*vertices.componentSize;
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, offset, vertices.data));
    if (normals.count)
    {
        normals.vbo = vertices.vbo;
        //redundant
        normals.offset = offset;
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, offset, normals.count*normals.components*normals.componentSize, normals.data));
        offset += normals.count*normals.components*normals.componentSize;
    }
    if (colors.count)
    {
        colors.vbo = vertices.vbo;
        //redundant
        colors.offset = offset;
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, offset, colors.count*colors.components*colors.componentSize, colors.data));
        offset += colors.count*colors.components*colors.componentSize;
    }
    if (texcoords.count)
    {
        texcoords.vbo = vertices.vbo;
        //redundant
        texcoords.offset = offset;
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, offset, texcoords.count*texcoords.components*texcoords.componentSize, texcoords.data));
        //offset += texcoords.count*texcoords.components*texcoords.componentSize;
    }
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Faces
    GL_CALL(glGenBuffers(1, &faces.vbo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces.vbo));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.count*faces.components*faces.componentSize, faces.data, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
void Entity2::render()
{
    static int colorUniformLoc = -1;
    if (colorUniformLoc == -1)
        colorUniformLoc=ShaderCore::findUniform("_color", shaders->getProgram()).first;
   // GL_CALL(glDisable(GL_CULL_FACE));
    //if (shaderIndex<shaders.size())
    //    shaders[shaderIndex]->useProgram();
    if (shaders)
        shaders->useProgram();
    //Bind the faces to be rendered
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces.vbo));
    //printf("%d\n", renderGroup[renderGroup.size() - 1].faceIndexStart+renderGroup[renderGroup.size() - 1].faceIndexCount);
    //GL_CALL(glDrawElements(GL_TRIANGLES, renderGroup[renderGroup.size() - 1].faceIndexStart + renderGroup[renderGroup.size() - 1].faceIndexCount, GL_UNSIGNED_INT, 0));
    //for (const auto &rg:renderGroup)
    for (unsigned int i = 0; i < renderGroup.size(); ++i)
    {
        const auto &rg = renderGroup[i];
        if (rg.materialId!=-1)
        {
            glm::vec4 color = glm::vec4(materials[rg.materialId].diffuse, materials[rg.materialId].opacity);
            GL_CALL(glUniform4fv(colorUniformLoc, 1, glm::value_ptr(color)));
        }
        else if (colorUniformLoc!=-1)
        {
            glm::vec4 color = glm::vec4(1);
            GL_CALL(glUniform4fv(colorUniformLoc, 1, glm::value_ptr(color)));
        }
        if (rg.faceComponentCount==3)
        {
            GL_CALL(glDrawElements(GL_TRIANGLES, rg.faceIndexCount, faces.componentType, (void*)(rg.faceIndexStart*faces.componentSize)));
        }
        else if (rg.faceComponentCount == 4)
        {
            GL_CALL(glDrawElements(GL_QUADS, rg.faceIndexCount, faces.componentType, (void*)(rg.faceIndexStart*faces.componentSize)));
        }
        else
            assert(false);
    }
   // GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    //Fixed fn
    GL_CALL(glEnable(GL_CULL_FACE));
    if (shaders)
        shaders->clearProgram();    
}