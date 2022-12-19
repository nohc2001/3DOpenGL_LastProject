#include "Header.h"

/*
* 추가하거나 고칠것  :
* 2. 노멀맵
* 3. 쉐도우 매핑
* 7. 소스 나누기
* 8. 게임 구조 짜기 (게임 오브젝트, 플레이어, 씬?)
*/
using namespace std;

class Ptr {

};

constexpr int maxW = 1234;
constexpr int maxH = 864;

class Color4f {
public:
	float r;
	float g;
	float b;
	float a;

	Color4f() { r = 0; g = 0; b = 0; a = 0; }
	Color4f(const Color4f& ref) { r = ref.r; g = ref.g; b = ref.b; a = ref.a; }
	Color4f(float R, float G, float B, float A = 1.0f) { r = R; g = G; b = B; a = A; }
	virtual ~Color4f() {}
};

class TextSortStruct {
public:
	shp::vec2f positioning;
	bool OutHide;
	bool drawNextLines;

	bool LineHeightUnity;
	float LineHeightrate = 1.0f;

	TextSortStruct() {
		positioning = shp::vec2f(0, 0); OutHide = true; drawNextLines = false; LineHeightrate = 1.0f; LineHeightUnity = true;
	}
	TextSortStruct(shp::vec2f p, bool o, bool nl, float lhr = 1.0f) {
		positioning = p; OutHide = o, drawNextLines = nl; LineHeightrate = lhr; LineHeightUnity = true;
	}
	TextSortStruct(const TextSortStruct& ref) {
		positioning = ref.positioning; OutHide = ref.OutHide; drawNextLines = ref.drawNextLines; LineHeightrate = ref.LineHeightrate;
		LineHeightUnity = ref.LineHeightUnity;
	}
	virtual ~TextSortStruct() {}
};

glm::vec2 WinPosToGLPos(int x, int y) {
	glm::vec2 pos;

	float wd2 = maxW / 2;
	float hd2 = maxH / 2;

	pos.x = (x - wd2) / wd2;
	pos.y = (hd2 - y) / hd2;

	return pos;
}

typedef struct vec3uint {
	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
};

typedef struct vertexData {
	float x;
	float y;
	float z;
	int t;
	int n;
};

FM_Model0* ShapeFreeMem; // 모양을 저장하는 메모리
FM_Model0* InstanceFreeMem; // 간단히 쓰다 버릴 목적으로 쓰는 메모리
// 원본 모델
typedef struct GLShapeOrigin {
	int vertexCount = 0;
	glm::vec3* posArr = nullptr;
	glm::vec3* colorArr = nullptr;
	glm::vec2* texUV_CoordArr = nullptr;
	glm::vec3* normalArr = nullptr;

	int texCount = 0;
	glm::vec3* originTexArr = nullptr;

	int normalCount = 0;
	glm::vec3* originNormalArr = nullptr;

	int indexCount = 0;
	vec3uint* IndexArr = nullptr;

	int textureID = -1;

	GLuint EBO;
	GLuint VAO;
	GLuint VBO[4] = {}; // pos, color, tex, normal

	bool isElement = true;

	glm::vec3 BorderSize;
};

//고정 오브젝트들
constexpr unsigned int MAX_SHAPE = 300;
GLShapeOrigin shapeArr[MAX_SHAPE] = {};
int shapeup = 0;

// 애니메이션 조각들
constexpr unsigned int MAX_ANIMFRAG_SHAPE = 300;
GLShapeOrigin AnimFragShapeArr[MAX_ANIMFRAG_SHAPE] = {};
int AFShapeup = 0;

constexpr unsigned int MAX_TEXTURE = 100;
unsigned int textureArr[MAX_TEXTURE] = {};
int texup = 0;

size_t AddTexture(const char* filename) {
	if (texup + 1 < MAX_TEXTURE) {
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// 텍스처 wrapping/filtering 옵션 설정(현재 바인딩된 텍스처 객체에 대해)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// 텍스처 로드 및 생성
		int width, height, nrChannels;
		unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			//gluBuild2DMipmaps(GL_TEXTURE_2D, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);

		textureArr[texup] = texture;
		texup += 1;
		return texture;
	}
	return 0;
}

void AddShape(GLShapeOrigin shape, size_t texid = -1) {
	if (shapeup + 1 < MAX_SHAPE) {
		shape.textureID = texid;
		shapeArr[shapeup] = shape;
		shapeup += 1;
	}
}

void AddAnimShape(GLShapeOrigin shape, size_t texid = -1) {
	if (AFShapeup + 1 < MAX_SHAPE) {
		shape.textureID = texid;
		AnimFragShapeArr[AFShapeup] = shape;
		AFShapeup += 1;
	}
}

constexpr int MAX_VERTEX = 20000;
constexpr int MAX_INDEX = 30000;
constexpr int MAX_UV = 20000;
vertexData InstancePosArray[MAX_VERTEX] = {};
vec3uint InstanceIndexArray[MAX_INDEX] = {};
glm::vec3 InstanceUVArray[MAX_UV] = {};
glm::vec3 InstanceNormalArray[MAX_UV] = {};
void ReadObjModel(GLShapeOrigin* reader, const char* filename) {
	ifstream in;
	in.open(filename);

	int insposup = 0;
	int insindexup = 0;
	int insUVup = 0;
	int insrealposup = 0;
	int insNormalUp = 0;

	while (in.eof() == false) {
		char rstr[128] = {};
		in >> rstr;
		if (strcmp(rstr, "v") == 0) {
			//좌표
			glm::vec3 pos;
			in >> pos.x;
			in >> pos.y;
			in >> pos.z;
			if (insposup + 1 < MAX_VERTEX) {
				InstancePosArray[insposup] = { pos.x, pos.y, pos.z, -1, -1 };
				insposup += 1;
			}
		}
		else if (strcmp(rstr, "f") == 0) {
			//인덱스
			vec3uint index;
			char istr[128] = {};
			int istrup = 0;
			char c = in.get();
			int num[128] = {};
			int numup = 0;
			while (c != '\n') {
				if (c == '/') {
					if (numup + 1 < 10) {
						num[numup] = atoi(istr);
						istrup = 0;
						numup += 1;
					}
					//in >> rstr;
				}
				else if (c == ' ') {
					if (istrup != 0) {
						num[numup] = atoi(istr);
						istrup = 0;
						numup += 1;
					}
					c = in.get();
					continue;
				}
				else {
					if (istrup + 1 < 128) {
						istr[istrup] = c;
						istr[istrup + 1] = '\0';
						istrup += 1;
					}
				}
				c = in.get();
			}

			index.v1 = num[0] - 1;
			if (InstancePosArray[index.v1].t == -1 || InstancePosArray[index.v1].n == -1) {
				InstancePosArray[index.v1].t = num[1] - 1;
				InstancePosArray[index.v1].n = num[2] - 1;
			}
			else if (InstancePosArray[index.v1].t != num[1] - 1 || InstancePosArray[index.v1].n != num[2] - 1) {
				vertexData pd = InstancePosArray[index.v1];
				glm::vec3 pos = glm::vec3(pd.x, pd.y, pd.z);
				bool isExist = false;
				for (int i = 0; i < insposup; ++i) {
					vertexData vd = InstancePosArray[index.v1];
					if (((pos.x == vd.x && pos.y == vd.y) && (pos.z == vd.z && num[1] - 1 == vd.t)) && num[2] - 1 == vd.n) {
						index.v1 = i;
						isExist = true;
						break;
					}
				}
				if (isExist == false) {
					if (insposup + 1 < MAX_VERTEX) {
						InstancePosArray[insposup] = { InstancePosArray[index.v1].x, InstancePosArray[index.v1].y, InstancePosArray[index.v1].z, num[1] - 1, num[2] - 1 };
						index.v1 = insposup;
						insposup += 1;
					}
				}
			}

			index.v2 = num[3] - 1;
			if (InstancePosArray[index.v2].t == -1 || InstancePosArray[index.v2].n == -1) {
				InstancePosArray[index.v2].t = num[4] - 1;
			}
			else if (InstancePosArray[index.v2].t != num[4] - 1) {
				vertexData pd = InstancePosArray[index.v2];
				glm::vec3 pos = glm::vec3(pd.x, pd.y, pd.z);
				bool isExist = false;
				for (int i = 0; i < insposup; ++i) {
					vertexData vd = InstancePosArray[index.v2];
					if (((pos.x == vd.x && pos.y == vd.y) && (pos.z == vd.z && num[4] - 1 == vd.t)) && num[5] - 1 == vd.n) {
						index.v2 = i;
						isExist = true;
						break;
					}
				}
				if (isExist == false) {
					if (insposup + 1 < MAX_VERTEX) {
						InstancePosArray[insposup] = { InstancePosArray[index.v2].x, InstancePosArray[index.v2].y, InstancePosArray[index.v2].z, num[4] - 1, num[5] - 1 };
						index.v2 = insposup;
						insposup += 1;
					}
				}
			}

			index.v3 = num[6] - 1;
			if (InstancePosArray[index.v3].t == -1) {
				InstancePosArray[index.v3].t = num[7] - 1;
			}
			else if (InstancePosArray[index.v3].t != num[7] - 1) {
				vertexData pd = InstancePosArray[index.v3];
				glm::vec3 pos = glm::vec3(pd.x, pd.y, pd.z);
				bool isExist = false;
				for (int i = 0; i < insposup; ++i) {
					vertexData vd = InstancePosArray[index.v3];
					if (((pos.x == vd.x && pos.y == vd.y) && (pos.z == vd.z && num[7] - 1 == vd.t)) && num[8] - 1 == vd.n) {
						index.v3 = i;
						isExist = true;
						break;
					}
				}
				if (isExist == false) {
					if (insposup + 1 < MAX_VERTEX) {
						InstancePosArray[insposup] = { InstancePosArray[index.v3].x, InstancePosArray[index.v3].y, InstancePosArray[index.v3].z, num[7] - 1, num[8] - 1 };
						index.v3 = insposup;
						insposup += 1;
					}
				}
			}

			if (insindexup + 1 < MAX_INDEX) {
				InstanceIndexArray[insindexup] = index;
				insindexup += 1;
			}
		}
		else if (strcmp(rstr, "vt") == 0) {
			// uv 좌표
			glm::vec3 pos;
			in >> pos.x;
			in >> pos.y;
			in >> pos.z;
			if (insUVup + 1 < MAX_UV) {
				InstanceUVArray[insUVup] = pos;
				insUVup += 1;
			}
		}
		else if (strcmp(rstr, "vn") == 0) {
			// 노멀
			glm::vec3 pos;
			in >> pos.x;
			in >> pos.y;
			in >> pos.z;
			if (insNormalUp + 1 < MAX_UV) {
				InstanceNormalArray[insNormalUp] = pos;
				insNormalUp += 1;
			}
		}
	}

	reader->vertexCount = insposup;
	reader->posArr = (glm::vec3*)ShapeFreeMem->_New(sizeof(glm::vec3) * insposup);
	reader->colorArr = (glm::vec3*)ShapeFreeMem->_New(sizeof(glm::vec3) * insposup);
	reader->texUV_CoordArr = (glm::vec2*)ShapeFreeMem->_New(sizeof(glm::vec2) * insposup);
	reader->normalArr = (glm::vec3*)ShapeFreeMem->_New(sizeof(glm::vec3) * insposup);
	for (int i = 0; i < insposup; ++i) {
		reader->posArr[i] = { InstancePosArray[i].x, InstancePosArray[i].y, InstancePosArray[i].z };
		reader->colorArr[i].x = (float)(rand() % 100) / 100.0f;
		reader->colorArr[i].y = (float)(rand() % 100) / 100.0f;
		reader->colorArr[i].z = (float)(rand() % 100) / 100.0f;
		if (InstancePosArray[i].t >= 0) {
			glm::vec3 insv3 = InstanceUVArray[InstancePosArray[i].t];
			reader->texUV_CoordArr[i] = glm::vec2(insv3.x, insv3.y);
		}
		else {
			reader->texUV_CoordArr[i] = glm::vec2(0, 0);
		}

		if (InstancePosArray[i].n >= 0) {
			glm::vec3 insv3 = InstanceNormalArray[InstancePosArray[i].n];
			reader->normalArr[i] = glm::vec3(insv3.x, insv3.y, insv3.z);
		}
		else {
			reader->normalArr[i] = glm::vec3(0, 0, 0);
		}
	}

	reader->indexCount = insindexup;
	reader->IndexArr = (vec3uint*)ShapeFreeMem->_New(sizeof(vec3uint) * insindexup);
	for (int i = 0; i < insindexup; ++i) {
		reader->IndexArr[i].v1 = InstanceIndexArray[i].v1;
		reader->IndexArr[i].v2 = InstanceIndexArray[i].v2;
		reader->IndexArr[i].v3 = InstanceIndexArray[i].v3;
	}

	in.close();
}

bool loadOBJ(GLShapeOrigin* reader, const char* path)
{
	int uvup = 0;
	int normalup = 0;
	int vertexup = 0;

	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	ifstream in;
	in.open(path);

	int insposup = 0;
	int insindexup = 0;
	int insUVup = 0;
	int insrealposup = 0;
	int insNormalUp = 0;

	while (in.eof() == false) {
		char rstr[128] = {};
		in >> rstr;
		if (strcmp(rstr, "v") == 0) {
			//좌표
			glm::vec3 pos;
			in >> pos.x;
			in >> pos.y;
			in >> pos.z;
			temp_vertices.push_back(pos);
		}
		else if (strcmp(rstr, "vt") == 0) {
			// uv 좌표
			glm::vec3 uv;
			in >> uv.x;
			in >> uv.y;
			in >> uv.z;
			temp_uvs.push_back(glm::vec2(uv.x, uv.y));
		}
		else if (strcmp(rstr, "vn") == 0) {
			// 노멀
			glm::vec3 normal;
			in >> normal.x;
			in >> normal.y;
			in >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(rstr, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			char blank;

			in >> vertexIndex[0];
			in >> blank;
			in >> uvIndex[0];
			in >> blank;
			in >> normalIndex[0];
			//in >> blank;
			in >> vertexIndex[1];
			in >> blank;
			in >> uvIndex[1];
			in >> blank;
			in >> normalIndex[1];
			//in >> blank;
			in >> vertexIndex[2];
			in >> blank;
			in >> uvIndex[2];
			in >> blank;
			in >> normalIndex[2];
			//in >> blank;

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}
	// For each vertex of each triangle
	reader->vertexCount = vertexIndices.size();
	reader->posArr = (glm::vec3*)ShapeFreeMem->_New(sizeof(glm::vec3) * reader->vertexCount);
	reader->colorArr = (glm::vec3*)ShapeFreeMem->_New(sizeof(glm::vec3) * reader->vertexCount);
	reader->texUV_CoordArr = (glm::vec2*)ShapeFreeMem->_New(sizeof(glm::vec2) * reader->vertexCount);
	reader->normalArr = (glm::vec3*)ShapeFreeMem->_New(sizeof(glm::vec3) * reader->vertexCount);
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		unsigned int vertexIndex = vertexIndices[i];
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uvs = temp_uvs[uvIndices[i] - 1];
		glm::vec3 normal = temp_normals[normalIndices[i] - 1];
		reader->posArr[i] = vertex;
		reader->colorArr[i] = glm::vec3(1, 1, 1);
		reader->texUV_CoordArr[i] = uvs;
		reader->normalArr[i] = normal;
	}
	reader->isElement = false;

	return true;
}

void InitShapeOrigin_OBJ(GLShapeOrigin* origin) {

	InstanceFreeMem->ClearAll();
	GLfloat* vertices = (GLfloat*)InstanceFreeMem->_New(origin->vertexCount * 11 * sizeof(GLfloat));

	glm::vec3 maxpos = glm::vec3(0, 0, 0);
	for (int i = 0; i < origin->vertexCount; ++i) {
		vertices[11 * i] = origin->posArr[i].x;
		if (maxpos.x < fabsf(origin->posArr[i].x)) {
			maxpos.x = fabsf(origin->posArr[i].x);
		}
		vertices[11 * i + 1] = origin->posArr[i].y;
		if (maxpos.y < fabsf(origin->posArr[i].y)) {
			maxpos.y = fabsf(origin->posArr[i].y);
		}
		vertices[11 * i + 2] = origin->posArr[i].z;
		if (maxpos.z < fabsf(origin->posArr[i].z)) {
			maxpos.z = fabsf(origin->posArr[i].z);
		}
		vertices[11 * i + 3] = origin->colorArr[i].x;
		vertices[11 * i + 4] = origin->colorArr[i].y;
		vertices[11 * i + 5] = origin->colorArr[i].z;
		vertices[11 * i + 6] = origin->texUV_CoordArr[i].x;
		vertices[11 * i + 7] = origin->texUV_CoordArr[i].y;
		vertices[11 * i + 8] = origin->normalArr[i].x;
		vertices[11 * i + 9] = origin->normalArr[i].y;
		vertices[11 * i + 10] = origin->normalArr[i].z;
	}

	origin->BorderSize = maxpos;

	glGenVertexArrays(1, &origin->VAO);
	glGenBuffers(1, &origin->VBO[0]);

	glBindVertexArray(origin->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, origin->VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, origin->vertexCount * 11 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// normal attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);
}

GLShapeOrigin InitShapeOrigin_UI(shp::rect4f range) {
	InstanceFreeMem->ClearAll();
	GLShapeOrigin origin;
	int vertexSiz = 6;
	GLfloat* verts = (GLfloat*)InstanceFreeMem->_New(vertexSiz * 5 * sizeof(GLfloat));
	verts[0] = range.lx; verts[1] = range.ly; verts[2] = 0; verts[3] = 1; verts[4] = 1;
	verts[5] = range.lx; verts[6] = range.fy; verts[7] = 0; verts[8] = 1; verts[9] = 0;
	verts[10] = range.fx; verts[11] = range.fy; verts[12] = 0; verts[13] = 0; verts[14] = 0;
	verts[15] = range.fx; verts[16] = range.ly; verts[17] = 0; verts[18] = 0; verts[19] = 1;
	verts[20] = range.lx; verts[21] = range.ly; verts[22] = 0; verts[23] = 1; verts[24] = 1;
	verts[25] = range.fx; verts[26] = range.fy; verts[27] = 0; verts[28] = 0; verts[29] = 0;
	origin.BorderSize = glm::vec3(1, 1, 1);
	origin.vertexCount = vertexSiz;
	glGenVertexArrays(1, &origin.VAO);
	glGenBuffers(1, &origin.VBO[0]);

	glBindVertexArray(origin.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, origin.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexSiz * 5 * sizeof(GLfloat), verts, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	return origin;
}

void AddShape_FromFileName(const char* meshfile, const char* texturefile) {
	if (shapeup + 1 < MAX_SHAPE) {
		GLShapeOrigin o;
		loadOBJ(&o, meshfile);
		InitShapeOrigin_OBJ(&o);
		o.textureID = AddTexture(texturefile);
		shapeArr[shapeup] = o;
		shapeup += 1;
	}
}

void AddShape_FFN_withTex(const char* meshfile, GLuint texture) {
	if (shapeup + 1 < MAX_SHAPE) {
		GLShapeOrigin o;
		loadOBJ(&o, meshfile);
		InitShapeOrigin_OBJ(&o);
		o.textureID = texture;
		shapeArr[shapeup] = o;
		shapeup += 1;
	}
}

void AddAnimShape_FromFileName(const char* meshfile, const char* texturefile) {
	if (AFShapeup + 1 < MAX_ANIMFRAG_SHAPE) {
		GLShapeOrigin o;
		loadOBJ(&o, meshfile);
		InitShapeOrigin_OBJ(&o);
		o.textureID = AddTexture(texturefile);
		AnimFragShapeArr[AFShapeup] = o;
		AFShapeup += 1;
	}
}

void InitShapeOrigin(GLShapeOrigin* origin) {
	InstanceFreeMem->ClearAll();
	GLfloat* vertices = (GLfloat*)InstanceFreeMem->_New(origin->vertexCount * 11 * sizeof(GLfloat));
	for (int i = 0; i < origin->vertexCount; ++i) {
		vertices[11 * i] = origin->posArr[i].x;
		vertices[11 * i + 1] = origin->posArr[i].y;
		vertices[11 * i + 2] = origin->posArr[i].z;
		vertices[11 * i + 3] = origin->colorArr[i].x;
		vertices[11 * i + 4] = origin->colorArr[i].y;
		vertices[11 * i + 5] = origin->colorArr[i].z;
		vertices[11 * i + 6] = origin->texUV_CoordArr[i].x;
		vertices[11 * i + 7] = origin->texUV_CoordArr[i].y;
		vertices[11 * i + 8] = origin->normalArr[i].x;
		vertices[11 * i + 9] = origin->normalArr[i].y;
		vertices[11 * i + 10] = origin->normalArr[i].z;
	}

	glGenVertexArrays(1, &origin->VAO);
	glGenBuffers(1, &origin->VBO[0]);
	glGenBuffers(1, &origin->EBO);

	glBindVertexArray(origin->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, origin->VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, origin->vertexCount * 11 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, origin->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, origin->indexCount * sizeof(vec3uint), origin->IndexArr, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// normal attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);
}

glm::mat4 CameraTrasform = glm::mat4(1.0f);
glm::vec3 CamPos = glm::vec3(0, 0, -5);
glm::vec3 viewDir;

glm::vec2 fov = glm::vec2(0, shp::PI * 60.0f / 180.0f);
float aspect = 1.42857f;
float fsc = 1000;
float nsc = 1;
glm::mat4 ViewTransform = glm::mat4(1.0f);

void SetCameraTransform(glm::vec3 pos, glm::vec3 ViewDir) {
	/*glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 X, Y, Z;
	Z = glm::normalize(ViewDir);
	X = glm::cross(Z, UP);
	Y = glm::cross(X, Z);

	CameraTrasform = glm::mat4(1.0f);
	CameraTrasform = glm::mat4(
		X.x, X.y, X.z, 0,
		Y.x, Y.y, Z.z, 0,
		-Z.x, -Z.y, -Z.z, 0,
		0, 0, 0, 1
	) * CameraTrasform;

	CameraTrasform = glm::translate(CameraTrasform, -pos);*/

	CameraTrasform = glm::lookAt(pos, ViewDir, glm::vec3(0, 1, 0));
	/*glm::mat4 Mproj = glm::mat4(
		glm::cot(fov.y / 2) / aspect, 0, 0, 0,
		0, glm::cot(fov.y / 2), 0, 0,
		0, 0, -fsc / (fsc - nsc), -nsc * fsc / (fsc - nsc),
		0, 0, -1, 0
	);
	CameraTrasform = CameraTrasform * Mproj;*/
}

void SetViewTransform() {
	glm::mat4 Mproj = glm::mat4(
		glm::cot(fov.y / 2) / aspect, 0, 0, 0,
		0, glm::cot(fov.y / 2), 0, 0,
		0, 0, -fsc / (fsc - nsc), -nsc * fsc / (fsc - nsc),
		0, 0, -1, 0
	);

	ViewTransform = Mproj;
}

GLuint Setshader(const char* vshader, const char* fshader);
char* filetobuf(char* file);

glm::vec3 lightPos(0, 0, 0);

struct DirLight {
	glm::vec3 direction;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

struct PointLight {
	glm::vec3 position;

	float intencity;
	float maxRange = 20;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

struct SpotLight {
	glm::vec3 position;
	glm::vec3 direction;

	float intencity;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float maxRange = 20;
};

constexpr unsigned int LightGetsCount = 10;

struct PointLightGets {
	PointLight* pl[LightGetsCount] = {};
	unsigned int maxnum = 0;
};

struct SpotLightGets {
	SpotLight* pl[LightGetsCount] = {};
	unsigned int maxnum = 0;
};

struct PointLight_Check {
	PointLight* ptr = nullptr;
	int check = 0;
};

struct SpotLight_Check {
	SpotLight* ptr = nullptr;
	int check = 0;
};

class LightSystem {
public:
	LightSystem() {
		PLC = FM_Model0();
		PLC.SetHeapData((byte8*)malloc(sizeof(PointLight_Check) * POINT_LIGHT_MAX), sizeof(PointLight_Check) * POINT_LIGHT_MAX);

		SLC = FM_Model0();
		SLC.SetHeapData((byte8*)malloc(sizeof(SpotLight_Check) * SPOT_LIGHT_MAX), sizeof(SpotLight_Check) * SPOT_LIGHT_MAX);
	}
	virtual ~LightSystem() {
	}

	// 환경 라이팅
	DirLight dirLight;

	// 포인트라이트
	static constexpr unsigned int POINT_LIGHT_MAX = 100;
	PointLight pointLightArr[POINT_LIGHT_MAX] = {};
	PointLight* PointLightXSort_PtrArr[POINT_LIGHT_MAX] = {}; // X 좌표 정렬
	PointLight* PointLightYSort_PtrArr[POINT_LIGHT_MAX] = {}; // Y 좌표 정렬
	PointLight* PointLightZSort_PtrArr[POINT_LIGHT_MAX] = {}; // Z 좌표 정렬
	unsigned int pointlight_up = 0;

	// 포인트라이트 연산을 위한 프리메모리
	FM_Model0 PLC;

	// 스포트라이트
	static constexpr unsigned int SPOT_LIGHT_MAX = 30;
	SpotLight spotLightArr[SPOT_LIGHT_MAX] = {};
	SpotLight* SpotLightXSort_PtrArr[SPOT_LIGHT_MAX] = {};
	SpotLight* SpotLightYSort_PtrArr[SPOT_LIGHT_MAX] = {};
	SpotLight* SpotLightZSort_PtrArr[SPOT_LIGHT_MAX] = {};
	unsigned int spotlight_up = 0;

	// 스포트라이트 연산을 위한 프리메모리
	FM_Model0 SLC;

	PointLight* AddPointLight(glm::vec3 position, float range, float intencity,
		glm::vec3 ambientColor, glm::vec3 diffuseColor, glm::vec3 specularColor) {
		if (pointlight_up + 1 < POINT_LIGHT_MAX) {
			PointLight pl;
			pl.ambient = ambientColor;
			pl.diffuse = diffuseColor;
			pl.specular = specularColor;
			pl.maxRange = range;
			pl.intencity = intencity;
			pl.position = position;
			pointLightArr[pointlight_up] = pl;
			PointLightXSort_PtrArr[pointlight_up] = &pointLightArr[pointlight_up];
			PointLightYSort_PtrArr[pointlight_up] = &pointLightArr[pointlight_up];
			PointLightZSort_PtrArr[pointlight_up] = &pointLightArr[pointlight_up];

			bool xdecide = false;
			bool ydecide = false;
			bool zdecide = false;
			for (int i = 0; i < pointlight_up; ++i) {
				if (xdecide == false && PointLightXSort_PtrArr[i]->position.x > pl.position.x) {
					for (int k = pointlight_up; k > i; --k) {
						PointLightXSort_PtrArr[k] = PointLightXSort_PtrArr[k - 1];
					}
					PointLightXSort_PtrArr[i] = &pointLightArr[pointlight_up];
					xdecide = true;
				}

				if (ydecide == false && PointLightYSort_PtrArr[i]->position.y > pl.position.y) {
					for (int k = pointlight_up; k > i; --k) {
						PointLightYSort_PtrArr[k] = PointLightYSort_PtrArr[k - 1];
					}
					PointLightYSort_PtrArr[i] = &pointLightArr[pointlight_up];
					ydecide = true;
				}

				if (zdecide == false && PointLightZSort_PtrArr[i]->position.z > pl.position.z) {
					for (int k = pointlight_up; k > i; --k) {
						PointLightZSort_PtrArr[k] = PointLightZSort_PtrArr[k - 1];
					}
					PointLightZSort_PtrArr[i] = &pointLightArr[pointlight_up];
					zdecide = true;
				}

				if (and3(xdecide, ydecide, zdecide)) {
					break;
				}
			}

			PointLight* plptr = &pointLightArr[pointlight_up];
			pointlight_up += 1;
			return plptr;
		}

		return nullptr;
	}

	SpotLight* AddSpotLight(glm::vec3 position, float range, float intencity,
		glm::vec3 ambientColor, glm::vec3 diffuseColor, glm::vec3 specularColor, float CutOff, float outerCutoff) {
		if (spotlight_up + 1 < SPOT_LIGHT_MAX) {
			SpotLight pl;
			pl.ambient = ambientColor;
			pl.diffuse = diffuseColor;
			pl.specular = specularColor;
			pl.maxRange = range;
			pl.intencity = intencity;
			pl.position = position;
			pl.cutOff = CutOff;
			pl.outerCutOff = outerCutoff;
			spotLightArr[spotlight_up] = pl;

			bool xdecide = false;
			bool ydecide = false;
			bool zdecide = false;
			for (int i = 0; i < spotlight_up; ++i) {
				if (xdecide == false && SpotLightXSort_PtrArr[i]->position.x > pl.position.x) {
					for (int k = spotlight_up - 1; k > i; --k) {
						SpotLightXSort_PtrArr[k] = SpotLightXSort_PtrArr[k - 1];
					}
					SpotLightXSort_PtrArr[i] = &spotLightArr[spotlight_up];
					xdecide = true;
				}

				if (ydecide == false && SpotLightYSort_PtrArr[i]->position.y > pl.position.y) {
					for (int k = spotlight_up - 1; k > i; --k) {
						SpotLightYSort_PtrArr[k] = SpotLightYSort_PtrArr[k - 1];
					}
					SpotLightYSort_PtrArr[i] = &spotLightArr[spotlight_up];
					ydecide = true;
				}

				if (zdecide == false && SpotLightZSort_PtrArr[i]->position.z > pl.position.z) {
					for (int k = spotlight_up - 1; k > i; --k) {
						SpotLightZSort_PtrArr[k] = SpotLightZSort_PtrArr[k - 1];
					}
					SpotLightZSort_PtrArr[i] = &spotLightArr[spotlight_up];
					zdecide = true;
				}

				if (and3(xdecide, ydecide, zdecide)) {
					break;
				}
			}

			SpotLight* slptr = &spotLightArr[spotlight_up];
			spotlight_up += 1;
			return slptr;
		}

		return nullptr;
	}

	// 아니 왜 maxnum이 10이 안되는지 의문이다. - 정렬이 제대로 안됬어도 이게 말이 되나 싶네
	// 3log(N) + 6 + x * (6 + i) = 3log(N) + 6 + 36 ~ 72? + 18 = 3log(N) + 78
	PointLightGets GetExcutingPointLights(shp::cube6f range) {
		// 세계에 빛이 10개 이하일때
		if (pointlight_up < 10) {
			PointLightGets plg;
			plg.maxnum = pointlight_up;
			for (int i = 0; i < plg.maxnum; ++i) {
				plg.pl[i] = &pointLightArr[i];
			}
			return plg;
		}

		// 피벗을 중앙에 배치
		int xpos, ypos, zpos;
		xpos = pointlight_up / 2;
		ypos = xpos;
		zpos = xpos;

		int xadd, yadd, zadd;
		xadd = pointlight_up / 4;
		yadd = xadd;
		zadd = xadd;

		bool xdecide = false;
		bool ydecide = false;
		bool zdecide = false;

		shp::vec3f cen = range.getCenter();

		while (true) {
			if (xdecide == false) {
				if (PointLightXSort_PtrArr[xpos]->position.x > cen.x) {
					xpos -= xadd;
					xadd /= 2;
				}
				else {
					xpos += xadd;
					xadd /= 2;
				}

				if (xadd == 0) {
					xdecide = true;
				}
			}

			if (ydecide == false) {
				if (PointLightYSort_PtrArr[ypos]->position.y > cen.y) {
					ypos -= yadd;
					yadd /= 2;
				}
				else {
					ypos += yadd;
					yadd /= 2;
				}

				if (yadd == 0) {
					ydecide = true;
				}
			}

			if (zdecide == false) {
				if (PointLightZSort_PtrArr[zpos]->position.z > cen.z) {
					zpos -= zadd;
					zadd /= 2;
				}
				else {
					zpos += zadd;
					zadd /= 2;
				}

				if (zadd == 0) {
					zdecide = true;
				}
			}

			if (and3(xdecide, ydecide, zdecide)) {
				break;
			}
		}

		PointLightGets plg;
		plg.maxnum = 0;
		int xp_delta = 0;
		int xm_delta = 1;
		int yp_delta = 0;
		int ym_delta = 1;
		int zp_delta = 0;
		int zm_delta = 1;
		float xyzMinMax = 10000000000.0f;

		int stack = 0;
		int mini = -1;
		float len[6] = {};
		for (int i = 0; i < 6; ++i) {
			len[i] = xyzMinMax;
		}

		PLC.ClearAll();
		int repeatnum = 0;
		while (repeatnum < 1024) {
			switch (mini) {
			case 0:
				if (xpos + xp_delta < pointlight_up) {
					len[0] = fabsf(PointLightXSort_PtrArr[xpos + xp_delta]->position.x - cen.x);
				}
				else {
					len[0] = 10000000000.0f;
				}
				break;
			case 1:
				if (xpos - xm_delta >= 0) {
					len[1] = fabsf(PointLightXSort_PtrArr[xpos - xm_delta]->position.x - cen.x);
				}
				else {
					len[1] = 10000000000.0f;
				}
				break;
			case 2:
				if (ypos + yp_delta < pointlight_up) {
					len[2] = fabsf(PointLightYSort_PtrArr[ypos + yp_delta]->position.y - cen.y);
				}
				else {
					len[2] = 10000000000.0f;
				}
				break;
			case 3:
				if (ypos - ym_delta >= 0) {
					len[3] = fabsf(PointLightYSort_PtrArr[ypos - ym_delta]->position.y - cen.y);
				}
				else {
					len[3] = 10000000000.0f;
				}
				break;
			case 4:
				if (zpos + zp_delta < pointlight_up) {
					len[4] = fabsf(PointLightZSort_PtrArr[zpos + zp_delta]->position.z - cen.z);
				}
				else {
					len[4] = 10000000000.0f;
				}
				break;
			case 5:
				if (zpos - zm_delta >= 0) {
					len[5] = fabsf(PointLightZSort_PtrArr[zpos - zm_delta]->position.z - cen.z);
				}
				else {
					len[5] = 10000000000.0f;
				}
				break;
			case -1:
			{
				if (xpos + xp_delta < pointlight_up) {
					len[0] = fabsf(PointLightXSort_PtrArr[xpos + xp_delta]->position.x - cen.x);
				}

				if (xpos - xm_delta >= 0) {
					len[1] = fabsf(PointLightXSort_PtrArr[xpos - xm_delta]->position.x - cen.x);
				}

				if (ypos + yp_delta < pointlight_up) {
					len[2] = fabsf(PointLightYSort_PtrArr[ypos + yp_delta]->position.y - cen.y);
				}

				if (ypos - ym_delta >= 0) {
					len[3] = fabsf(PointLightYSort_PtrArr[ypos - ym_delta]->position.y - cen.y);
				}

				if (zpos + zp_delta < pointlight_up) {
					len[4] = fabsf(PointLightZSort_PtrArr[zpos + zp_delta]->position.z - cen.z);
				}

				if (zpos - zm_delta >= 0) {
					len[5] = fabsf(PointLightZSort_PtrArr[zpos - zm_delta]->position.z - cen.z);
				}
				break;
			}
			}

			xyzMinMax = 10000000000.0f;

			for (int i = 0; i < 6; ++i) {
				if (len[i] < xyzMinMax) {
					xyzMinMax = len[i];
					mini = i;
				}
			}

			bool isExist = false;
			switch (mini) {
			case 0:
			{
				if (xpos + xp_delta > pointlight_up - 1) {
					break;
				}

				for (int i = 0; i < stack; ++i) {
					PointLight_Check* ins = (PointLight_Check*)(&PLC.Data[sizeof(PointLight_Check) * i]);
					if (PointLightXSort_PtrArr[xpos + xp_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}

				if (isExist == false) {
					PointLight_Check* plc = (PointLight_Check*)PLC._New(sizeof(PointLight_Check));
					plc->ptr = PointLightXSort_PtrArr[xpos + xp_delta];
					plc->check = 1;
					xp_delta += 1;
					stack += 1;
				}

				++xp_delta;
				break;
			}
			case 1:
			{
				if (xpos - xm_delta < 0) {
					break;
				}

				for (int i = 0; i < stack; ++i) {
					PointLight_Check* ins = (PointLight_Check*)(&PLC.Data[sizeof(PointLight_Check) * i]);
					if (PointLightXSort_PtrArr[xpos - xm_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					PointLight_Check* plc = (PointLight_Check*)PLC._New(sizeof(PointLight_Check));
					plc->ptr = PointLightXSort_PtrArr[xpos - xm_delta];
					plc->check = 1;
					stack += 1;
				}

				++xm_delta;
				break;
			}
			case 2:
			{
				if (ypos + yp_delta > pointlight_up - 1) {
					break;
				}

				for (int i = 0; i < stack; ++i) {
					PointLight_Check* ins = (PointLight_Check*)(&PLC.Data[sizeof(PointLight_Check) * i]);
					if (PointLightYSort_PtrArr[ypos + yp_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					PointLight_Check* plc = (PointLight_Check*)PLC._New(sizeof(PointLight_Check));
					plc->ptr = PointLightYSort_PtrArr[ypos + yp_delta];
					plc->check = 1;
					stack += 1;
				}

				++yp_delta;
				break;
			}
			case 3:
			{
				if (ypos - ym_delta < 0) {
					break;
				}

				for (int i = 0; i < stack; ++i) {
					PointLight_Check* ins = (PointLight_Check*)(&PLC.Data[sizeof(PointLight_Check) * i]);
					if (PointLightYSort_PtrArr[ypos - ym_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					PointLight_Check* plc = (PointLight_Check*)PLC._New(sizeof(PointLight_Check));
					plc->ptr = PointLightYSort_PtrArr[ypos - ym_delta];
					plc->check = 1;
					stack += 1;
				}

				++ym_delta;
				break;
			}
			case 4:
			{
				if (zpos + zp_delta > pointlight_up - 1) {
					break;
				}

				for (int i = 0; i < stack; ++i) {
					PointLight_Check* ins = (PointLight_Check*)(&PLC.Data[sizeof(PointLight_Check) * i]);
					if (PointLightZSort_PtrArr[zpos + zp_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					PointLight_Check* plc = (PointLight_Check*)PLC._New(sizeof(PointLight_Check));
					plc->ptr = PointLightZSort_PtrArr[zpos + zp_delta];
					plc->check = 1;
					stack += 1;
				}

				++zp_delta;
				break;
			}
			case 5:
			{
				if (zpos - zm_delta < 0) {
					break;
				}

				for (int i = 0; i < stack; ++i) {
					PointLight_Check* ins = (PointLight_Check*)(&PLC.Data[sizeof(PointLight_Check) * i]);
					if (PointLightZSort_PtrArr[zpos - zm_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					PointLight_Check* plc = (PointLight_Check*)PLC._New(sizeof(PointLight_Check));
					plc->ptr = PointLightZSort_PtrArr[zpos - zm_delta];
					plc->check = 1;
					stack += 1;
				}

				++zm_delta;
				break;
			}
			}

			if (plg.maxnum >= 10) {
				break;
			}

			++repeatnum;
		}

		return plg;
	}

	SpotLightGets GetExcutingSpotLights(shp::cube6f range) {
		// 세계에 빛이 10개 이하일때
		if (spotlight_up < 10) {
			SpotLightGets plg;
			plg.maxnum = spotlight_up;
			for (int i = 0; i < plg.maxnum; ++i) {
				plg.pl[i] = &spotLightArr[i];
			}
			return plg;
		}

		// 피벗을 중앙에 배치
		int xpos, ypos, zpos;
		xpos = spotlight_up / 2;
		ypos = xpos;
		zpos = xpos;

		int xadd, yadd, zadd;
		xadd = spotlight_up / 4;
		yadd = xadd;
		zadd = xadd;

		bool xdecide = false;
		bool ydecide = false;
		bool zdecide = false;

		shp::vec3f cen = range.getCenter();

		while (true) {
			if (xdecide == false) {
				if (SpotLightXSort_PtrArr[xpos]->position.x > cen.x) {
					xpos += xadd;
					xadd /= 2;
				}
				else {
					xpos -= xadd;
					xadd /= 2;
				}

				if (xadd == 0) {
					xdecide = true;
				}
			}

			if (ydecide == false) {
				if (SpotLightYSort_PtrArr[ypos]->position.y > cen.y) {
					ypos += yadd;
					yadd /= 2;
				}
				else {
					ypos -= yadd;
					yadd /= 2;
				}

				if (yadd == 0) {
					ydecide = true;
				}
			}

			if (zdecide == false) {
				if (SpotLightZSort_PtrArr[zpos]->position.z > cen.z) {
					zpos += zadd;
					zadd /= 2;
				}
				else {
					zpos -= zadd;
					zadd /= 2;
				}

				if (zadd == 0) {
					zdecide = true;
				}
			}

			if (and3(xdecide, ydecide, zdecide)) {
				break;
			}
		}

		SpotLightGets plg;
		plg.maxnum = 0;
		int xp_delta = 0;
		int xm_delta = 1;
		int yp_delta = 0;
		int ym_delta = 1;
		int zp_delta = 0;
		int zm_delta = 1;
		float xyzMinMax = 10000000000.0f;

		int stack = 0;
		int mini = -1;
		float len[6] = {};
		for (int i = 0; i < 6; ++i) {
			len[i] = xyzMinMax;
		}

		PLC.ClearAll();
		while (true) {
			switch (mini) {
			case 0:
				if (xpos + xp_delta < spotlight_up) {
					len[0] = fabsf(SpotLightXSort_PtrArr[xpos + xp_delta]->position.x - cen.x);
				}
				break;
			case 1:
				if (xpos - xm_delta >= 0) {
					len[1] = fabsf(SpotLightXSort_PtrArr[xpos - xm_delta]->position.x - cen.x);
				}
				break;
			case 2:
				if (ypos + yp_delta < spotlight_up) {
					len[2] = fabsf(SpotLightYSort_PtrArr[ypos + yp_delta]->position.y - cen.y);
				}
				break;
			case 3:
				if (ypos - ym_delta >= 0) {
					len[3] = fabsf(SpotLightYSort_PtrArr[ypos - ym_delta]->position.y - cen.y);
				}
				break;
			case 4:
				if (zpos + zp_delta < spotlight_up) {
					len[4] = fabsf(SpotLightZSort_PtrArr[zpos + zp_delta]->position.z - cen.z);
				}
				break;
			case 5:
				if (zpos - zm_delta >= 0) {
					len[5] = fabsf(SpotLightZSort_PtrArr[zpos - zm_delta]->position.z - cen.z);
				}
				break;
			case -1:
			{
				if (xpos + xp_delta < spotlight_up) {
					len[0] = fabsf(SpotLightXSort_PtrArr[xpos + xp_delta]->position.x - cen.x);
				}

				if (xpos - xm_delta >= 0) {
					len[1] = fabsf(SpotLightXSort_PtrArr[xpos - xm_delta]->position.x - cen.x);
				}

				if (ypos + yp_delta < spotlight_up) {
					len[2] = fabsf(SpotLightYSort_PtrArr[ypos + yp_delta]->position.y - cen.y);
				}

				if (ypos - ym_delta >= 0) {
					len[3] = fabsf(SpotLightYSort_PtrArr[ypos - ym_delta]->position.y - cen.y);
				}

				if (zpos + zp_delta < spotlight_up) {
					len[4] = fabsf(SpotLightZSort_PtrArr[zpos + zp_delta]->position.z - cen.z);
				}

				if (zpos - zm_delta >= 0) {
					len[5] = fabsf(SpotLightZSort_PtrArr[zpos - zm_delta]->position.z - cen.z);
				}
				break;
			}
			}

			xyzMinMax = 10000000000.0f;

			for (int i = 0; i < 6; ++i) {
				if (len[i] < xyzMinMax) {
					xyzMinMax = len[i];
					mini = i;
				}
			}

			bool isExist = false;
			switch (mini) {
			case 0:
			{
				for (int i = 0; i < stack; ++i) {
					SpotLight_Check* ins = (SpotLight_Check*)(&PLC.Data[sizeof(SpotLight_Check) * i]);
					if (SpotLightXSort_PtrArr[xpos + xp_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}

				if (isExist == false) {
					SpotLight_Check* plc = (SpotLight_Check*)PLC._New(sizeof(SpotLight_Check));
					plc->ptr = SpotLightXSort_PtrArr[xpos + xp_delta];
					plc->check = 1;
					xp_delta += 1;
					stack += 1;
				}

				++xp_delta;
				break;
			}
			case 1:
			{
				for (int i = 0; i < stack; ++i) {
					SpotLight_Check* ins = (SpotLight_Check*)(&PLC.Data[sizeof(SpotLight_Check) * i]);
					if (SpotLightXSort_PtrArr[xpos - xm_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					SpotLight_Check* plc = (SpotLight_Check*)PLC._New(sizeof(SpotLight_Check));
					plc->ptr = SpotLightXSort_PtrArr[xpos - xm_delta];
					plc->check = 1;
					stack += 1;
				}

				++xm_delta;
				break;
			}
			case 2:
			{
				for (int i = 0; i < stack; ++i) {
					SpotLight_Check* ins = (SpotLight_Check*)(&PLC.Data[sizeof(SpotLight_Check) * i]);
					if (SpotLightYSort_PtrArr[ypos + yp_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					SpotLight_Check* plc = (SpotLight_Check*)PLC._New(sizeof(SpotLight_Check));
					plc->ptr = SpotLightYSort_PtrArr[ypos + yp_delta];
					plc->check = 1;
					stack += 1;
				}

				++yp_delta;
				break;
			}
			case 3:
			{
				for (int i = 0; i < stack; ++i) {
					SpotLight_Check* ins = (SpotLight_Check*)(&PLC.Data[sizeof(SpotLight_Check) * i]);
					if (SpotLightYSort_PtrArr[ypos - ym_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					SpotLight_Check* plc = (SpotLight_Check*)PLC._New(sizeof(SpotLight_Check));
					plc->ptr = SpotLightYSort_PtrArr[ypos - ym_delta];
					plc->check = 1;
					stack += 1;
				}

				++ym_delta;
				break;
			}
			case 4:
			{
				for (int i = 0; i < stack; ++i) {
					SpotLight_Check* ins = (SpotLight_Check*)(&PLC.Data[sizeof(SpotLight_Check) * i]);
					if (SpotLightZSort_PtrArr[zpos + zp_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					SpotLight_Check* plc = (SpotLight_Check*)PLC._New(sizeof(SpotLight_Check));
					plc->ptr = SpotLightZSort_PtrArr[zpos + zp_delta];
					plc->check = 1;
					stack += 1;
				}

				++zp_delta;
				break;
			}
			case 5:
			{
				for (int i = 0; i < stack; ++i) {
					SpotLight_Check* ins = (SpotLight_Check*)(&PLC.Data[sizeof(SpotLight_Check) * i]);
					if (SpotLightZSort_PtrArr[zpos - zm_delta] == ins->ptr && ins->check < 3) {
						ins->check += 1;
						isExist = true;
						if (ins->check >= 3) {
							if (plg.maxnum + 1 <= 10) {
								plg.pl[plg.maxnum] = ins->ptr;
								plg.maxnum += 1;
							}
						}
					}
				}
				if (isExist == false) {
					SpotLight_Check* plc = (SpotLight_Check*)PLC._New(sizeof(SpotLight_Check));
					plc->ptr = SpotLightZSort_PtrArr[zpos - zm_delta];
					plc->check = 1;
					stack += 1;
				}

				++zm_delta;
				break;
			}
			}

			if (plg.maxnum >= 10) {
				break;
			}
		}

		return plg;
	}
};

LightSystem lightSys;

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};
class GLSkyBox {
public:
	unsigned int cubemapTexture;
	unsigned int skyboxVAO, skyboxVBO;
	GLuint skyboxShader;

	GLSkyBox() {}
	virtual ~GLSkyBox() {}

	void Init() {
		skyboxShader = Setshader("Shaders\\ST3_CubeMap_VShader.glsl", "Shaders\\ST3_CubeMap_FShader.glsl");

		vector<std::string> faces
		{
			"Resources\\skybox\\right.jpg",
			"Resources\\skybox\\left.jpg",
			"Resources\\skybox\\top.jpg",
			"Resources\\skybox\\bottom.jpg",
			"Resources\\skybox\\front.jpg",
			"Resources\\skybox\\back.jpg"
		};

		cubemapTexture = loadCubemap(faces);

		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	}

	void Draw() {
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

		glUseProgram(skyboxShader);

		unsigned int uid = glGetUniformLocation(skyboxShader, "view");
		if (uid != -1) {
			glm::mat4 vm = glm::mat4(glm::mat3(CameraTrasform));
			glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(vm));
		}

		uid = glGetUniformLocation(skyboxShader, "projection");
		if (uid != -1) {
			glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(ViewTransform));
		}

		uid = glGetUniformLocation(skyboxShader, "distance");
		if (uid != -1) {
			glUniform1f(uid, sqrtf(CamPos.x * CamPos.x + CamPos.y * CamPos.y + CamPos.z * CamPos.z));
		}

		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default
	}
};

void DrawString(HDC hdc, const char* fontname, const int fontsiz, const TCHAR* str, shp::rect4f loc, Color4f color,
	TextSortStruct tss, bool bDebug);

//모델의 인스턴스
bool LineDebug = false;
bool useTRS = true;
glm::mat4 TransformMatrix = glm::mat4(1.0f);
class GLShapeInstance {
public:
	//siz : 4*5 + 12*3 = 20 + 36 = 56 byte?
	size_t shapeorigin_ID = 0;
	//GLShapeOrigin* shape_origin;
	GLuint ShaderProgram;

	GLuint Ambient_Tex2d = 0;
	GLuint Diffuse_Tex2d = 0;
	GLuint Specular_Tex2d = 0;

	//인스턴스의 트랜스폼
	glm::vec3 Translate;
	glm::vec3 Rotation;
	glm::vec3 Scale;

	unsigned int GLMod = GL_TRIANGLES;

	GLShapeInstance() {

	}

	virtual ~GLShapeInstance() {
	}

	GLShapeInstance* Init() {
		shapeorigin_ID = 0;
		ShaderProgram = 0;
		Ambient_Tex2d = 0;
		Diffuse_Tex2d = 0;
		Specular_Tex2d = 0;
		Translate = glm::vec3(0, 0, 0);
		Rotation = glm::vec3(0, 0, 0);
		Scale = glm::vec3(1, 1, 1);
		GLMod = GL_TRIANGLES;

		//가상함수 테이블 설정
		GLShapeInstance go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		return this;
	}

	void SetShader(GLuint shader) {
		ShaderProgram = shader;
	}

	void SetTranslate(glm::vec3 tr) {
		Translate = tr;
	}

	void SetRotation(glm::vec3 rot) {
		Rotation = rot;
	}

	void SetScale(glm::vec3 siz) {
		Scale = siz;
	}

	glm::mat4 GetTransformMatrix() {
		glm::mat4 TransformMat = glm::mat4(1.0f);
		TransformMat = glm::translate(TransformMat, Translate);
		TransformMat = glm::rotate(TransformMat, glm::radians(Rotation.x), glm::vec3(1, 0, 0));
		TransformMat = glm::rotate(TransformMat, glm::radians(Rotation.y), glm::vec3(0, 1, 0));
		TransformMat = glm::rotate(TransformMat, glm::radians(Rotation.z), glm::vec3(0, 0, 1));
		TransformMat = glm::scale(TransformMat, Scale);
		return TransformMat;
	}

	// type : d(diffuse map) | a(ambient occulsion map) | s(specular map) | n(normal map)
	void SetTexture(char type, unsigned int texture) {
		switch (type) {
		case 'd':
			Diffuse_Tex2d = texture;
			break;
		case 'a':
			Ambient_Tex2d = texture;
			break;
		case 's':
			Specular_Tex2d = texture;
			break;
		}

	}

	void Draw() {
		glm::mat4 TransformMat = glm::mat4(1.0f);
		if (useTRS) {
			TransformMat = glm::translate(TransformMat, Translate);
			TransformMat = glm::rotate(TransformMat, glm::radians(Rotation.x), glm::vec3(1, 0, 0));
			TransformMat = glm::rotate(TransformMat, glm::radians(Rotation.y), glm::vec3(0, 1, 0));
			TransformMat = glm::rotate(TransformMat, glm::radians(Rotation.z), glm::vec3(0, 0, 1));
			TransformMat = glm::scale(TransformMat, Scale);
		}
		else {
			TransformMat = TransformMatrix;
		}


		GLShapeOrigin* shape_origin = &shapeArr[shapeorigin_ID];
		if (shape_origin->textureID != -1) {
			Diffuse_Tex2d = shape_origin->textureID;
			Ambient_Tex2d = shape_origin->textureID;
			Specular_Tex2d = shape_origin->textureID;
		}

		unsigned int uid = 0;
		glUseProgram(ShaderProgram);

		int ltype = 0;

		uid = glGetUniformLocation(ShaderProgram, "dirLight.direction");
		if (uid != -1) {
			glUniform3f(uid, parametic3xyz(lightSys.dirLight.direction));
		}

		uid = glGetUniformLocation(ShaderProgram, "dirLight.ambient");
		if (uid != -1) {
			glUniform3f(uid, parametic3xyz(lightSys.dirLight.ambient));
		}

		uid = glGetUniformLocation(ShaderProgram, "dirLight.diffuse");
		if (uid != -1) {
			glUniform3f(uid, parametic3xyz(lightSys.dirLight.diffuse));
		}

		uid = glGetUniformLocation(ShaderProgram, "dirLight.specular");
		if (uid != -1) {
			glUniform3f(uid, parametic3xyz(lightSys.dirLight.specular));
		}

		shp::cube6f range = shp::cube6f(parametic3xyz(Translate), parametic3xyz(Translate));

		uid = glGetUniformLocation(ShaderProgram, "PointLightNum");
		if (uid != -1) {
			PointLightGets plg = lightSys.GetExcutingPointLights(range);
			char posstr[] = "pointLights[0].position";
			char rangestr[] = "pointLights[0].range";
			char intencitystr[] = "pointLights[0].intencity";
			char ambientstr[] = "pointLights[0].ambient";
			char diffusestr[] = "pointLights[0].diffuse";
			char specularstr[] = "pointLights[0].specular";

			glUniform1i(uid, plg.maxnum);

			for (int i = 0; i < plg.maxnum; ++i) {
				uid = glGetUniformLocation(ShaderProgram, posstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(plg.pl[i]->position));
				}

				uid = glGetUniformLocation(ShaderProgram, rangestr);
				if (uid != -1) {
					glUniform1f(uid, plg.pl[i]->maxRange);
				}

				uid = glGetUniformLocation(ShaderProgram, intencitystr);
				if (uid != -1) {
					glUniform1f(uid, plg.pl[i]->intencity);
				}

				uid = glGetUniformLocation(ShaderProgram, ambientstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(plg.pl[i]->ambient));
				}

				uid = glGetUniformLocation(ShaderProgram, diffusestr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(plg.pl[i]->diffuse));
				}

				uid = glGetUniformLocation(ShaderProgram, specularstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(plg.pl[i]->specular));
				}

				plus6(posstr[12], rangestr[12], intencitystr[12], ambientstr[12], diffusestr[12], specularstr[12]);
			}
		}

		uid = glGetUniformLocation(ShaderProgram, "SpotLightNum");
		if (uid != -1) {
			SpotLightGets slg = lightSys.GetExcutingSpotLights(range);
			char sposstr[] = "SpotLights[0].position";
			char srangestr[] = "SpotLights[0].range";
			char sintencitystr[] = "SpotLights[0].intencity";
			char sambientstr[] = "SpotLights[0].ambient";
			char sdiffusestr[] = "SpotLights[0].diffuse";
			char sspecularstr[] = "SpotLights[0].specular";
			char scutoffstr[] = "SpotLights[0].cutOff";
			char soutercutoffstr[] = "SpotLights[0].outerCutOff";

			glUniform1i(uid, slg.maxnum);

			for (int i = 0; i < slg.maxnum; ++i) {
				uid = glGetUniformLocation(ShaderProgram, sposstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(slg.pl[i]->position));
				}

				uid = glGetUniformLocation(ShaderProgram, srangestr);
				if (uid != -1) {
					glUniform1f(uid, slg.pl[i]->maxRange);
				}

				uid = glGetUniformLocation(ShaderProgram, sintencitystr);
				if (uid != -1) {
					glUniform1f(uid, slg.pl[i]->intencity);
				}

				uid = glGetUniformLocation(ShaderProgram, sambientstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(slg.pl[i]->ambient));
				}

				uid = glGetUniformLocation(ShaderProgram, sdiffusestr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(slg.pl[i]->diffuse));
				}

				uid = glGetUniformLocation(ShaderProgram, sspecularstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(slg.pl[i]->specular));
				}

				uid = glGetUniformLocation(ShaderProgram, scutoffstr);
				if (uid != -1) {
					glUniform1f(uid, slg.pl[i]->cutOff);
				}

				uid = glGetUniformLocation(ShaderProgram, soutercutoffstr);
				if (uid != -1) {
					glUniform1f(uid, slg.pl[i]->outerCutOff);
				}

				plus8(sposstr[12], srangestr[12], sintencitystr[12], sambientstr[12], sdiffusestr[12], sspecularstr[12],
					scutoffstr[12], soutercutoffstr[12]);
			}
		}

		uid = glGetUniformLocation(ShaderProgram, "ViewPos");
		if (uid != -1) {
			glUniform3f(uid, CamPos.x, CamPos.y, CamPos.z);
		}

		if (Ambient_Tex2d != 0) {
			uid = glGetUniformLocation(ShaderProgram, "material.ambient");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, Ambient_Tex2d);
			}
		}
		else {
			uid = glGetUniformLocation(ShaderProgram, "material.ambient");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, textureArr[0]);
			}
		}

		if (Diffuse_Tex2d != 0) {
			uid = glGetUniformLocation(ShaderProgram, "material.diffuse");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Diffuse_Tex2d);
			}
		}
		else {
			uid = glGetUniformLocation(ShaderProgram, "material.diffuse");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureArr[0]);
			}
		}

		if (Specular_Tex2d != 0) {
			uid = glGetUniformLocation(ShaderProgram, "material.specular");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, Specular_Tex2d);
			}
		}
		else {
			uid = glGetUniformLocation(ShaderProgram, "material.specular");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textureArr[0]);
			}
		}


		uid = glGetUniformLocation(ShaderProgram, "material.shininess");
		if (uid != -1) {
			glUniform1f(uid, 32.0f);
		}

		uid = glGetUniformLocation(ShaderProgram, "material.specularRate");
		if (uid != -1) {
			glUniform1f(uid, 1.0f);
		}

		uid = glGetUniformLocation(ShaderProgram, "material.ambientRate");
		if (uid != -1) {
			glUniform1f(uid, 1.0f);
		}

		uid = glGetUniformLocation(ShaderProgram, "transform");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(TransformMat));
		uid = glGetUniformLocation(ShaderProgram, "cameraTr");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(CameraTrasform));
		uid = glGetUniformLocation(ShaderProgram, "viewTr");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(ViewTransform));

		uid = glGetUniformLocation(ShaderProgram, "bWireframe");
		glUniform1i(uid, 0);

		if (shape_origin->isElement) {
			glBindVertexArray(shape_origin->VAO);
			glDrawElements(GLMod, shape_origin->indexCount * 3, GL_UNSIGNED_INT, 0);
			if (LineDebug) {
				glLineWidth(5);
				uid = glGetUniformLocation(ShaderProgram, "bWireframe");
				glUniform1i(uid, 1);
				glDrawElements(GL_LINE_LOOP, shape_origin->indexCount * 3, GL_UNSIGNED_INT, 0);
				glLineWidth(1);
			}
		}
		else {
			glBindVertexArray(shape_origin->VAO);
			glDrawArrays(GLMod, 0, shape_origin->vertexCount);
			//glDrawElements(GLMod, shape_origin->indexCount * 3, GL_UNSIGNED_INT, 0);
		}
	}

	void DrawUI() {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(ShaderProgram);
		GLMod = GL_TRIANGLES;
		GLShapeOrigin* shape_origin = &shapeArr[shapeorigin_ID];
		if (shape_origin->textureID != -1) {
			Diffuse_Tex2d = shape_origin->textureID;
			Ambient_Tex2d = shape_origin->textureID;
			Specular_Tex2d = shape_origin->textureID;
		}

		unsigned int uid = glGetUniformLocation(ShaderProgram, "ScreenSiz");
		if (uid != -1) {
			glUniform2f(uid, maxW, maxH);
		}

		uid = glGetUniformLocation(ShaderProgram, "Mov");
		if (uid != -1) {
			glUniform2f(uid, Translate.x, Translate.y);
		}

		uid = glGetUniformLocation(ShaderProgram, "Image");
		if (uid != -1) {
			glUniform1i(uid, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Diffuse_Tex2d);
		}

		glBindVertexArray(shape_origin->VAO);
		glDrawArrays(GLMod, 0, shape_origin->vertexCount);
		glDisable(GL_BLEND);
	}
};

class GLShapeInstance_WithParentAndPivot {
public:
	GLShapeInstance_WithParentAndPivot* Parent; // p0?
	//glm::vec3 pivot_in_parent; // p1'
	GLShapeInstance shape; // arr p2

	//glm::vec3 local_Rotation; // M1

	GLShapeInstance_WithParentAndPivot() {

	}

	virtual ~GLShapeInstance_WithParentAndPivot() {

	}

	void Init() {
		GLShapeInstance_WithParentAndPivot go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		Parent = nullptr;
		shape.Init();
		shape.shapeorigin_ID = 0;
		shape.Translate = glm::vec3(0, 0, 0);
		shape.Rotation = glm::vec3(0, 0, 0);
	}

	glm::mat4 GetTransformMat() {
		glm::mat4 TransformMat = shape.GetTransformMatrix();
		GLShapeInstance_WithParentAndPivot* pobj = Parent;
		while (pobj != nullptr) {
			TransformMat = pobj->shape.GetTransformMatrix() * TransformMat;
			pobj = pobj->Parent;
		}

		return TransformMat;
	}

	void Draw() {
		glm::mat4 TransformMat = shape.GetTransformMatrix();
		GLShapeInstance_WithParentAndPivot* pobj = Parent;
		while (pobj != nullptr) {
			TransformMat = pobj->shape.GetTransformMatrix() * TransformMat;
			pobj = pobj->Parent;
		}

		GLShapeOrigin* shape_origin = &AnimFragShapeArr[shape.shapeorigin_ID];
		if (shape_origin->textureID != -1) {
			shape.Diffuse_Tex2d = shape_origin->textureID;
			shape.Ambient_Tex2d = shape_origin->textureID;
			shape.Specular_Tex2d = shape_origin->textureID;
		}

		unsigned int uid = 0;
		glUseProgram(shape.ShaderProgram);

		int ltype = 0;

		uid = glGetUniformLocation(shape.ShaderProgram, "dirLight.direction");
		if (uid != -1) {
			glUniform3f(uid, parametic3xyz(lightSys.dirLight.direction));
		}

		uid = glGetUniformLocation(shape.ShaderProgram, "dirLight.ambient");
		if (uid != -1) {
			glUniform3f(uid, parametic3xyz(lightSys.dirLight.ambient));
		}

		uid = glGetUniformLocation(shape.ShaderProgram, "dirLight.diffuse");
		if (uid != -1) {
			glUniform3f(uid, parametic3xyz(lightSys.dirLight.diffuse));
		}

		uid = glGetUniformLocation(shape.ShaderProgram, "dirLight.specular");
		if (uid != -1) {
			glUniform3f(uid, parametic3xyz(lightSys.dirLight.specular));
		}

		shp::cube6f range = shp::cube6f(parametic3xyz(shape.Translate), parametic3xyz(shape.Translate));

		uid = glGetUniformLocation(shape.ShaderProgram, "PointLightNum");
		if (uid != -1) {
			PointLightGets plg = lightSys.GetExcutingPointLights(range);
			char posstr[] = "pointLights[0].position";
			char rangestr[] = "pointLights[0].range";
			char intencitystr[] = "pointLights[0].intencity";
			char ambientstr[] = "pointLights[0].ambient";
			char diffusestr[] = "pointLights[0].diffuse";
			char specularstr[] = "pointLights[0].specular";

			glUniform1i(uid, plg.maxnum);

			for (int i = 0; i < plg.maxnum; ++i) {
				uid = glGetUniformLocation(shape.ShaderProgram, posstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(plg.pl[i]->position));
				}

				uid = glGetUniformLocation(shape.ShaderProgram, rangestr);
				if (uid != -1) {
					glUniform1f(uid, plg.pl[i]->maxRange);
				}

				uid = glGetUniformLocation(shape.ShaderProgram, intencitystr);
				if (uid != -1) {
					glUniform1f(uid, plg.pl[i]->intencity);
				}

				uid = glGetUniformLocation(shape.ShaderProgram, ambientstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(plg.pl[i]->ambient));
				}

				uid = glGetUniformLocation(shape.ShaderProgram, diffusestr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(plg.pl[i]->diffuse));
				}

				uid = glGetUniformLocation(shape.ShaderProgram, specularstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(plg.pl[i]->specular));
				}

				plus6(posstr[12], rangestr[12], intencitystr[12], ambientstr[12], diffusestr[12], specularstr[12]);
			}
		}

		uid = glGetUniformLocation(shape.ShaderProgram, "SpotLightNum");
		if (uid != -1) {
			SpotLightGets slg = lightSys.GetExcutingSpotLights(range);
			char sposstr[] = "SpotLights[0].position";
			char srangestr[] = "SpotLights[0].range";
			char sintencitystr[] = "SpotLights[0].intencity";
			char sambientstr[] = "SpotLights[0].ambient";
			char sdiffusestr[] = "SpotLights[0].diffuse";
			char sspecularstr[] = "SpotLights[0].specular";
			char scutoffstr[] = "SpotLights[0].cutOff";
			char soutercutoffstr[] = "SpotLights[0].outerCutOff";

			glUniform1i(uid, slg.maxnum);

			for (int i = 0; i < slg.maxnum; ++i) {
				uid = glGetUniformLocation(shape.ShaderProgram, sposstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(slg.pl[i]->position));
				}

				uid = glGetUniformLocation(shape.ShaderProgram, srangestr);
				if (uid != -1) {
					glUniform1f(uid, slg.pl[i]->maxRange);
				}

				uid = glGetUniformLocation(shape.ShaderProgram, sintencitystr);
				if (uid != -1) {
					glUniform1f(uid, slg.pl[i]->intencity);
				}

				uid = glGetUniformLocation(shape.ShaderProgram, sambientstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(slg.pl[i]->ambient));
				}

				uid = glGetUniformLocation(shape.ShaderProgram, sdiffusestr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(slg.pl[i]->diffuse));
				}

				uid = glGetUniformLocation(shape.ShaderProgram, sspecularstr);
				if (uid != -1) {
					glUniform3f(uid, parametic3xyz(slg.pl[i]->specular));
				}

				uid = glGetUniformLocation(shape.ShaderProgram, scutoffstr);
				if (uid != -1) {
					glUniform1f(uid, slg.pl[i]->cutOff);
				}

				uid = glGetUniformLocation(shape.ShaderProgram, soutercutoffstr);
				if (uid != -1) {
					glUniform1f(uid, slg.pl[i]->outerCutOff);
				}

				plus8(sposstr[12], srangestr[12], sintencitystr[12], sambientstr[12], sdiffusestr[12], sspecularstr[12],
					scutoffstr[12], soutercutoffstr[12]);
			}
		}

		uid = glGetUniformLocation(shape.ShaderProgram, "ViewPos");
		if (uid != -1) {
			glUniform3f(uid, CamPos.x, CamPos.y, CamPos.z);
		}

		if (shape.Ambient_Tex2d != 0) {
			uid = glGetUniformLocation(shape.ShaderProgram, "material.ambient");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, shape.Ambient_Tex2d);
			}
		}
		else {
			uid = glGetUniformLocation(shape.ShaderProgram, "material.ambient");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, textureArr[0]);
			}
		}

		if (shape.Diffuse_Tex2d != 0) {
			uid = glGetUniformLocation(shape.ShaderProgram, "material.diffuse");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, shape.Diffuse_Tex2d);
			}
		}
		else {
			uid = glGetUniformLocation(shape.ShaderProgram, "material.diffuse");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureArr[0]);
			}
		}

		if (shape.Specular_Tex2d != 0) {
			uid = glGetUniformLocation(shape.ShaderProgram, "material.specular");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, shape.Specular_Tex2d);
			}
		}
		else {
			uid = glGetUniformLocation(shape.ShaderProgram, "material.specular");
			if (uid != -1) {
				glUniform1i(uid, 0);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textureArr[0]);
			}
		}


		uid = glGetUniformLocation(shape.ShaderProgram, "material.shininess");
		if (uid != -1) {
			glUniform1f(uid, 32.0f);
		}

		uid = glGetUniformLocation(shape.ShaderProgram, "material.specularRate");
		if (uid != -1) {
			glUniform1f(uid, 1.0f);
		}

		uid = glGetUniformLocation(shape.ShaderProgram, "material.ambientRate");
		if (uid != -1) {
			glUniform1f(uid, 1.0f);
		}

		uid = glGetUniformLocation(shape.ShaderProgram, "transform");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(TransformMat));
		uid = glGetUniformLocation(shape.ShaderProgram, "cameraTr");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(CameraTrasform));
		uid = glGetUniformLocation(shape.ShaderProgram, "viewTr");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(ViewTransform));

		uid = glGetUniformLocation(shape.ShaderProgram, "bWireframe");
		glUniform1i(uid, 0);

		if (shape_origin->isElement) {
			glBindVertexArray(shape_origin->VAO);
			glDrawElements(shape.GLMod, shape_origin->indexCount * 3, GL_UNSIGNED_INT, 0);
			if (LineDebug) {
				glLineWidth(5);
				uid = glGetUniformLocation(shape.ShaderProgram, "bWireframe");
				glUniform1i(uid, 1);
				glDrawElements(GL_LINE_LOOP, shape_origin->indexCount * 3, GL_UNSIGNED_INT, 0);
				glLineWidth(1);
			}
		}
		else {
			glBindVertexArray(shape_origin->VAO);
			glDrawArrays(shape.GLMod, 0, shape_origin->vertexCount);
			//glDrawElements(GLMod, shape_origin->indexCount * 3, GL_UNSIGNED_INT, 0);
		}
	}
};

typedef struct AnimKey {
	float time = 0;
	glm::vec3 translate;
	glm::vec3 rotation;
	glm::vec3 scale;
};

FM_Model1* GameObjFM;

class Animation {
public:
	bool enable = false;
	size_t ObjSizs = 0;
	InfiniteArray<AnimKey>* animateArr = nullptr;
	glm::vec2 flow = glm::vec2(0, 1);

	Animation() :
		ObjSizs(0),
		animateArr(nullptr),
		flow(glm::vec2(0, 1))
	{

	}

	virtual ~Animation() {

	}

	void Init(size_t siz, FM_Model* fm) {
		Animation go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		ObjSizs = siz;
		animateArr = (InfiniteArray<AnimKey>*)GameObjFM->_New(sizeof(InfiniteArray<AnimKey>) * siz);

		InfiniteArray<AnimKey> kk;
		vp = *(int*)&kk;
		for (int i = 0; i < siz; ++i) {
			*((int*)&animateArr[i]) = vp;
			animateArr[i].NULLState();
			animateArr[i].SetFM(fm);
			animateArr[i].Init(0);
		}

		flow = glm::vec2(0, 1);
		enable = true;
	}

	float Flow(float f) {
		return f;
	}

	void SortWithTime() {
		for (int i = 0; i < ObjSizs; ++i) {
			for (int a = 0; a < animateArr[i].size(); ++a) {
				if (animateArr[i][a].time < 0) animateArr[i][a].time = 10000;
				for (int b = a + 1; b < animateArr[i].size(); ++b) {
					if (animateArr[i][b].time < 0) animateArr[i][b].time = 10000;

					if (animateArr[i][a].time > animateArr[i][b].time) {
						AnimKey ak;
						ak = animateArr[i][b];
						animateArr[i][b] = animateArr[i][a];
						animateArr[i][a] = ak;
					}
				}
			}
		}
	}

	AnimKey* GetAnim(float time) {
		SortWithTime();
		//InstanceFreeMem->ClearAll(); //스레드마다 다르게 할당하고, 리턴된 값을 복사하는 것이 원칙.

		AnimKey* rarr = (AnimKey*)InstanceFreeMem->_New(sizeof(AnimKey) * ObjSizs);
		if (rarr == nullptr) {
			InstanceFreeMem->ClearAll();
			rarr = (AnimKey*)InstanceFreeMem->_New(sizeof(AnimKey) * ObjSizs);
		}
		memset(rarr, 0, sizeof(AnimKey) * ObjSizs);
		for (int i = 0; i < ObjSizs; ++i) {
			InfiniteArray<AnimKey>& keys = animateArr[i];
			int max = keys.size();
			int n = max / 2;
			int delta = max / 4;

			if (max == 1) {
				rarr[i].rotation = keys[0].rotation;
				rarr[i].scale = keys[0].scale;
				rarr[i].translate = keys[0].translate;
			}

			//while (true) {
			//	if (delta <= 3) {
			//		for (int a = n - delta; a < n + delta+1; ++a) {
			//			if (keys[a].time <= time && time <= keys[a + 1].time) {
			//				float ft = Flow((time - keys[a].time) / (keys[a+1].time - keys[a].time));
			//				rarr[i].rotation = ft * (keys[a + 1].rotation - keys[a].rotation) + keys[a].rotation;
			//				rarr[i].scale = ft * (keys[a + 1].scale - keys[a].scale) + keys[a].scale;
			//				rarr[i].translate = ft * (keys[a + 1].translate - keys[a].translate) + keys[a].translate;
			//				break;
			//			}
			//		}
			//		break;
			//	}
			//	if (keys[n].time < time) {
			//		n += delta;
			//		delta /= 2;
			//		continue;
			//	}
			//	if (keys[n].time > time) {
			//		n -= delta;
			//		delta /= 2;
			//		continue;
			//	}
			//}

			for (int a = 0; a < keys.size() - 1; ++a) {
				int next = a + 1;
				if (next == keys.size() - 1) {
					next = 0;
				}
				if (keys[a].time <= time && time <= keys[a + 1].time) {
					float ft = Flow((time - keys[a].time) / (keys[a + 1].time - keys[a].time));
					rarr[i].rotation = ft * (keys[next].rotation - keys[a].rotation) + keys[a].rotation;
					rarr[i].scale = ft * (keys[next].scale - keys[a].scale) + keys[a].scale;
					rarr[i].translate = ft * (keys[next].translate - keys[a].translate) + keys[a].translate;
					break;
				}
			}

			if (time > keys[keys.up - 1].time) {
				rarr[i].rotation = keys[keys.up - 1].rotation;
				rarr[i].scale = keys[keys.up - 1].scale;
				rarr[i].translate = keys[keys.up - 1].translate;
			}
		}

		return rarr;
	}

	void SaveAnimation(const char* filename) {
		ofstream out;
		out.open(filename);
		out << ObjSizs << ' ';
		for (int i = 0; i < ObjSizs; ++i) {
			out << animateArr[i].size() << '\n';
			for (int k = 0; k < animateArr[i].size(); ++k) {
				out << animateArr[i][k].time << '\n';
				out << animateArr[i][k].translate.x << ' ' << animateArr[i][k].translate.y << ' ' << animateArr[i][k].translate.z << '\n';
				out << animateArr[i][k].rotation.x << ' ' << animateArr[i][k].rotation.y << ' ' << animateArr[i][k].rotation.z << '\n';
				out << animateArr[i][k].scale.x << ' ' << animateArr[i][k].scale.y << ' ' << animateArr[i][k].scale.z << '\n';
			}
		}
		out.close();
	}

	void LoadAnimation(const char* filename) {
		enable = true;
		ifstream in;
		in.open(filename);
		in >> ObjSizs;

		animateArr = (InfiniteArray<AnimKey>*)GameObjFM->_New(sizeof(InfiniteArray<AnimKey>) * ObjSizs);

		for (int i = 0; i < ObjSizs; ++i) {
			size_t siz = 0;
			in >> siz;
			animateArr[i].NULLState();
			animateArr[i].SetFM((FM_Model*)GameObjFM);
			animateArr[i].Init(siz);
			animateArr[i].up = siz;
			for (int k = 0; k < siz; ++k) {
				in >> animateArr[i][k].time;
				in >> animateArr[i][k].translate.x;
				in >> animateArr[i][k].translate.y;
				in >> animateArr[i][k].translate.z;
				in >> animateArr[i][k].rotation.x;
				in >> animateArr[i][k].rotation.y;
				in >> animateArr[i][k].rotation.z;
				in >> animateArr[i][k].scale.x;
				in >> animateArr[i][k].scale.y;
				in >> animateArr[i][k].scale.z;
			}
		}

		in.close();
	}
};

GLuint shaderID;
GLuint LightSourceShaderID;
GLuint textShaderID;
GLuint ScreenShaderID;

class GLRigidBody {
public:
	GLShapeInstance_WithParentAndPivot* objArr = nullptr; // 움직일 오브젝트들
	AnimKey* defaultKey = nullptr; // 기본적인 모습
	size_t ObjSiz = 0;
	size_t objup = 0;

	InfiniteArray<Animation> AnimArr;
	Animation* present_animation; // 현재 애니메이션

	GLRigidBody() :
		objArr(nullptr), defaultKey(nullptr),
		ObjSiz(0), objup(0),
		AnimArr(InfiniteArray<Animation>()),
		present_animation(nullptr)
	{

	}

	virtual ~GLRigidBody() {

	}

	void Init() {
		GLRigidBody go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		objArr = nullptr;
		ObjSiz = 0;
		objup = 0;

		defaultKey = nullptr;

		present_animation = nullptr;
	}

	void SetDefalutKeys() {
		if (defaultKey == nullptr) {
			defaultKey = (AnimKey*)GameObjFM->_New(sizeof(AnimKey) * ObjSiz);
		}

		for (int i = 0; i < ObjSiz; ++i) {
			defaultKey[i].rotation = objArr[i].shape.Rotation;
			defaultKey[i].translate = objArr[i].shape.Translate;
			defaultKey[i].scale = objArr[i].shape.Scale;
			defaultKey[i].time = 0;
		}
	}

	void Draw() {
		for (int i = 0; i < objup; ++i) {
			objArr[i].Draw();
		}
	}

	void SaveBodyAndDefalutKey(const char* filename) {
		ofstream out;
		out.open(filename);

		out << objup << '\n';
		for (int i = 0; i < objup; ++i) {
			out << objArr[i].shape.shapeorigin_ID << ' ';
			int parentID = -1;
			for (int k = 0; k < objup; ++k) {
				if (&objArr[k] == objArr[i].Parent) {
					parentID = k;
					break;
				}
			}
			out << parentID << '\n';
			//부모의 데이터도 저장
		}
		for (int i = 0; i < objup; ++i) {
			out << defaultKey[i].translate.x << ' ' << defaultKey[i].translate.y << ' ' << defaultKey[i].translate.z << '\n';
			out << defaultKey[i].rotation.x << ' ' << defaultKey[i].rotation.y << ' ' << defaultKey[i].rotation.z << '\n';
			out << defaultKey[i].scale.x << ' ' << defaultKey[i].scale.y << ' ' << defaultKey[i].scale.z << '\n';
		}

		out.close();
	}

	void LoadBodyAndDefalutKey(const char* filename) {
		ifstream in;
		in.open(filename);
		in >> objup;
		//objArr = 
		int parentIDs[20] = {};
		for (int i = 0; i < 20; ++i) {
			parentIDs[i] = -1;
		}
		for (int i = 0; i < objup; ++i) {
			objArr[i].Init();
			objArr[i].shape.SetShader(shaderID);
			in >> objArr[i].shape.shapeorigin_ID;
			in >> parentIDs[i];
		}

		for (int i = 0; i < objup; ++i) {
			if (parentIDs[i] < 0) {
				objArr[i].Parent = nullptr;
			}
			else {
				objArr[i].Parent = &objArr[parentIDs[i]];
			}
		}

		for (int i = 0; i < objup; ++i) {
			in >> objArr[i].shape.Translate.x;
			in >> objArr[i].shape.Translate.y;
			in >> objArr[i].shape.Translate.z;
			in >> objArr[i].shape.Rotation.x;
			in >> objArr[i].shape.Rotation.y;
			in >> objArr[i].shape.Rotation.z;
			in >> objArr[i].shape.Scale.x;
			in >> objArr[i].shape.Scale.y;
			in >> objArr[i].shape.Scale.z;
		}

		SetDefalutKeys();

		in.close();
	}
};

class GameObject {
public:
	GLShapeInstance shape;
	shp::cube6f border;
	bool enable = true;

	//다른 오브젝트와 충돌체크를 할 것인지
	bool isCollide = true;
	char layer = 'o';
	shp::vec3f velocity = shp::vec3f(0, 0, 0);
	bool touched = false; // 땅에 닿았는지 체크함.

	GameObject() {
	}
	GameObject(const GameObject& ref) {
		shape = ref.shape;
		border = ref.border;
		enable = ref.enable;
		isCollide = ref.isCollide;
		layer = ref.layer;
		velocity = ref.velocity;
		touched = ref.touched;
	}
	virtual ~GameObject() {}

	GameObject* Init() {
		enable = true;
		shape.Init();
		shape.GLMod = GL_TRIANGLES;
		shape.Ambient_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Diffuse_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Specular_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Rotation = glm::vec3(0, 0, 0);
		shape.Translate = glm::vec3(0, 0, 0);
		shape.Scale = glm::vec3(1, 1, 1);

		touched = false;

		glm::vec3 b = shapeArr[shape.shapeorigin_ID].BorderSize;
		border = shp::cube6f(-b.x, -b.y, -b.z, b.x, b.y, b.z);

		velocity = shp::vec3f(0, 0, 0);
		isCollide = true;
		layer = 'o';

		//가상함수 테이블 설정
		GameObject go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		return this;
	}

	virtual void Update(float delta) {
		if (enable) {
		}
	}

	virtual void Render() {
		if (enable) {
			shape.Draw();
		}
	}

	virtual void Event(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		return;
	}

	virtual size_t GetSiz() {
		return sizeof(GameObject);
	}

	void SetBorder() {
		glm::vec3 b = shapeArr[shape.shapeorigin_ID].BorderSize;
		border = shp::cube6f(-b.x, -b.y, -b.z, b.x, b.y, b.z);
	}

	shp::cube6f GetBorder(bool afterMove, float delta = 0) {
		shp::cube6f b = border;
		b.fx *= shape.Scale.x;
		b.lx *= shape.Scale.x;

		b.fy *= shape.Scale.y;
		b.ly *= shape.Scale.y;

		b.fz *= shape.Scale.z;
		b.lz *= shape.Scale.z;

		if (afterMove) {
			b.setCenter(shp::vec3f(parametic3xyz(shape.Translate)) + velocity);
		}
		else {
			b.setCenter(shp::vec3f(parametic3xyz(shape.Translate)));
		}

		return b;
	}
};

typedef struct NPCScript {
	TCHAR talker_name[20];
	TCHAR script[128];
};

constexpr NPCScript npc_script[128] =
{
	{L"개발자", L"마우스 오른쪽 클릭시 말걸기/대화진행이 가능합니다!"}, //0
	{L"친구", L"날도 어두워지고, 좀 무서우니까 그만 놀고 집으로 가자."}, //1
	{L"주인공", L"그래.. 좀 무섭다. 근데 우리 어디서 왔더라? 기억이 안나."}, //2
	{L"친구", L"무섭지만 정신 똑바로 차리라구."}, //3
	{L"친구", L"이동은 WASD로, 점프는 SPACE로, 공격은 클릭으로 할 수 있어. 알고있지?"}, //4
	{L"주인공", L"그 정도는 알고 있어."}, //5
	{L"친구", L"그럼 어서 여기를 나갈 방법을 생각해보자."}, // 6
	{L"", L""}, //7
	{L"친구", L"오.. 열쇠를 찾으면 길을 열 수 있구나."}, //8
	{L"친구", L"열쇠를 찾아서 게속 가보자."}, //9
	{L"친구", L"우리 돌아갈 수 있을까? 갑자기 나타나는 뱀도 무섭고 말이야."}, //10
	{L"친구", L"뱀한테 물려서 상처를 입으면.. 돌아가지 못할지도 몰라.."}, //11
	{L"주인공", L"헉"}, //12
	{L"친구", L"포션이라도 있으면 Q버튼을 눌러 상처를 치료할 수 있을 텐데.."}, //13
	{L"", L""}, //14
	{L"친구", L"음.. 두 갈래길인가? 열쇠는 하나인데 문은 두개야.."}, //15
	{L"친구", L"어디로 가야 되지?"}, //16
	{L"주인공", L"둘 중 하나를 선택해야 될 것 같아.. 고민되네.."}, //17
	{L"", L""}, //18
	{L"친구", L"으으.. 산속에는 뱀들이 참 많은 것 같아."}, //19
	{L"친구", L"빨리 집으로 가고 싶어."}, //20
	{L"주인공", L"그런데 어디로 가야하지?"}, //21
	{L"친구", L"글쎄.. 게속 돌아다니다 보면 알게되지 않을까?"}, //22
	{L"주인공", L"일단 열쇠를 찾자."}, //23
	{L"", L""}, //24
	{L"친구", L"와 도착했다! 집이야!"}, //25
	{L"개발자", L"게임은 여기까지입니다."}, //26
	{L"개발자", L"구현된 부분이 별로 없고, 한참 부족한 게임이지만"}, //27
	{L"개발자", L"플레이 해주셔서 정말 감사합니다."}, //28
	{L"", L""}, //29
};

int UIStart = 0;
int ItemStart = 0;
int MonsterStart = 0;
int MAX_itemID = 3;
class Player : GameObject {
public:
	float gravity = 10;
	float yadd = 0;

	bool Wkey, Akey, Skey, Dkey;
	bool RBtn = false;
	float MoveSpeed = 0.1f;

	GLRigidBody Body;

	float animateTime = 0;
	float maxframes[5] = { 1, 4, 1, 4, 6 };
	int state = 0;
	float seeingAngle = 0;
	float seeingHeight = 2;
	glm::vec2 attackflow = glm::vec2(0, 1);

	int HP = 5;
	int MAXHP = 5;
	GLShapeInstance HPIcons;
	int keyNum = 0;
	GLShapeInstance KeyIcons;
	int PotionNum = 0;
	GLShapeInstance PotionIcons;

	TCHAR DialogText[256] = {}; // 말하는 내용
	TCHAR TalkerText[64] = {}; // 말하는 사람을 나타내는 텍스트
	bool DialogVisible = false;
	GLShapeInstance DialogShape;
	int dialogOffset = 0;
	glm::vec2 dialogNextFlow = glm::vec2(0, 0.3f);
	Ptr* gm;

	Player()
	{
		Wkey = false;
		Akey = false;
		Skey = false;
		Dkey = false;
		MoveSpeed = 1;
		gravity = 10;
		yadd = 0;
	}

	virtual ~Player() {

	}

	void IsDamaged();

	void GoNextStage();

	void AddHitBox(int damage, shp::cube6f range);

	Player* Init(Ptr* GM) {
		//가상함수 테이블 설정
		attackflow = glm::vec2(0, 1);
		gm = GM;
		
		shape.Init();
		shape.GLMod = GL_TRIANGLES;
		shape.Ambient_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Diffuse_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Specular_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Rotation = glm::vec3(0, 0, 0);
		shape.Translate = glm::vec3(0, 0, 0);
		shape.Scale = glm::vec3(1, 1, 1);

		glm::vec3 b = shapeArr[shape.shapeorigin_ID].BorderSize;
		border = shp::cube6f(-b.x, -b.y, -b.z, b.x, b.y, b.z);

		Wkey = false;
		Akey = false;
		Skey = false;
		Dkey = false;
		MoveSpeed = 2;
		gravity = 10;
		yadd = 0;

		touched = false;

		isCollide = true;
		layer = 'p';
		velocity = shp::vec3f(0, 0, 0);

		Player go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		maxframes[0] = 1;
		maxframes[1] = 4;
		maxframes[2] = 1;
		maxframes[3] = 4;
		maxframes[4] = 6;
		state = 1;
		animateTime = 0;
		seeingAngle = 0;
		seeingHeight = 2.5f;

		Body.Init();
		Body.ObjSiz = 20;
		Body.objArr = (GLShapeInstance_WithParentAndPivot*)GameObjFM->_New(sizeof(GLShapeInstance_WithParentAndPivot) * Body.ObjSiz);
		Body.LoadBodyAndDefalutKey("Animations\\Player_Rigidbody.txt");
		Body.AnimArr.NULLState();
		Body.AnimArr.SetFM((FM_Model*)GameObjFM);
		Body.AnimArr.Init(5);
		Body.AnimArr[0].LoadAnimation("Animations\\Player_Idle.txt");
		Body.AnimArr[1].LoadAnimation("Animations\\Player_Walk.txt");
		Body.AnimArr[2].LoadAnimation("Animations\\Player_Jump.txt");
		Body.AnimArr[3].LoadAnimation("Animations\\Player_SwingAttack.txt");
		Body.AnimArr[4].LoadAnimation("Animations\\Player_Hello.txt");
		Body.present_animation = &Body.AnimArr[0];

		HP = 5;
		MAXHP = 5;
		HPIcons.ShaderProgram = ScreenShaderID;
		HPIcons.Diffuse_Tex2d = textureArr[2];
		HPIcons.shapeorigin_ID = UIStart;

		keyNum = 0;
		KeyIcons.ShaderProgram = ScreenShaderID;
		KeyIcons.Diffuse_Tex2d = textureArr[3];
		KeyIcons.shapeorigin_ID = UIStart;

		PotionNum = 0;
		PotionIcons.ShaderProgram = ScreenShaderID;
		PotionIcons.Diffuse_Tex2d = textureArr[6];
		PotionIcons.shapeorigin_ID = UIStart;

		DialogVisible = false;
		DialogShape.ShaderProgram = ScreenShaderID;
		DialogShape.shapeorigin_ID = UIStart + 1;
		DialogText[0] = 0;
		TalkerText[0] = 0;
		dialogNextFlow = glm::vec2(0, 0.3f);

		enable = true;
		return this;
	}

	virtual void Update(float delta) {
		if (enable) {
			dialogNextFlow.x += delta;

			//중력과 점프
			if (HP <= 0) {
				enable = false;
			}

			constexpr float AnimationAccel = 5;
			animateTime += AnimationAccel * delta;

			if (state == 3 && animateTime >= maxframes[state] - 1) {
				if (attackflow.x > attackflow.y) {
					float f = 3;
					shp::cube6f cu = shp::cube6f(-f, -f, -f, f, f, f);
					cu.setCenter(shp::vec3f(parametic3xyz(shape.Translate)));
					AddHitBox(1, cu);
					attackflow.x = 0;
					Music::PlayOnce(3);
				}
			}

			if (animateTime >= maxframes[state]) {
				if (state == 3 || state == 4) {
					state = 0;
				}
				animateTime -= (float)maxframes[state];
			}

			if (Body.AnimArr[state].enable) {
				AnimKey* akarr = Body.AnimArr[state].GetAnim(animateTime);
				for (int i = 0; i < Body.objup; ++i) {
					Body.objArr[i].shape.Translate = akarr[i].translate;
					Body.objArr[i].shape.Rotation = akarr[i].rotation;
					Body.objArr[i].shape.Scale = akarr[i].scale;
				}
			}

			Body.objArr[0].shape.SetScale(glm::vec3(0.01f, 0.01f, 0.01f));
			Body.objArr[0].shape.SetTranslate(shape.Translate + glm::vec3(0, 0.2f, 0));
			Body.objArr[0].shape.SetRotation(shape.Rotation + glm::vec3(0, -90 - 180 * seeingAngle / shp::PI, 0));

			if (touched) {
				yadd = 0;
			}

			IsDamaged();
			attackflow.x += delta;

			yadd += gravity * delta;
			velocity.y -= yadd * delta;

			if (DialogVisible == false) {
				//이동
				float xd = cosf(seeingAngle);
				float zd = sinf(seeingAngle);
				if (Wkey) {
					state = 1;
					velocity.z -= zd * MoveSpeed * delta;
					velocity.x -= xd * MoveSpeed * delta;
				}
				if (Akey) {
					state = 1;
					velocity.z += xd * MoveSpeed * delta;
					velocity.x -= zd * MoveSpeed * delta;
				}
				if (Skey) {
					state = 1;
					velocity.z += zd * MoveSpeed * delta;
					velocity.x += xd * MoveSpeed * delta;
				}
				if (Dkey) {
					state = 1;
					velocity.z -= xd * MoveSpeed * delta;
					velocity.x += zd * MoveSpeed * delta;
				}
			}


			if (fabsf(velocity.x) + fabsf(velocity.z) == 0 && (state != 3 && state != 4)) {
				state = 0;
			}

			if (touched == false) {
				state = 2;
			}

			GoNextStage();
		}
	}

	virtual void Render() {
		if (enable) {
			Body.Draw();

			if (DialogVisible == false) {
				for (int i = 0; i < HP; ++i) {
					HPIcons.Translate = glm::vec3(100 + 100 * i, maxH - 100, 1);
					HPIcons.DrawUI();
				}

				TCHAR numstr[128] = {};
				PotionIcons.Translate = glm::vec3(maxW - 100, maxH - 100, 1);
				PotionIcons.DrawUI();

				wsprintf(numstr, L"%d", PotionNum);
				DrawString(hdc, "Resources\\Font\\MalangmalangB.ttf", 40, numstr, shp::rect4f(maxW - 50, maxH - 100, maxW, maxH), Color4f(1, 1, 1, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);

				KeyIcons.Translate = glm::vec3(maxW - 300, maxH - 100, 1);
				KeyIcons.DrawUI();

				wsprintf(numstr, L"%d", keyNum);
				DrawString(hdc, "Resources\\Font\\MalangmalangB.ttf", 40, numstr, shp::rect4f(maxW - 250, maxH - 100, maxW, maxH), Color4f(1, 1, 1, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);
			}
			else {
				DialogShape.Translate = glm::vec3(maxW / 2, 5 * maxH / 6, 1);
				DialogShape.DrawUI();
				DrawString(hdc, "Resources\\Font\\MalangmalangB.ttf", 30, TalkerText, shp::rect4f(100, 8 * maxH / 12, maxW - 100, maxH), Color4f(1, 1, 1, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);
				DrawString(hdc, "Resources\\Font\\MalangmalangB.ttf", 20, DialogText, shp::rect4f(100, 9 * maxH / 12, maxW - 100, maxH), Color4f(1, 1, 1, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);
				//DrawString(hdc, "Resources\\Font\\MalangmalangB.ttf", 30, L"튜토리얼 맨", shp::rect4f(100, 8 * maxH / 12, maxW - 100, maxH), Color4f(1, 1, 1, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);
				//DrawString(hdc, "Resources\\Font\\MalangmalangB.ttf", 20, L"안녕! 반가워 튜토리얼을 시작하자!", shp::rect4f(100, 9 * maxH / 12, maxW-100, maxH), Color4f(1, 1, 1, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);
			}
		}
	}

	virtual void Event(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (message == WM_KEYDOWN) {
			if (wParam == 'W') {
				Wkey = true;
			}
			if (wParam == 'A') {
				Akey = true;
			}
			if (wParam == 'S') {
				Skey = true;
			}
			if (wParam == 'D') {
				Dkey = true;
			}
			if (wParam == VK_SPACE) {
				if (touched && DialogVisible == false) {
					touched = false;
					yadd = -5;
				}
			}
			if (wParam == 'Q') {
				//포션 사용
				if (PotionNum > 0) {
					PotionNum -= 1;
					if (HP + 2 <= MAXHP) {
						HP += 2;
					}
					else {
						HP = MAXHP;
					}
				}

			}
		}
		if (message == WM_KEYUP) {
			if (wParam == 'W') {
				Wkey = false;
			}
			if (wParam == 'A') {
				Akey = false;
			}
			if (wParam == 'S') {
				Skey = false;
			}
			if (wParam == 'D') {
				Dkey = false;
			}
		}

		if (message == WM_LBUTTONDOWN) {
			animateTime = 0;
			state = 3;
		}
		if (message == WM_RBUTTONDOWN) {
			RBtn = true;

			if (DialogVisible == false) {
				animateTime = 0;
				state = 4;
			}

			if (DialogVisible == true && dialogNextFlow.x > dialogNextFlow.y) {
				dialogOffset += 1;
				dialogNextFlow.x = 0;
				if (lstrcmp(npc_script[dialogOffset].talker_name, L"") == 0) {
					DialogVisible = false;
				}
				else {
					PlayerDialogControlFunc(npc_script[dialogOffset].talker_name, npc_script[dialogOffset].script, true);
				}
			}
		}
		if (message == WM_RBUTTONUP) {
			RBtn = false;
		}
		return;
	}

	virtual size_t GetSiz() {
		return sizeof(Player);
	}

	void CameraWork() {
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		float dx = cursorPos.x - maxW / 2;
		float dy = cursorPos.y - maxH / 2;
		dx /= 100;
		dy /= 100;
		if (true) {
			seeingAngle += dx;
			if (inside(0.1f, seeingHeight + dy, 3.0f)) {
				seeingHeight += dy;
			}
		}
		SetCursorPos(maxW / 2, maxH / 2);
		glm::vec3 pos = shape.Translate + glm::vec3(2 * cos(seeingAngle), seeingHeight, 2 * sin(seeingAngle));
		SetCameraTransform(pos, pos + glm::vec3(-3 * cos(seeingAngle), -(seeingHeight), -3 * sin(seeingAngle)));
	}

	void PlayerDialogControlFunc(const TCHAR* TalkerName, const TCHAR* DiaText, bool isEnable) {
		wcscpy_s(TalkerText, TalkerName);
		wcscpy_s(DialogText, DiaText);
		DialogVisible = isEnable;
	}

	void AddItem(int itemid) {
		switch (itemid) {
		case 0:
			keyNum += 1;
			break;
		case 1:
			PotionNum += 1;
			break;
		case 3:
			if (HP + 1 <= MAXHP) {
				HP += 1;
			}
			break;
		}
	}
};

class Monster : GameObject {
public:
	int MonID = 0;

	float gravity = 10;
	float yadd = 0;
	float MoveSpeed = 0.1f;
	GLRigidBody Body;

	float animateTime = 0;
	float maxframes[5] = { 1, 4, 1, 4, 0 };
	int state = 0;
	float seeingAngle = 0;

	int HP = 3;
	int MAXHP = 3;

	float seekRange = 10;
	float chaseRange = 5;
	float attackRange = 1;
	glm::vec2 attackflow = glm::vec2(0, 1);

	Player* Target = nullptr;
	Ptr* gm;

	Monster() {}
	virtual ~Monster() {}

	void IsDamaged();

	void AddHitBox(int damage, shp::cube6f range);

	Monster* Init(int monsterid, Player* target, Ptr* GM) {
		gm = GM;
		Monster go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		Target = target;

		enable = true;
		shape.Init();
		shape.GLMod = GL_TRIANGLES;
		shape.Ambient_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Diffuse_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Specular_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Rotation = glm::vec3(0, 0, 0);
		shape.Translate = glm::vec3(0, 0, 0);
		shape.Scale = glm::vec3(1, 1, 1);

		attackflow = glm::vec2(0, 1);

		glm::vec3 b = shapeArr[shape.shapeorigin_ID].BorderSize;
		border = shp::cube6f(-b.x / 2, -b.y / 2, -b.z / 2, b.x / 2, b.y / 2, b.z / 2);

		MoveSpeed = 1;
		gravity = 10;
		yadd = 0;

		touched = false;

		isCollide = true;
		layer = 'p';
		velocity = shp::vec3f(0, 0, 0);

		animateTime = 0;
		seeingAngle = 0;

		MonID = monsterid;
		switch (MonID) {
		case 0:
			seekRange = 13;
			chaseRange = 8;
			attackRange = 1;

			maxframes[0] = 1;
			maxframes[1] = 4;
			maxframes[2] = 1;
			maxframes[3] = 4;
			maxframes[4] = 6;
			state = 1;

			Body.Init();
			Body.ObjSiz = 20;
			Body.objArr = (GLShapeInstance_WithParentAndPivot*)GameObjFM->_New(sizeof(GLShapeInstance_WithParentAndPivot) * Body.ObjSiz);
			Body.LoadBodyAndDefalutKey("Animations\\Snake_Rigidbody.txt");
			Body.AnimArr.NULLState();
			Body.AnimArr.SetFM((FM_Model*)GameObjFM);
			Body.AnimArr.Init(4);
			Body.AnimArr[0].LoadAnimation("Animations\\Snake_Idle.txt");
			Body.AnimArr[1].LoadAnimation("Animations\\Snake_Walk.txt");
			Body.AnimArr[2].LoadAnimation("Animations\\Snake_Idle.txt");
			Body.AnimArr[3].LoadAnimation("Animations\\Snake_Attack.txt");
			Body.present_animation = &Body.AnimArr[0];
			break;
		}

		HP = 3;
		MAXHP = 3;

		return this;
	}

	virtual void Update(float delta) {
		if (enable) {

			if (HP <= 0) {
				
				enable = false;
				return;
			}

			velocity = shp::vec3f(0, 0, 0);

			constexpr float AnimationAccel = 5;
			animateTime += AnimationAccel * delta;
			if (animateTime >= maxframes[state]) {
				animateTime = 0;
				//animateTime -= (float)maxframes[state];
			}

			if (Body.AnimArr[state].enable) {
				AnimKey* akarr = Body.AnimArr[state].GetAnim(animateTime);
				for (int i = 0; i < Body.objup; ++i) {
					Body.objArr[i].shape.Translate = akarr[i].translate;
					Body.objArr[i].shape.Rotation = akarr[i].rotation;
					Body.objArr[i].shape.Scale = akarr[i].scale;
				}
			}

			Body.objArr[0].shape.SetScale(glm::vec3(0.016f, 0.016f, 0.016f));
			Body.objArr[0].shape.SetTranslate(shape.Translate + glm::vec3(0, -0.4f, 0));
			Body.objArr[0].shape.SetRotation(shape.Rotation + glm::vec3(0, 90 - 180 * seeingAngle / shp::PI, 0));

			shp::vec3f targetpos = shp::vec3f(parametic3xyz(((GameObject*)Target)->shape.Translate));
			shp::cube6f targetborder = ((GameObject*)Target)->GetBorder(false, 0);
			shp::vec3f thispos = shp::vec3f(parametic3xyz(shape.Translate));
			shp::vec3f dir = targetpos - thispos;
			dir = dir / shp::get_distance3D(shp::vec3f(0, 0, 0), dir);
			float len = get_distance3D(targetpos, thispos);

			if (touched) {
				yadd = 0;
			}

			IsDamaged();
			attackflow.x += delta;

			if (inside(chaseRange, len, seekRange)) {
				shp::angle2f angle = shp::angle2f(dir.x, dir.z);
				seeingAngle = angle.radian;
				state = 0;
			}
			else if (inside(attackRange, len, chaseRange)) {
				if (touched && dir.y > 0.15f) {
					touched = false;
					yadd = -5;
				}

				shp::angle2f angle = shp::angle2f(dir.x, dir.z);
				seeingAngle = angle.radian;

				velocity.x += dir.x * MoveSpeed * delta;
				velocity.z += dir.z * MoveSpeed * delta;
				state = 1;
			}
			else if (len <= attackRange) {
				shp::angle2f angle = shp::angle2f(dir.x, dir.z);
				seeingAngle = angle.radian;
				if (animateTime >= 3) {
					// 공격
					if (attackflow.x > attackflow.y) {
						float f = 0.5f;
						shp::cube6f cu = shp::cube6f(-f, -f, -f, f, f, f);
						cu.setCenter(shp::vec3f(parametic3xyz(shape.Translate)));
						AddHitBox(1, cu);
						attackflow.x = 0;
					}
				}
				state = 3;
			}



			yadd += gravity * delta;
			velocity.y -= yadd * delta;


			if (fabsf(velocity.x) + fabsf(velocity.z) == 0 && (state != 3 && state != 4)) {
				state = 0;
			}

			if (touched == false) {
				state = 2;
			}
		}
	}

	virtual void Render() {
		if (enable) {
			Body.Draw();
		}
	}

	virtual void Event(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		return;
	}

	virtual size_t GetSiz() {
		return sizeof(Monster);
	}


};

class NPC : GameObject {
public:
	int talkstrId = 0;

	float gravity = 10;
	float yadd = 0;
	float MoveSpeed = 0.1f;
	GLRigidBody Body;

	float animateTime = 0;
	float maxframes[5] = { 1, 4, 1, 4, 0 };
	int state = 0;
	float seeingAngle = 0;

	int HP = 3;
	int MAXHP = 3;

	float TalkRange = 3;

	Player* Target = nullptr;
	Ptr* gm;

	NPC() {}
	virtual ~NPC() {}

	NPC* Init(int talkid, Player* target, Ptr* GM) {
		
		gm = GM;
		NPC go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		Target = target;

		shape.Init();
		shape.GLMod = GL_TRIANGLES;
		shape.Ambient_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Diffuse_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Specular_Tex2d = shapeArr[shape.shapeorigin_ID].textureID;
		shape.Rotation = glm::vec3(0, 0, 0);
		shape.Translate = glm::vec3(0, 0, 0);
		shape.Scale = glm::vec3(1, 1, 1);

		glm::vec3 b = shapeArr[shape.shapeorigin_ID].BorderSize;
		border = shp::cube6f(-b.x / 2, -b.y / 2, -b.z / 2, b.x / 2, b.y / 2, b.z / 2);

		gravity = 10;
		yadd = 0;

		touched = false;

		isCollide = true;
		layer = 'p';
		velocity = shp::vec3f(0, 0, 0);

		animateTime = 0;
		seeingAngle = 0;

		TalkRange = 3;
		maxframes[0] = 1;
		state = 0;

		Body.Init();
		Body.ObjSiz = 20;
		Body.objArr = (GLShapeInstance_WithParentAndPivot*)GameObjFM->_New(sizeof(GLShapeInstance_WithParentAndPivot) * Body.ObjSiz);
		Body.LoadBodyAndDefalutKey("Animations\\Player_Rigidbody.txt");
		Body.AnimArr.NULLState();
		Body.AnimArr.SetFM((FM_Model*)GameObjFM);
		Body.AnimArr.Init(1);
		Body.AnimArr[0].LoadAnimation("Animations\\Player_Idle.txt");
		Body.present_animation = &Body.AnimArr[0];

		talkstrId = talkid;
		enable = true;
		return this;
	}

	virtual void Update(float delta) {
		if (enable) {

			velocity = shp::vec3f(0, 0, 0);

			constexpr float AnimationAccel = 5;
			animateTime += AnimationAccel * delta;
			if (animateTime >= maxframes[state]) {
				animateTime = 0;
				//animateTime -= (float)maxframes[state];
			}

			if (Body.AnimArr[state].enable) {
				AnimKey* akarr = Body.AnimArr[state].GetAnim(animateTime);
				for (int i = 0; i < Body.objup; ++i) {
					Body.objArr[i].shape.Translate = akarr[i].translate;
					Body.objArr[i].shape.Rotation = akarr[i].rotation;
					Body.objArr[i].shape.Scale = akarr[i].scale;
				}
			}

			Body.objArr[0].shape.SetScale(glm::vec3(0.01f, 0.01f, 0.01f));
			Body.objArr[0].shape.SetTranslate(shape.Translate + glm::vec3(0, 0.2f, 0));
			Body.objArr[0].shape.SetRotation(shape.Rotation + glm::vec3(0, 90 - 180 * seeingAngle / shp::PI, 0));

			shp::vec3f targetpos = shp::vec3f(parametic3xyz(((GameObject*)Target)->shape.Translate));
			shp::cube6f targetborder = ((GameObject*)Target)->GetBorder(false, 0);
			shp::vec3f thispos = shp::vec3f(parametic3xyz(shape.Translate));
			shp::vec3f dir = targetpos - thispos;
			dir = dir / shp::get_distance3D(shp::vec3f(0, 0, 0), dir);
			float len = get_distance3D(targetpos, thispos);

			if (touched) {
				yadd = 0;
			}

			if (len <= TalkRange) {
				shp::angle2f angle = shp::angle2f(dir.x, dir.z);
				seeingAngle = angle.radian;
				if (Target->RBtn && Target->dialogNextFlow.x > Target->dialogNextFlow.y) {
					Target->PlayerDialogControlFunc(npc_script[talkstrId].talker_name, npc_script[talkstrId].script, true);
					Target->dialogOffset = talkstrId;
				}
				// 대화창
			}

			yadd += gravity * delta;
			velocity.y -= yadd * delta;
		}
	}

	virtual void Render() {
		if (enable) {
			Body.Draw();
		}
	}

	virtual void Event(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		return;
	}

	virtual size_t GetSiz() {
		return sizeof(NPC);
	}

};

class ItemDrop : GameObject {
public:
	int itemid;
	shp::cube6f RootingTrigger = shp::cube6f(-1, -1, -1, 1, 1, 1);
	Player* Target;

	float gravity = 10;
	float yadd = 0;

	ItemDrop() {}
	virtual ~ItemDrop() {}

	ItemDrop* Init(int id, Player* targetobj) {
		ItemDrop go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		itemid = id;
		RootingTrigger = shp::cube6f(-0.3, -0.3, -0.3, 0.3, 0.3, 0.3);
		shape.Init();
		shape.shapeorigin_ID = ItemStart + id;
		shape.ShaderProgram = shaderID;

		border = shp::cube6f(-1, -1, -1, 1, 1, 1);
		shp::cube6f bo;
		vp = *(int*)&bo;
		*((int*)&border) = vp;

		isCollide = true;
		layer = 'p';
		velocity = shp::vec3f(0, 0, 0);

		float f = 0.5f / shapeArr[shape.shapeorigin_ID].BorderSize.y;
		shape.Translate = glm::vec3(0, 0, 0);
		shape.Scale = glm::vec3(f, f, f);

		Target = targetobj;
		enable = true;
		return this;
	}

	virtual void Update(float delta) {
		if (enable) {
			velocity = shp::vec3f(0, 0, 0);

			velocity.y -= 1 * delta;

			constexpr int speed = 50;
			shape.SetRotation(shape.Rotation + glm::vec3(0, speed * delta, 0));
			shp::cube6f range = RootingTrigger;
			range.setCenter(shp::vec3f(parametic3xyz(shape.Translate)));
			if (shp::isCubeContactCube(range, ((GameObject*)Target)->GetBorder(false, 0))) {
				Music::PlayOnce(2);
				Target->AddItem(itemid);
				enable = false;
			}

			velocity.y -= 0.1f * delta;
		}
	}

	virtual void Render() {
		if (enable) {
			shape.Draw();
		}
	}

	virtual void Event(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		return;
	}

	virtual size_t GetSiz() {
		return sizeof(ItemDrop);
	}
};

class Gate : GameObject {
public:
	bool isopen = false;
	shp::cube6f RootingTrigger = shp::cube6f(-1, -1, -1, 1, 1, 1);
	Player* Target;
	glm::vec2 flow = glm::vec2(0, 1);
	glm::vec3 originPos = glm::vec3(0, 0, 0);

	Gate() {}
	virtual ~Gate() {}

	Gate* Init(Player* targetobj) {
		Gate go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		isopen = false;
		shape.Init();
		shape.shapeorigin_ID = 30;
		shape.ShaderProgram = shaderID;
		shape.SetTranslate(glm::vec3(0, 0, 0));

		glm::vec3 b = shapeArr[shape.shapeorigin_ID].BorderSize;
		border = shp::cube6f(-b.x, -b.y, -b.z, b.x, b.y, b.z);
		float f = 1.0f / shapeArr[shape.shapeorigin_ID].BorderSize.y;
		shape.Translate = glm::vec3(0, 0, 0);
		shape.Scale = glm::vec3(f, f, f);

		isCollide = true;
		layer = 'o';
		velocity = shp::vec3f(0, 0, 0);
		RootingTrigger = shp::cube6f(-1, -1, -1, 1, 1, 1);
		Target = targetobj;

		flow = glm::vec2(0, 1);
		originPos = glm::vec3(0, 0, 0);
		enable = true;
		return this;
	}

	virtual void Update(float delta) {
		if (enable) {
			if (isopen) {
				isCollide = false;
				flow.x += delta;
				if (flow.x > flow.y) {
					shape.SetTranslate(originPos + glm::vec3(0, 4, 0));
				}
				else {
					shape.SetTranslate(originPos + glm::vec3(0, 4 * (flow.x / flow.y), 0));
				}
			}

			if (isopen == false) {
				shp::cube6f range = RootingTrigger;
				range.setCenter(shp::vec3f(parametic3xyz(shape.Translate)));
				if (Target->keyNum > 0) {
					if (shp::isCubeContactCube(range, ((GameObject*)Target)->GetBorder(false, 0))) {
						Target->keyNum -= 1;
						isopen = true;
						Music::Stop(1);
						Music::Play(1, true);
						flow.x = 0;
					}
				}
			}
		}
	}

	virtual void Render() {
		if (enable) {
			shape.Draw();
		}
	}

	virtual void Event(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		return;
	}

	virtual size_t GetSiz() {
		return sizeof(GameObject);
	}
};

enum class GameMode {
	PlayMode = 0,
	LevelEditMode = 1,
	AnimationEditMode = 2,
	EffectEditMode = 3
};

bool WKeyPressed = false;
bool SKeyPressed = false;
bool AKeyPressed = false;
bool DKeyPressed = false;
float yrot = 0;
float xrot = 0;

GLShapeInstance LightSource[100];

typedef struct collidRelation {
	char stoplayer = 0;
	char movelayer = 0;
};

typedef struct PostObjData {
	char objtype = 'i'; // i - 아이템 / g - gate / m - 몬스터 / n - NPC / r - road(다음 스테이지)
	glm::vec3 tr;
	glm::vec3 rot;
	glm::vec3 siz;
	int parameter[10] = {};
};

typedef struct Hitbox {
	bool enable = false; // true일때만 공격됨.
	char owner = 'p'; // p - 플레이아 공격 / m - 적 공격
	shp::cube6f box; // 범위
	int damage = 1; // 데미지.
	glm::vec2 flow = glm::vec2(0, 0.3f);
};

constexpr char stageStr[10][128] = {
	"Stages\\Stage0.txt",
	"Stages\\Stage1.txt",
	"Stages\\Stage2.txt",
	"Stages\\Stage3.txt",
	"Stages\\Stage4.txt"
};

constexpr float AnimationAccel = 5;
class GameManager : Ptr {
public:
	//오브젝트들은 FM_MODEL1에 의해 저장되는데, 비활성화 시 lifecheck를 수정
	FM_Model1* GameObjectFreeMem = nullptr;
	static constexpr size_t MAX_OBJECT = 1000;
	GameObject* ObjArr[MAX_OBJECT] = {};
	size_t objup = 0;
	glm::vec2 objArrangement_Flow = glm::vec2(0, 5); // 5초마다 오브젝트가 정리됨.
	GLuint MainShader = 0;

	GameMode gameMode = GameMode::PlayMode;
	int selectID = 0;
	bool isSelect = false;
	shp::cube6f SelectionTrigger = shp::cube6f(-1, -1, -1, 1, 1, 1);
	glm::vec3 SelectionDir = glm::vec3(0, 0, 0);
	GLShapeInstance SelectionCube;
	char selectMod = 'o'; // o - 오브젝트 | ㅣ - 라이트 | c - 콜리더

	collidRelation colRArr[10] = {};
	int colRup = 0;

	GLRigidBody* MainRigidBody;
	char AnimMode = 'd'; // o - 오브젝트 추가 | d - defalut state 정의 | a - 애니메이션
	int presentAnimFrame = 0;
	float presentTime = 0;
	bool isPlayingAnim = false;
	int AnimMaxFrame = 0;

	InfiniteArray<PostObjData> posobjData;

	static constexpr int hitmaxMax = 100;
	Hitbox HitboxArr[hitmaxMax] = {};
	int hitboxup = 0;

	int poststart = 0;
	Player* playerObj = nullptr;

	shp::cube6f portalRange;
	int nextstage = -1;

	bool realPlay = true;

	GameManager() {}
	virtual ~GameManager() {}

	void SetFM(FM_Model1* FM) {
		GameObjectFreeMem = FM;
	}

	void AddObject(GameObject* obj) {
		if (objup + 1 < MAX_OBJECT) {
			obj->shape.SetShader(MainShader);
			ObjArr[objup] = obj;
			objup += 1;
		}
	}

	void Update(float delta) {
		objArrangement_Flow.x += delta;
		for (int i = 0; i < objup; ++i) {
			if (ObjArr[i]->enable == false) continue;
			ObjArr[i]->Update(delta);
		}

		if (objArrangement_Flow.x > objArrangement_Flow.y) {
			objArrangement_Flow.x = 0;

			for (int i = 0; i < objup; ++i) {
				if (ObjArr[i]->enable == false)
				{
					if (GameObjectFreeMem != nullptr) {
						GameObjectFreeMem->_Delete((byte8*)ObjArr[i], ObjArr[i]->GetSiz());

						for (int k = i + 1; k < objup; ++k) {
							ObjArr[k - 1] = ObjArr[k];
						}
						objup -= 1;
					}
				}
			}
		}

		if (gameMode == GameMode::LevelEditMode) {
			if (isSelect) {
				SelectionCube.SetScale(shapeArr[ObjArr[selectID]->shape.shapeorigin_ID].BorderSize * (2 * ObjArr[selectID]->shape.Scale.x));
				SelectionCube.SetTranslate(ObjArr[selectID]->shape.Translate);
			}
		}
		else if (gameMode == GameMode::AnimationEditMode) {
			if (isSelect) {
				GLShapeInstance* shape = &MainRigidBody->objArr[selectID].shape;
				if (shape != nullptr) {
					float size = 1;
					GLShapeInstance_WithParentAndPivot* temp = &MainRigidBody->objArr[selectID];
					size *= temp->shape.Scale.x;
					while (temp->Parent != nullptr) {
						temp = temp->Parent;
						size *= temp->shape.Scale.x;
					}
					SelectionCube.SetScale(AnimFragShapeArr[shape->shapeorigin_ID].BorderSize * (2 * size));
					SelectionCube.SetTranslate(shape->Translate);
				}
			}

			if (isPlayingAnim) {
				presentTime += AnimationAccel * delta;
				if (presentTime >= AnimMaxFrame) {
					presentTime -= (float)AnimMaxFrame;
				}
				presentAnimFrame = (int)(presentTime);

				if (MainRigidBody->AnimArr[0].enable) {
					AnimKey* akarr = MainRigidBody->AnimArr[0].GetAnim(presentTime);
					for (int i = 0; i < MainRigidBody->objup; ++i) {
						MainRigidBody->objArr[i].shape.Translate = akarr[i].translate;
						MainRigidBody->objArr[i].shape.Rotation = akarr[i].rotation;
						MainRigidBody->objArr[i].shape.Scale = akarr[i].scale;
					}
				}
			}
		}
		else if (gameMode == GameMode::PlayMode) {
			playerObj->CameraWork();

			for (int i = 0; i < hitboxup; ++i) {
				Hitbox* hb = &HitboxArr[i];
				if (hb->enable == false) continue;
				hb->flow.x += delta;
				if (hb->flow.x > hb->flow.y) {
					hb->enable = false;
				}
			}
		}

		if (gameMode != GameMode::PlayMode) {
			POINT cursorPos;
			GetCursorPos(&cursorPos);
			float dx = cursorPos.x - maxW / 2;
			float dy = cursorPos.y - maxH / 2;
			dx /= 10;
			dy /= 10;
			if (true) {
				yrot += dx;
				xrot += dy;
				glm::vec4 pos = glm::vec4(0, 0, 1, 1);
				glm::mat4 rotmat = glm::rotate(glm::mat4(1.0), glm::radians(-yrot), glm::vec3(0, 1, 0));
				pos = rotmat * pos;
				glm::mat4 rotmat2 = glm::rotate(glm::mat4(1.0), glm::radians(-xrot), glm::vec3(-pos.z, 0, pos.x));
				pos = rotmat2 * pos;
				viewDir = CamPos + glm::vec3(pos);
				//viewDir = glm::vec3(pos.x, pos.y, pos.z);
			}
			SetCursorPos(maxW / 2, maxH / 2);

			SetCameraTransform(CamPos, viewDir);

			float speed = 2;

			if (WKeyPressed) {
				CamPos += delta * speed * (viewDir - CamPos);
			}

			if (SKeyPressed) {
				CamPos -= delta * speed * (viewDir - CamPos);
			}

			if (AKeyPressed) {
				glm::vec3 vd = viewDir - CamPos;
				CamPos += delta * speed * (glm::vec3(vd.z, 0, -vd.x));
			}

			if (DKeyPressed) {
				glm::vec3 vd = viewDir - CamPos;
				CamPos += delta * speed * (glm::vec3(-vd.z, 0, vd.x));
			}
		}

		if (gameMode != GameMode::PlayMode) {
			return;
		}

		//collision code
		for (int i = 0; i < colRup; ++i) {
			char ml = colRArr[i].movelayer;
			char sl = colRArr[i].stoplayer;
			for (int m = 0; m < objup; ++m) {
				GameObject* M = ObjArr[m];
				bool istouched = false;
				if (M->isCollide && M->layer == ml) {
					for (int s = 0; s < objup; ++s) {
						if (s == m) continue;
						GameObject* S = ObjArr[s];
						if (S->isCollide && S->layer == sl) {
							shp::cube6f mcube_move = M->GetBorder(true);
							shp::cube6f mcube = M->GetBorder(false);
							shp::cube6f scube = S->GetBorder(false);

							if (shp::isCubeContactCube(mcube_move, scube)) {
								bool slide = false;

								shp::cube6f temp = shp::cube6f(mcube_move.fx, mcube.fy, mcube.fz, mcube_move.lx, mcube.ly, mcube.lz);
								if (shp::isCubeContactCube(temp, scube)) {
									M->velocity.x = 0;
									slide = true;
								}

								temp = shp::cube6f(mcube.fx, mcube_move.fy, mcube.fz, mcube.lx, mcube_move.ly, mcube.lz);
								if (shp::isCubeContactCube(temp, scube)) {
									istouched = true;
									M->velocity.y = 0;
									slide = true;
								}

								temp = shp::cube6f(mcube.fx, mcube.fy, mcube_move.fz, mcube.lx, mcube.ly, mcube_move.lz);
								if (shp::isCubeContactCube(temp, scube)) {
									M->velocity.z = 0;
									slide = true;
								}

								if (slide == false) {
									M->velocity = shp::vec3f(0, 0, 0);
								}
							}
						}
					}
				}

				if (istouched) {
					M->touched = true;
				}
				else {
					M->touched = false;
				}

				M->shape.Translate += glm::vec3(parametic3xyz(M->velocity));
				M->velocity = shp::vec3f(0, 0, 0);
			}
		}
	}

	void Render() {
		for (int i = 0; i < objup; ++i) {
			if (ObjArr[i]->enable == false) continue;
			ObjArr[i]->Render();
		}

		if (gameMode == GameMode::LevelEditMode) {
			if (isSelect) {
				SelectionCube.Draw();
			}

			GLShapeInstance temp;
			temp.Init();
			temp.ShaderProgram = shaderID;
			for (int i = 0; i < posobjData.size(); ++i) {
				if (posobjData[i].objtype == 'i') {
					int itemid = posobjData[i].parameter[0];
					temp.shapeorigin_ID = ItemStart + itemid;
					temp.Translate = posobjData[i].tr;
					float f = 1.0f / shapeArr[temp.shapeorigin_ID].BorderSize.y;
					temp.Scale = glm::vec3(f, f, f);
					temp.Draw();
				}
				else if (posobjData[i].objtype == 'g') {
					int flip = posobjData[i].parameter[0];
					temp.shapeorigin_ID = 30;
					temp.Translate = posobjData[i].tr;
					temp.Rotation = posobjData[i].rot;
					float f = 7.0f / shapeArr[temp.shapeorigin_ID].BorderSize.y;
					temp.Scale = glm::vec3(f, f, f);
					temp.Draw();
				}
				else if (posobjData[i].objtype == 'm') {
					int monstart = MonsterStart + posobjData[i].parameter[0];
					temp.shapeorigin_ID = monstart;
					temp.Translate = posobjData[i].tr;
					temp.Rotation = posobjData[i].rot;
					float f = 1.0f / shapeArr[temp.shapeorigin_ID].BorderSize.y;
					temp.Scale = glm::vec3(f, f, f);
					temp.Draw();
				}
				else if (posobjData[i].objtype == 'n') {
					int monstart = 0;
					temp.shapeorigin_ID = monstart;
					temp.Translate = posobjData[i].tr;
					temp.Rotation = posobjData[i].rot;
					float f = 0.3f / shapeArr[temp.shapeorigin_ID].BorderSize.y;
					temp.Scale = glm::vec3(f, f, f);
					temp.Draw();
				}
				else if (posobjData[i].objtype == 'r') {
					temp.shapeorigin_ID = 2;
					temp.Translate = posobjData[i].tr;
					temp.Scale = posobjData[i].siz;
					temp.Draw();
				}
			}

			TCHAR wstr[128] = {};
			wsprintf(wstr, L"선택 모드 : %c", selectMod);
			DrawString(hdc, "Resources\\Font\\MalangmalangR.ttf", 20, wstr, shp::rect4f(0, 0, 1000, 700), Color4f(0, 0, 0, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);
			if (selectMod == 'n') {
				wsprintf(wstr, L"대화 번호 : %d", posobjData[selectID].parameter[0]);
				DrawString(hdc, "Resources\\Font\\MalangmalangR.ttf", 20, wstr, shp::rect4f(0, 100, 1000, 700), Color4f(0, 0, 0, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);
			}
			else if (selectMod == 'r') {
				wsprintf(wstr, L"이동할 스테이지 번호 : %d", posobjData[selectID].parameter[0]);
				DrawString(hdc, "Resources\\Font\\MalangmalangR.ttf", 20, wstr, shp::rect4f(0, 100, 1000, 700), Color4f(0, 0, 0, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);
			}
		}
		else if (gameMode == GameMode::AnimationEditMode) {
			MainRigidBody->Draw();
			if (isSelect) {
				useTRS = false;
				TransformMatrix = MainRigidBody->objArr[selectID].GetTransformMat();
				TransformMatrix = glm::scale(TransformMatrix, glm::vec3(20, 20, 20));
				SelectionCube.Draw();
				useTRS = true;
			}
			TCHAR wstr[128] = {};
			wsprintf(wstr, L"KEY NUM : %d", presentAnimFrame);
			DrawString(hdc, "Resources\\Font\\MalangmalangR.ttf", 50, wstr, shp::rect4f(0, 0, 1000, 700), Color4f(0, 0, 0, 1), TextSortStruct(shp::vec2f(-1, -1), true, true), false);
		}
	}

	void Event(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		static bool ctrl = false;
		static bool alt = false;
		int maxtex = 2;
		if (gameMode == GameMode::LevelEditMode) {
			if (message == WM_KEYDOWN) {
				if (wParam == 'P') {
					selectID = 0;
					isSelect = false;
					switch (selectMod) {
					case 'o': // 오브젝트
						selectMod = 'l';
						break;
					case 'l': // 라이팅
						selectMod = 'c';
						break;
					case 'c':
						selectMod = 'i';
						break;
					case 'i': // 아이템
						selectMod = 'g';
						break;
					case 'g': // 문
						selectMod = 'm';
						break;
					case 'm': // 몬스터
						selectMod = 'n';
						break;
					case 'n': // NPC
						selectMod = 'r';
						break;
					case 'r': // Next Stage로 가는 포탈
						selectMod = 'o';
						break;
					}
				}

				if (selectMod == 'o') {
					if (wParam == 'O') {
						GameObject* go = (GameObject*)GameObjectFreeMem->_New(sizeof(GameObject));
						go->shape.shapeorigin_ID = 1;
						go->Init();
						float coordAv = shapeArr[go->shape.shapeorigin_ID].BorderSize.y + shapeArr[go->shape.shapeorigin_ID].BorderSize.x + shapeArr[go->shape.shapeorigin_ID].BorderSize.z;
						coordAv = 3 / coordAv;
						selectID = objup;
						isSelect = true;
						go->shape.SetScale(glm::vec3(coordAv, coordAv, coordAv));
						AddObject(go);
					}

					if (wParam == 'I') {
						if (isSelect == true) {
							GameObject* go = (GameObject*)GameObjectFreeMem->_New(sizeof(GameObject));
							go->Init();
							*go = GameObject(*ObjArr[selectID]);
							selectID = objup;
							isSelect = true;
							AddObject(go);
						}
					}

					if (wParam == 'T') {
						if (ObjArr[selectID]->shape.Diffuse_Tex2d + 1 < maxtex) {
							ObjArr[selectID]->shape.Diffuse_Tex2d += 1;
						}
						else {
							ObjArr[selectID]->shape.Diffuse_Tex2d = 0;
						}
					}

					if (wParam == 'Q') {
						int n = ObjArr[selectID]->shape.shapeorigin_ID;
						if (n - 1 >= 0) {
							--ObjArr[selectID]->shape.shapeorigin_ID;
							glm::vec3 b = shapeArr[ObjArr[selectID]->shape.shapeorigin_ID].BorderSize;
							ObjArr[selectID]->border = shp::cube6f(-b.x, -b.y, -b.z, b.x, b.y, b.z);
							float coordAv = b.x + b.y + b.z;
							coordAv = 3 / coordAv;
							ObjArr[selectID]->shape.SetScale(glm::vec3(coordAv, coordAv, coordAv));
						}
					}

					if (wParam == 'E') {
						int n = ObjArr[selectID]->shape.shapeorigin_ID;
						if (n + 1 < shapeup) {
							++ObjArr[selectID]->shape.shapeorigin_ID;
							glm::vec3 b = shapeArr[ObjArr[selectID]->shape.shapeorigin_ID].BorderSize;
							ObjArr[selectID]->border = shp::cube6f(-b.x, -b.y, -b.z, b.x, b.y, b.z);
							float coordAv = b.x + b.y + b.z;
							coordAv = 3 / coordAv;
							ObjArr[selectID]->shape.SetScale(glm::vec3(coordAv, coordAv, coordAv));
						}
					}

					constexpr float movSpeed = 1;
					if (wParam == VK_LEFT) {
						if (alt == false) {
							if (ctrl == true) {
								ObjArr[selectID]->shape.Translate.x += movSpeed * 10;
							}
							else {
								ObjArr[selectID]->shape.Translate.x += movSpeed;
							}
						}
						else {
							ObjArr[selectID]->shape.Rotation.y += movSpeed * shp::PI / 8.0f;
						}
					}

					if (wParam == VK_RIGHT) {
						if (alt == false) {
							if (ctrl == true) {
								ObjArr[selectID]->shape.Translate.x -= movSpeed * 10;
							}
							else {
								ObjArr[selectID]->shape.Translate.x -= movSpeed;
							}
						}
						else {
							ObjArr[selectID]->shape.Rotation.y -= movSpeed * shp::PI / 8.0f;
						}
					}

					if (wParam == VK_UP) {
						if (ctrl == true) {
							ObjArr[selectID]->shape.Translate.z += movSpeed * 10;
						}
						else {
							ObjArr[selectID]->shape.Translate.z += movSpeed;
						}
					}

					if (wParam == VK_DOWN) {
						if (ctrl == true) {
							ObjArr[selectID]->shape.Translate.z -= movSpeed * 10;
						}
						else {
							ObjArr[selectID]->shape.Translate.z -= movSpeed;
						}
					}

					if (wParam == 'N') {
						if (ctrl == true) {
							ObjArr[selectID]->shape.Translate.y += movSpeed;
						}
						else {
							ObjArr[selectID]->shape.Translate.y += movSpeed / 10;
						}

					}

					if (wParam == 'M') {
						if (ctrl == true) {
							ObjArr[selectID]->shape.Translate.y -= movSpeed;
						}
						else {
							ObjArr[selectID]->shape.Translate.y -= movSpeed / 10;
						}
					}

					float rate = 0.001f;
					if (wParam == 'Z') {
						float rr = rate;
						if (ctrl) rr = rate * 10;
						float x = ObjArr[selectID]->shape.Scale.x;
						ObjArr[selectID]->shape.Scale = glm::vec3(x - rr, x - rr, x - rr);
					}

					if (wParam == 'C') {
						float rr = rate;
						if (ctrl) rr = rate * 10;
						float x = ObjArr[selectID]->shape.Scale.x;
						ObjArr[selectID]->shape.Scale = glm::vec3(x + rr, x + rr, x + rr);
					}
				}
				else if (selectMod == 'l') {
					if (wParam == 'O') {
						selectID = lightSys.pointlight_up;
						lightSys.AddPointLight(glm::vec3(0, 0, 0), 30, 1, glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

						for (int i = 0; i < lightSys.pointlight_up; ++i) {
							LightSource[i].shapeorigin_ID = 0;
							float siz = 0.2f;
							LightSource[i].ShaderProgram = LightSourceShaderID;
							LightSource[i].SetTranslate(lightSys.pointLightArr[i].position);
							LightSource[i].SetScale(glm::vec3(siz, siz, siz));
							LightSource[i].SetTexture('d', textureArr[0]);
						}
					}

					constexpr float movSpeed = 1;
					if (wParam == VK_LEFT) {
						if (ctrl == true) {
							lightSys.pointLightArr[selectID].position.x += movSpeed * 10;
						}
						else {
							lightSys.pointLightArr[selectID].position.x += movSpeed;
						}
					}

					if (wParam == VK_RIGHT) {
						if (ctrl == true) {
							lightSys.pointLightArr[selectID].position.x -= movSpeed * 10;
						}
						else {
							lightSys.pointLightArr[selectID].position.x -= movSpeed;
						}
					}

					if (wParam == VK_UP) {
						if (ctrl == true) {
							lightSys.pointLightArr[selectID].position.z += movSpeed * 10;
						}
						else {
							lightSys.pointLightArr[selectID].position.z += movSpeed;
						}
					}

					if (wParam == VK_DOWN) {
						if (ctrl == true) {
							lightSys.pointLightArr[selectID].position.z -= movSpeed * 10;
						}
						else {
							lightSys.pointLightArr[selectID].position.z -= movSpeed;
						}
					}

					if (wParam == 'N') {
						if (ctrl == true) {
							lightSys.pointLightArr[selectID].position.y += movSpeed * 10;
						}
						else {
							lightSys.pointLightArr[selectID].position.y += movSpeed;
						}

					}

					if (wParam == 'M') {
						if (ctrl == true) {
							lightSys.pointLightArr[selectID].position.y -= movSpeed * 10;
						}
						else {
							lightSys.pointLightArr[selectID].position.y -= movSpeed;
						}
					}
				}
				else if (selectMod == 'i') {
					if (wParam == 'O') {
						PostObjData pod;
						pod.tr = glm::vec3(0, 0, 0);
						pod.objtype = 'i';
						pod.parameter[0] = 0;
						selectID = posobjData.up;
						posobjData.push_back(pod);
						isSelect = true;
					}

					if (wParam == 'Q') {
						if (posobjData[selectID].parameter[0] - 1 >= 0) {
							posobjData[selectID].parameter[0] -= 1;
						}
					}

					if (wParam == 'E') {
						if (posobjData[selectID].parameter[0] + 1 < MAX_itemID) {
							posobjData[selectID].parameter[0] += 1;
						}
					}

					constexpr float movSpeed = 1;
					if (wParam == VK_LEFT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x += movSpeed;
						}
					}

					if (wParam == VK_RIGHT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x -= movSpeed;
						}
					}

					if (wParam == VK_UP) {
						if (ctrl == true) {
							posobjData[selectID].tr.z += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z += movSpeed;
						}
					}

					if (wParam == VK_DOWN) {
						if (ctrl == true) {
							posobjData[selectID].tr.z -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z -= movSpeed;
						}
					}

					if (wParam == 'N') {
						if (ctrl == true) {
							posobjData[selectID].tr.y += movSpeed;
						}
						else {
							posobjData[selectID].tr.y += movSpeed / 10;
						}

					}

					if (wParam == 'M') {
						if (ctrl == true) {
							posobjData[selectID].tr.y -= movSpeed;
						}
						else {
							posobjData[selectID].tr.y -= movSpeed / 10;
						}
					}
				}
				else if (selectMod == 'g') {
					if (wParam == 'O') {
						PostObjData pod;
						pod.tr = glm::vec3(0, 0, 0);
						pod.rot = glm::vec3(0, 0, 0);
						pod.objtype = 'g';
						pod.parameter[0] = 0; // 0 - 그대로 / 1 - 90도 회전
						selectID = posobjData.up;
						posobjData.push_back(pod);
						isSelect = true;
					}

					if (wParam == 'Q') {
						if (posobjData[selectID].parameter[0] == 0) {
							posobjData[selectID].parameter[0] = 1;
							posobjData[selectID].rot = glm::vec3(0, 90, 0);
						}
						else {
							posobjData[selectID].parameter[0] = 0;
							posobjData[selectID].rot = glm::vec3(0, 0, 0);
						}
					}

					constexpr float movSpeed = 1;
					if (wParam == VK_LEFT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x += movSpeed;
						}
					}

					if (wParam == VK_RIGHT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x -= movSpeed;
						}
					}

					if (wParam == VK_UP) {
						if (ctrl == true) {
							posobjData[selectID].tr.z += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z += movSpeed;
						}
					}

					if (wParam == VK_DOWN) {
						if (ctrl == true) {
							posobjData[selectID].tr.z -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z -= movSpeed;
						}
					}

					if (wParam == 'N') {
						if (ctrl == true) {
							posobjData[selectID].tr.y += movSpeed;
						}
						else {
							posobjData[selectID].tr.y += movSpeed / 10;
						}

					}

					if (wParam == 'M') {
						if (ctrl == true) {
							posobjData[selectID].tr.y -= movSpeed;
						}
						else {
							posobjData[selectID].tr.y -= movSpeed / 10;
						}
					}
				}
				else if (selectMod == 'm') {
					if (wParam == 'O') {
						PostObjData pod;
						pod.tr = glm::vec3(0, 0, 0);
						pod.rot = glm::vec3(0, 0, 0);
						pod.objtype = 'm';
						pod.parameter[0] = 0; // 0 - Snake / ...
						selectID = posobjData.up;
						posobjData.push_back(pod);
						isSelect = true;
					}

					if (wParam == 'Q') {
						if (posobjData[selectID].parameter[0] - 1 >= 0) {
							posobjData[selectID].parameter[0] -= 1;
						}
					}

					if (wParam == 'E') {
						if (posobjData[selectID].parameter[0] + 1 < MAX_itemID) {
							posobjData[selectID].parameter[0] += 1;
						}
					}

					constexpr float movSpeed = 1;
					if (wParam == VK_LEFT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x += movSpeed;
						}
					}

					if (wParam == VK_RIGHT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x -= movSpeed;
						}
					}

					if (wParam == VK_UP) {
						if (ctrl == true) {
							posobjData[selectID].tr.z += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z += movSpeed;
						}
					}

					if (wParam == VK_DOWN) {
						if (ctrl == true) {
							posobjData[selectID].tr.z -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z -= movSpeed;
						}
					}

					if (wParam == 'N') {
						if (ctrl == true) {
							posobjData[selectID].tr.y += movSpeed;
						}
						else {
							posobjData[selectID].tr.y += movSpeed / 10;
						}

					}

					if (wParam == 'M') {
						if (ctrl == true) {
							posobjData[selectID].tr.y -= movSpeed;
						}
						else {
							posobjData[selectID].tr.y -= movSpeed / 10;
						}
					}
				}
				else if (selectMod == 'n') {
					if (wParam == 'O') {
						PostObjData pod;
						pod.tr = glm::vec3(0, 0, 0);
						pod.rot = glm::vec3(0, 0, 0);
						pod.objtype = 'n';
						pod.parameter[0] = 0; // talkID
						selectID = posobjData.up;
						posobjData.push_back(pod);
						isSelect = true;
					}

					if (wParam == 'Q') {
						if (posobjData[selectID].parameter[0] - 1 >= 0) {
							posobjData[selectID].parameter[0] -= 1;
						}
					}

					if (wParam == 'E') {
						if (posobjData[selectID].parameter[0] + 1 < 128) {
							posobjData[selectID].parameter[0] += 1;
						}
					}

					constexpr float movSpeed = 1;
					if (wParam == VK_LEFT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x += movSpeed;
						}
					}

					if (wParam == VK_RIGHT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x -= movSpeed;
						}
					}

					if (wParam == VK_UP) {
						if (ctrl == true) {
							posobjData[selectID].tr.z += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z += movSpeed;
						}
					}

					if (wParam == VK_DOWN) {
						if (ctrl == true) {
							posobjData[selectID].tr.z -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z -= movSpeed;
						}
					}

					if (wParam == 'N') {
						if (ctrl == true) {
							posobjData[selectID].tr.y += movSpeed;
						}
						else {
							posobjData[selectID].tr.y += movSpeed / 10;
						}

					}

					if (wParam == 'M') {
						if (ctrl == true) {
							posobjData[selectID].tr.y -= movSpeed;
						}
						else {
							posobjData[selectID].tr.y -= movSpeed / 10;
						}
					}
				}
				else if (selectMod == 'r') {
					if (wParam == 'O') {
						PostObjData pod;
						pod.tr = glm::vec3(0, 0, 0);
						pod.siz = glm::vec3(1, 1, 1);
						pod.objtype = 'r';
						pod.parameter[0] = 0; // 다음으로 갈 스테이지 id
						selectID = posobjData.up;
						posobjData.push_back(pod);
						isSelect = true;
					}

					if (wParam == 'Q') {
						if (posobjData[selectID].parameter[0] - 1 >= 0) {
							posobjData[selectID].parameter[0] -= 1;
						}
					}

					if (wParam == 'E') {
						if (posobjData[selectID].parameter[0] + 1 < 100) {
							posobjData[selectID].parameter[0] += 1;
						}
					}

					if (wParam == 'Z') {
						posobjData[selectID].siz.x -= 0.1f;
						posobjData[selectID].siz.y -= 0.1f;
						posobjData[selectID].siz.z -= 0.1f;
					}

					if (wParam == 'C') {
						posobjData[selectID].siz.x += 0.1f;
						posobjData[selectID].siz.y += 0.1f;
						posobjData[selectID].siz.z += 0.1f;
					}

					constexpr float movSpeed = 1;
					if (wParam == VK_LEFT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x += movSpeed;
						}
					}

					if (wParam == VK_RIGHT) {
						if (ctrl == true) {
							posobjData[selectID].tr.x -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.x -= movSpeed;
						}
					}

					if (wParam == VK_UP) {
						if (ctrl == true) {
							posobjData[selectID].tr.z += movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z += movSpeed;
						}
					}

					if (wParam == VK_DOWN) {
						if (ctrl == true) {
							posobjData[selectID].tr.z -= movSpeed * 10;
						}
						else {
							posobjData[selectID].tr.z -= movSpeed;
						}
					}

					if (wParam == 'N') {
						if (ctrl == true) {
							posobjData[selectID].tr.y += movSpeed;
						}
						else {
							posobjData[selectID].tr.y += movSpeed / 10;
						}

					}

					if (wParam == 'M') {
						if (ctrl == true) {
							posobjData[selectID].tr.y -= movSpeed;
						}
						else {
							posobjData[selectID].tr.y -= movSpeed / 10;
						}
					}
				}

				if (wParam == VK_CONTROL) {
					ctrl = true;
				}

				if (wParam == 'X') {
					alt = true;
				}

				if (wParam == 'R' || realPlay) {
					
					realPlay = false;
					Music::Play(0, true);
					Music::SetChannelVolume(0, 0.5f);

					poststart = objup;
					playerObj = (Player*)GameObjectFreeMem->_New(sizeof(Player));
					if (playerObj != nullptr) {
						playerObj->Init(this);
						((GameObject*)playerObj)->shape.shapeorigin_ID = 0;
						playerObj->RBtn = true;
						AddObject((GameObject*)playerObj);
					}

					for (int i = 0; i < posobjData.size(); ++i) {
						if (posobjData[i].objtype == 'i') {
							ItemDrop* idobj = (ItemDrop*)GameObjectFreeMem->_New(sizeof(ItemDrop));
							idobj->Init(posobjData[i].parameter[0], playerObj);
							((GameObject*)idobj)->shape.Translate = posobjData[i].tr;
							AddObject((GameObject*)idobj);
						}
						else if (posobjData[i].objtype == 'g') {
							Gate* gateobj = (Gate*)GameObjectFreeMem->_New(sizeof(Gate));
							gateobj->Init(playerObj);
							((GameObject*)gateobj)->shape.Translate = posobjData[i].tr;
							((GameObject*)gateobj)->shape.Rotation = posobjData[i].rot;
							float f = 7.0f / shapeArr[((GameObject*)gateobj)->shape.shapeorigin_ID].BorderSize.y;
							((GameObject*)gateobj)->shape.Scale = glm::vec3(f, f, f);
							gateobj->originPos = posobjData[i].tr;
							shp::vec3f cen = ((GameObject*)gateobj)->border.getCenter();
							glm::vec2 wh = glm::vec2(((GameObject*)gateobj)->border.getw(), ((GameObject*)gateobj)->border.getd());
							if (posobjData[i].parameter[0] == 1) {
								((GameObject*)gateobj)->border.fx = cen.x - wh.y / 2;
								((GameObject*)gateobj)->border.lx = cen.x + wh.y / 2;
								((GameObject*)gateobj)->border.fz = cen.z - wh.x / 2;
								((GameObject*)gateobj)->border.lz = cen.z + wh.x / 2;
							}

							AddObject((GameObject*)gateobj);
						}
						else if (posobjData[i].objtype == 'm') {
							Monster* monsterobj = (Monster*)GameObjectFreeMem->_New(sizeof(Monster));
							monsterobj->Init(posobjData[i].parameter[0], playerObj, this);
							((GameObject*)monsterobj)->shape.Translate = posobjData[i].tr;
							((GameObject*)monsterobj)->shape.Rotation = posobjData[i].rot;
							float f = 1.0f / shapeArr[((GameObject*)monsterobj)->shape.shapeorigin_ID].BorderSize.y;
							((GameObject*)monsterobj)->shape.Scale = glm::vec3(f, f, f);

							AddObject((GameObject*)monsterobj);
						}
						else if (posobjData[i].objtype == 'n') {
							NPC* npcobj = (NPC*)GameObjectFreeMem->_New(sizeof(NPC));
							npcobj->Init(posobjData[i].parameter[0], playerObj, this);
							((GameObject*)npcobj)->shape.Translate = posobjData[i].tr;
							((GameObject*)npcobj)->shape.Rotation = posobjData[i].rot;
							float f = 1.0f / shapeArr[((GameObject*)npcobj)->shape.shapeorigin_ID].BorderSize.y;
							((GameObject*)npcobj)->shape.Scale = glm::vec3(f, f, f);

							AddObject((GameObject*)npcobj);
						}
						else if (posobjData[i].objtype == 'r') {
							glm::vec3 tr = posobjData[i].tr;
							glm::vec3 siz = posobjData[i].siz;
							portalRange = shp::cube6f(tr.x - siz.x, tr.y - siz.y, tr.z - siz.z, tr.x + siz.x, tr.y + siz.y, tr.z + siz.z);
							nextstage = posobjData[i].parameter[0];
						}
					}


					gameMode = GameMode::PlayMode;
				}

				if (wParam == 'U') {
					SaveStage("PresentStage.txt");
				}

				if (wParam == 'L') {
					LoadStage("PresentStage.txt");
				}
			}
			else if (message == WM_KEYUP) {
				if (wParam == VK_CONTROL) {
					ctrl = false;
				}

				if (wParam == 'X') {
					alt = false;
				}
			}

			if (message == WM_RBUTTONDOWN) {
				if (selectMod == 'o') {
					isSelect = false;
					SelectionDir = viewDir - CamPos;
					SelectionTrigger = shp::cube6f(-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f);
					SelectionTrigger.setCenter(shp::vec3f(parametic3xyz(CamPos)));

					bool selectFinish = false;
					for (int i = 0; i < 1000; ++i) {
						for (int k = 0; k < objup; ++k) {
							shp::vec3f objp = shp::vec3f(parametic3xyz(ObjArr[k]->shape.Translate));
							shp::cube6f cube = ObjArr[k]->border;
							cube.setCenter(objp);
							if (shp::isCubeContactCube(SelectionTrigger, cube)) {
								selectFinish = true;
								selectID = k;
								isSelect = true;
								SelectionCube.SetScale(shapeArr[ObjArr[selectID]->shape.shapeorigin_ID].BorderSize * (2 * ObjArr[selectID]->shape.Scale.x));
								SelectionCube.SetTranslate(ObjArr[selectID]->shape.Translate);
								break;
							}
						}

						if (selectFinish) {
							break;
						}

						glm::vec3 cen = glm::vec3(parametic3xyz(SelectionTrigger.getCenter()));
						cen = cen + SelectionDir;
						SelectionTrigger = shp::cube6f(cen.x - 1, cen.y - 1, cen.z - 1, cen.x + 1, cen.y + 1, cen.z + 1);
					}
				}
			}


		}
		else if (gameMode == GameMode::PlayMode) {
			if (message == WM_KEYDOWN) {
				if (wParam == 'R') {
					GameObjectFreeMem->_Delete((byte8*)playerObj, sizeof(Player));
					playerObj = nullptr;

					for (int i = poststart + 1; i < objup; ++i) {
						int n = i - (poststart + 1);
						if (posobjData[n].objtype == 'i') {
							ObjArr[i]->enable = false;
							GameObjectFreeMem->_Delete((byte8*)ObjArr[i], sizeof(ItemDrop));
						}
						else if (posobjData[n].objtype == 'g') {
							ObjArr[i]->enable = false;
							GameObjectFreeMem->_Delete((byte8*)ObjArr[i], sizeof(Gate));
						}
						else if (posobjData[n].objtype == 'm') {
							ObjArr[i]->enable = false;
							GameObjectFreeMem->_Delete((byte8*)ObjArr[i], sizeof(Monster));
						}
						else if (posobjData[n].objtype == 'n') {
							ObjArr[i]->enable = false;
							GameObjectFreeMem->_Delete((byte8*)ObjArr[i], sizeof(NPC));
						}
					}

					objup = poststart;

					gameMode = GameMode::LevelEditMode;
				}
			}
		}
		else if (gameMode == GameMode::AnimationEditMode) {
			if (MainRigidBody == nullptr) {
				MainRigidBody = (GLRigidBody*)GameObjectFreeMem->_New(sizeof(GLRigidBody));
				*MainRigidBody = GLRigidBody();
				MainRigidBody->Init();
				MainRigidBody->ObjSiz = 20;
				MainRigidBody->objArr = (GLShapeInstance_WithParentAndPivot*)GameObjectFreeMem->_New(sizeof(GLShapeInstance_WithParentAndPivot) * MainRigidBody->ObjSiz);
				Animation* anim = (Animation*)GameObjectFreeMem->_New(sizeof(Animation));

				anim->Init(MainRigidBody->ObjSiz, (FM_Model*)GameObjectFreeMem);
				vector<int> aa;
				MainRigidBody->AnimArr.SetFM((FM_Model*)GameObjectFreeMem);
				MainRigidBody->AnimArr.Init(0);
				MainRigidBody->AnimArr.push_back(*anim);
				MainRigidBody->AnimArr[0].Init(20, (FM_Model*)GameObjectFreeMem);
				MainRigidBody->AnimArr.SetVPTR();
			}

			if (message == WM_KEYDOWN) {
				if (isSelect) {
					constexpr float movSpeed = 0.1f;
					constexpr float RotSpeed = 1.0f;
					if (wParam == VK_LEFT) {
						if (alt == false) {
							if (ctrl == true) {
								MainRigidBody->objArr[selectID].shape.Translate.x += movSpeed / 10;
							}
							else {
								MainRigidBody->objArr[selectID].shape.Translate.x += movSpeed;
							}
						}
						else {
							MainRigidBody->objArr[selectID].shape.Rotation.x += RotSpeed * shp::PI / 8.0f;
						}
					}

					if (wParam == VK_RIGHT) {
						if (alt == false) {
							if (ctrl == true) {
								MainRigidBody->objArr[selectID].shape.Translate.x -= movSpeed * 10;
							}
							else {
								MainRigidBody->objArr[selectID].shape.Translate.x -= movSpeed;
							}
						}
						else {
							MainRigidBody->objArr[selectID].shape.Rotation.x -= RotSpeed * shp::PI / 8.0f;
						}
					}

					if (wParam == VK_UP) {
						if (alt == false) {
							if (ctrl == true) {
								MainRigidBody->objArr[selectID].shape.Translate.z += movSpeed / 10;
							}
							else {
								MainRigidBody->objArr[selectID].shape.Translate.z += movSpeed;
							}
						}
						else {
							MainRigidBody->objArr[selectID].shape.Rotation.z += RotSpeed * shp::PI / 8.0f;
						}
					}

					if (wParam == VK_DOWN) {
						if (alt == false) {
							if (ctrl == true) {
								MainRigidBody->objArr[selectID].shape.Translate.z -= movSpeed / 10;
							}
							else {
								MainRigidBody->objArr[selectID].shape.Translate.z -= movSpeed;
							}
						}
						else {
							MainRigidBody->objArr[selectID].shape.Rotation.z -= RotSpeed * shp::PI / 8.0f;
						}
					}

					if (wParam == 'N') {
						if (alt == false) {
							if (ctrl == true) {
								MainRigidBody->objArr[selectID].shape.Translate.y += movSpeed * 10;
							}
							else {
								MainRigidBody->objArr[selectID].shape.Translate.y += movSpeed;
							}
						}
						else {
							MainRigidBody->objArr[selectID].shape.Rotation.y += RotSpeed * shp::PI / 8.0f;
						}
					}

					if (wParam == 'M') {
						if (alt == false) {
							if (ctrl == true) {
								MainRigidBody->objArr[selectID].shape.Translate.y -= movSpeed * 10;
							}
							else {
								MainRigidBody->objArr[selectID].shape.Translate.y -= movSpeed;
							}
						}
						else {
							MainRigidBody->objArr[selectID].shape.Rotation.y -= RotSpeed * shp::PI / 8.0f;
						}
					}
				}

				if (isSelect) {
					if (wParam == 'Z') {
						if (selectID - 1 >= 0) {
							selectID -= 1;
						}
					}

					if (wParam == 'X') {
						if (alt) {
							alt = false;
						}
						else alt = true;
					}

					if (wParam == 'C') {
						if (selectID + 1 < MainRigidBody->objup) {
							selectID += 1;
						}
					}

					if (wParam == 'Q') {
						if (MainRigidBody->objArr[selectID].shape.shapeorigin_ID - 1 >= 0) {
							MainRigidBody->objArr[selectID].shape.shapeorigin_ID -= 1;
						}
					}

					if (wParam == 'E') {
						if (MainRigidBody->objArr[selectID].shape.shapeorigin_ID + 1 < shapeup) {
							MainRigidBody->objArr[selectID].shape.shapeorigin_ID += 1;
						}
					}
				}

				if (AnimMode == 'd') {
					if (wParam == 'O') {
						isSelect = true;
						int n = MainRigidBody->objup;
						MainRigidBody->objArr[MainRigidBody->objup].Init();
						MainRigidBody->objArr[MainRigidBody->objup].shape.ShaderProgram = MainShader;
						if (MainRigidBody->objup > 0) {
							MainRigidBody->objArr[MainRigidBody->objup].Parent = &MainRigidBody->objArr[selectID];
						}
						else {
							constexpr float rate = 0.02f;
							MainRigidBody->objArr[MainRigidBody->objup].shape.SetScale(glm::vec3(rate, rate, rate));
						}
						MainRigidBody->objup += 1;

						selectID = n;
					}

					if (wParam == 'U') {
						MainRigidBody->SetDefalutKeys();
						MainRigidBody->SaveBodyAndDefalutKey("Rigidbody.txt");
						AnimMode = 'k';
					}

					if (wParam == 'L') {
						//Load
						MainRigidBody->LoadBodyAndDefalutKey("Rigidbody.txt");
						isSelect = true;
						selectID = 0;
						//AnimMode = 'k';
					}
				}

				if (AnimMode == 'k') {
					if (wParam == 'K') {
						// 키 프레임 삽입
						for (int i = 0; i < MainRigidBody->objup; ++i) {
							AnimKey ak;
							presentAnimFrame = (int)presentTime;
							ak.time = presentAnimFrame;
							ak.translate = MainRigidBody->objArr[i].shape.Translate;
							ak.rotation = MainRigidBody->objArr[i].shape.Rotation;
							ak.scale = MainRigidBody->objArr[i].shape.Scale;
							MainRigidBody->AnimArr[0].animateArr[i].push_back(ak);
							MainRigidBody->AnimArr[0].SortWithTime();
						}

						if (presentAnimFrame > AnimMaxFrame) {
							AnimMaxFrame = presentAnimFrame;
						}
					}

					if (wParam == 'P') {
						//플레이
						if (isPlayingAnim) isPlayingAnim = false;
						else isPlayingAnim = true;
					}

					if (wParam == 'R') {
						//프레임 이동
						if (presentTime - 1 >= 0) {
							presentTime -= 1;
						}
						presentTime = (int)presentTime;
						//if (presentTime >= AnimMaxFrame) {
						//	presentTime -= (float)AnimMaxFrame;
						//}
						presentAnimFrame = (int)(presentTime);

						if (MainRigidBody->AnimArr[0].enable) {
							AnimKey* akarr = MainRigidBody->AnimArr[0].GetAnim(presentTime);
							for (int i = 0; i < MainRigidBody->objup; ++i) {
								MainRigidBody->objArr[i].shape.Translate = akarr[i].translate;
								MainRigidBody->objArr[i].shape.Rotation = akarr[i].rotation;
								MainRigidBody->objArr[i].shape.Scale = akarr[i].scale;
							}
						}
					}

					if (wParam == 'T') {
						//프레임 이동
						presentTime += 1;
						presentTime = (int)presentTime;
						//if (presentTime >= AnimMaxFrame) {
						//	presentTime -= (float)AnimMaxFrame;
						//}
						presentAnimFrame = (int)(presentTime);

						if (MainRigidBody->AnimArr[0].enable) {
							AnimKey* akarr = MainRigidBody->AnimArr[0].GetAnim(presentTime);
							for (int i = 0; i < MainRigidBody->objup; ++i) {
								MainRigidBody->objArr[i].shape.Translate = akarr[i].translate;
								MainRigidBody->objArr[i].shape.Rotation = akarr[i].rotation;
								MainRigidBody->objArr[i].shape.Scale = akarr[i].scale;
							}
						}
					}

					if (wParam == 'Y') {
						MainRigidBody->AnimArr[0].SaveAnimation("PresentAnimation.txt");
					}

					if (wParam == 'J') {
						MainRigidBody->AnimArr[0].LoadAnimation("PresentAnimation.txt");
						for (int i = 0; i < MainRigidBody->AnimArr[0].ObjSizs; ++i) {
							for (int k = 0; k < MainRigidBody->AnimArr[0].animateArr[i].size(); ++k) {
								float t = MainRigidBody->AnimArr[0].animateArr[i][k].time;
								if (AnimMaxFrame < t) {
									AnimMaxFrame = (int)t;
								}
							}
						}
					}
				}
			}
		}

		for (int i = 0; i < objup; ++i) {
			if (ObjArr[i]->enable == false) continue;
			ObjArr[i]->Event(hwnd, message, wParam, lParam);
		}
	}

	void Init() {
		for (int i = 0; i < hitmaxMax; ++i) {
			HitboxArr[i].enable = false;
		}

		posobjData.NULLState();
		posobjData.SetFM((FM_Model*)GameObjFM);
		posobjData.Init(10);

		SelectionCube.SetShader(MainShader);
		SelectionCube.shapeorigin_ID = 2;
		SelectionCube.SetTexture('d', textureArr[0]);
		SelectionCube.SetTexture('a', textureArr[0]);
		SelectionCube.SetTexture('s', textureArr[0]);
		SelectionCube.SetScale(glm::vec3(1, 1, 1));
		SelectionCube.SetTranslate(glm::vec3(0, 0, 0));

		colRArr[colRup].movelayer = 'p';
		colRArr[colRup].stoplayer = 'o';
		colRup += 1;

		nextstage = -1;
		
		if (gameMode == GameMode::PlayMode) {
			gameMode = GameMode::LevelEditMode;
			realPlay = false;
			LoadStage(stageStr[0]);

			realPlay = true;

			//gameMode = GameMode::PlayMode;
		}
		else {
			realPlay = false;
		}
	}

	void StageRelease() {
		if (gameMode == GameMode::PlayMode) {
			GameObjectFreeMem->_Delete((byte8*)playerObj, sizeof(Player));
			playerObj = nullptr;

			for (int i = poststart + 1; i < objup; ++i) {
				int n = i - (poststart + 1);
				if (posobjData[n].objtype == 'i') {
					ObjArr[i]->enable = false;
					GameObjectFreeMem->_Delete((byte8*)ObjArr[i], sizeof(ItemDrop));
				}
				else if (posobjData[n].objtype == 'g') {
					ObjArr[i]->enable = false;
					GameObjectFreeMem->_Delete((byte8*)ObjArr[i], sizeof(Gate));
				}
				else if (posobjData[n].objtype == 'm') {
					ObjArr[i]->enable = false;
					GameObjectFreeMem->_Delete((byte8*)ObjArr[i], sizeof(Monster));
				}
				else if (posobjData[n].objtype == 'n') {
					ObjArr[i]->enable = false;
					GameObjectFreeMem->_Delete((byte8*)ObjArr[i], sizeof(NPC));
				}
			}

			objup = poststart;

			gameMode = GameMode::LevelEditMode;
		}

		for (int i = 0; i < objup; ++i) {
			GameObjectFreeMem->_Delete((byte8*)ObjArr[i], sizeof(GameObject));
		}
		objup = 0;

		int selectID = 0;
		bool isSelect = false;

		posobjData.clear();

		nextstage = -1;
	}

	void SaveStage(const char* filename) {
		ofstream out;
		out.open(filename);
		out << objup << '\n';
		for (int i = 0; i < objup; ++i) {
			GameObject* obj = ObjArr[i];
			out << obj->isCollide << '\n';
			out << obj->shape.shapeorigin_ID << '\n';
			out << obj->shape.Translate.x << ' ' << obj->shape.Translate.y << ' ' << obj->shape.Translate.z << '\n';
			out << obj->shape.Rotation.x << ' ' << obj->shape.Rotation.y << ' ' << obj->shape.Rotation.z << '\n';
			out << obj->shape.Scale.x << ' ' << obj->shape.Scale.y << ' ' << obj->shape.Scale.z << '\n';
		}

		out << lightSys.pointlight_up << '\n';
		for (int i = 0; i < lightSys.pointlight_up; ++i) {
			out << lightSys.pointLightArr[i].position.x << ' ' << lightSys.pointLightArr[i].position.y << ' ' << lightSys.pointLightArr[i].position.z << '\n';
			out << lightSys.pointLightArr[i].ambient.x << ' ' << lightSys.pointLightArr[i].ambient.y << ' ' << lightSys.pointLightArr[i].ambient.z << '\n';
			out << lightSys.pointLightArr[i].diffuse.x << ' ' << lightSys.pointLightArr[i].diffuse.y << ' ' << lightSys.pointLightArr[i].diffuse.z << '\n';
			out << lightSys.pointLightArr[i].specular.x << ' ' << lightSys.pointLightArr[i].specular.y << ' ' << lightSys.pointLightArr[i].specular.z << '\n';
			out << lightSys.pointLightArr[i].maxRange << '\n';
			out << lightSys.pointLightArr[i].intencity << '\n';
		}

		out << posobjData.size() << '\n';
		for (int i = 0; i < posobjData.size(); ++i) {
			out << posobjData[i].objtype << ' ';
			out << posobjData[i].tr.x << ' ' << posobjData[i].tr.y << ' ' << posobjData[i].tr.z << '\n';
			out << posobjData[i].rot.x << ' ' << posobjData[i].rot.y << ' ' << posobjData[i].rot.z << '\n';
			out << posobjData[i].siz.x << ' ' << posobjData[i].siz.y << ' ' << posobjData[i].siz.z << '\n';
			for (int k = 0; k < 10; ++k) {
				out << posobjData[i].parameter[k] << ' ';
			}
			out << '\n';
		}

		out.close();
	}

	void LoadStage(const char* filename) {
		ifstream in;
		in.open(filename);

		int objnum = 0;
		in >> objnum;
		objup = 0;
		for (int i = 0; i < objnum; ++i) {
			GameObject* obj = (GameObject*)GameObjFM->_New(sizeof(GameObject));
			obj->Init();
			in >> obj->isCollide;
			in >> obj->shape.shapeorigin_ID;
			in >> obj->shape.Translate.x;
			in >> obj->shape.Translate.y;
			in >> obj->shape.Translate.z;
			in >> obj->shape.Rotation.x;
			in >> obj->shape.Rotation.y;
			in >> obj->shape.Rotation.z;
			in >> obj->shape.Scale.x;
			in >> obj->shape.Scale.y;
			in >> obj->shape.Scale.z;
			obj->shape.ShaderProgram = MainShader;
			obj->SetBorder();
			AddObject(obj);
		}

		int lightNum = 0;
		in >> lightNum;
		lightSys.pointlight_up = 0;
		for (int i = 0; i < lightNum; ++i) {
			glm::vec3 pos, amb, dif, spe;
			float maxr, inten;
			in >> pos.x;
			in >> pos.y;
			in >> pos.z;
			in >> amb.x;
			in >> amb.y;
			in >> amb.z;
			in >> dif.x;
			in >> dif.y;
			in >> dif.z;
			in >> spe.x;
			in >> spe.y;
			in >> spe.z;
			in >> maxr;
			in >> inten;

			lightSys.AddPointLight(pos, maxr, inten, amb, dif, spe);
		}

		for (int i = 0; i < lightSys.pointlight_up; ++i) {
			LightSource[i].shapeorigin_ID = 0;
			float siz = 0.2f;
			LightSource[i].ShaderProgram = LightSourceShaderID;
			LightSource[i].SetTranslate(lightSys.pointLightArr[i].position);
			LightSource[i].SetScale(glm::vec3(siz, siz, siz));
			LightSource[i].SetTexture('d', textureArr[0]);
		}

		int n = 0;
		in >> n;
		posobjData.NULLState();
		posobjData.SetFM((FM_Model*)GameObjFM);
		posobjData.Init(n);
		posobjData.up = n;
		for (int i = 0; i < n; ++i) {
			in >> posobjData[i].objtype;

			in >> posobjData[i].tr.x;
			in >> posobjData[i].tr.y;
			in >> posobjData[i].tr.z;

			in >> posobjData[i].rot.x;
			in >> posobjData[i].rot.y;
			in >> posobjData[i].rot.z;

			in >> posobjData[i].siz.x;
			in >> posobjData[i].siz.y;
			in >> posobjData[i].siz.z;

			for (int k = 0; k < 10; ++k) {
				in >> posobjData[i].parameter[k];
			}
		}
	}

	void NextStage(int n) {
		StageRelease();

		LoadStage(stageStr[n]);
		
		poststart = objup;
		playerObj = (Player*)GameObjectFreeMem->_New(sizeof(Player));
		if (playerObj != nullptr) {
			playerObj->Init(this);
			((GameObject*)playerObj)->shape.shapeorigin_ID = 0;
			AddObject((GameObject*)playerObj);
		}

		for (int i = 0; i < posobjData.size(); ++i) {
			if (posobjData[i].objtype == 'i') {
				ItemDrop* idobj = (ItemDrop*)GameObjectFreeMem->_New(sizeof(ItemDrop));
				idobj->Init(posobjData[i].parameter[0], playerObj);
				((GameObject*)idobj)->shape.Translate = posobjData[i].tr;
				AddObject((GameObject*)idobj);
			}
			else if (posobjData[i].objtype == 'g') {
				Gate* gateobj = (Gate*)GameObjectFreeMem->_New(sizeof(Gate));
				gateobj->Init(playerObj);
				((GameObject*)gateobj)->shape.Translate = posobjData[i].tr;
				((GameObject*)gateobj)->shape.Rotation = posobjData[i].rot;
				float f = 7.0f / shapeArr[((GameObject*)gateobj)->shape.shapeorigin_ID].BorderSize.y;
				((GameObject*)gateobj)->shape.Scale = glm::vec3(f, f, f);
				gateobj->originPos = posobjData[i].tr;
				shp::vec3f cen = ((GameObject*)gateobj)->border.getCenter();
				glm::vec2 wh = glm::vec2(((GameObject*)gateobj)->border.getw(), ((GameObject*)gateobj)->border.getd());
				if (posobjData[i].parameter[0] == 1) {
					((GameObject*)gateobj)->border.fx = cen.x - wh.y / 2;
					((GameObject*)gateobj)->border.lx = cen.x + wh.y / 2;
					((GameObject*)gateobj)->border.fz = cen.z - wh.x / 2;
					((GameObject*)gateobj)->border.lz = cen.z + wh.x / 2;
				}

				AddObject((GameObject*)gateobj);
			}
			else if (posobjData[i].objtype == 'm') {
				Monster* monsterobj = (Monster*)GameObjectFreeMem->_New(sizeof(Monster));
				monsterobj->Init(posobjData[i].parameter[0], playerObj, this);
				((GameObject*)monsterobj)->shape.Translate = posobjData[i].tr;
				((GameObject*)monsterobj)->shape.Rotation = posobjData[i].rot;
				float f = 1.0f / shapeArr[((GameObject*)monsterobj)->shape.shapeorigin_ID].BorderSize.y;
				((GameObject*)monsterobj)->shape.Scale = glm::vec3(f, f, f);

				AddObject((GameObject*)monsterobj);
			}
			else if (posobjData[i].objtype == 'n') {
				NPC* npcobj = (NPC*)GameObjectFreeMem->_New(sizeof(NPC));
				npcobj->Init(posobjData[i].parameter[0], playerObj, this);
				((GameObject*)npcobj)->shape.Translate = posobjData[i].tr;
				((GameObject*)npcobj)->shape.Rotation = posobjData[i].rot;
				float f = 1.0f / shapeArr[((GameObject*)npcobj)->shape.shapeorigin_ID].BorderSize.y;
				((GameObject*)npcobj)->shape.Scale = glm::vec3(f, f, f);

				AddObject((GameObject*)npcobj);
			}
			else if (posobjData[i].objtype == 'r') {
				glm::vec3 tr = posobjData[i].tr;
				glm::vec3 siz = posobjData[i].siz;
				portalRange = shp::cube6f(tr.x - siz.x, tr.y - siz.y, tr.z - siz.z, tr.x + siz.x, tr.y + siz.y, tr.z + siz.z);
				nextstage = posobjData[i].parameter[0];
			}
		}

		gameMode = GameMode::PlayMode;
		// 모두 초기화
		// id가 n인 스테이지 로드
	}
};

void Player::IsDamaged() {
	GameManager* GM = (GameManager*)gm;
	for (int i = 0; i < GM->hitmaxMax; ++i) {
		Hitbox hb = GM->HitboxArr[i];
		if (hb.enable == false) continue;
		if (hb.owner == 'm' && shp::isCubeContactCube(GetBorder(false, 0), hb.box)) {
			GM->HitboxArr[i].enable = false;
			HP -= hb.damage;
			yadd = -2;
			touched = false;
		}
	}
}

void Monster::IsDamaged() {
	GameManager* GM = (GameManager*)gm;
	for (int i = 0; i < GM->hitmaxMax; ++i) {
		Hitbox hb = GM->HitboxArr[i];
		if (hb.enable == false) continue;
		if (hb.owner == 'p' && shp::isCubeContactCube(GetBorder(false, 0), hb.box)) {
			Music::Stop(1);
			Music::Play(1, true);
			GM->HitboxArr[i].enable = false;
			HP -= hb.damage;
			yadd = -2;
			touched = false;
		}
	}
}

void Player::AddHitBox(int damage, shp::cube6f range) {
	GameManager* GM = (GameManager*)gm;
	Hitbox hb;

	int count = 0;
	while (GM->HitboxArr[GM->hitboxup].enable) {
		count += 1;
		if (GM->hitboxup + 1 < GM->hitmaxMax) {
			GM->hitboxup += 1;
		}
		else {
			GM->hitboxup = 0;
		}

		if (count > 100) {
			return;
		}
	}

	if (GM->hitboxup + 1 < GM->hitmaxMax) {
		if (GM->HitboxArr[GM->hitboxup].enable == false) {
			hb.box = range;
			hb.damage = damage;
			hb.owner = 'p';
			hb.enable = true;
			hb.flow = glm::vec2(0, 0.3f);
			GM->HitboxArr[GM->hitboxup] = hb;
			GM->hitboxup += 1;
			
		}
	}
}

void Monster::AddHitBox(int damage, shp::cube6f range) {
	GameManager* GM = (GameManager*)gm;
	Hitbox hb;

	int count = 0;
	while (GM->HitboxArr[GM->hitboxup].enable) {
		count += 1;
		if (GM->hitboxup + 1 < GM->hitmaxMax) {
			GM->hitboxup += 1;
		}
		else {
			GM->hitboxup = 0;
		}

		if (count > 100) {
			return;
		}
	}

	if (GM->hitboxup + 1 < GM->hitmaxMax) {
		hb.box = range;
		hb.damage = damage;
		hb.owner = 'm';
		hb.flow = glm::vec2(0, 0.3f);
		hb.enable = true;
		GM->HitboxArr[GM->hitboxup] = hb;
		GM->hitboxup += 1;
	}
}

void Player::GoNextStage() {
	GameManager* GM = (GameManager*)gm;
	shp::cube6f prange = GetBorder(false, 0);
	if (shp::isCubeContactCube(prange, GM->portalRange) && GM->nextstage >= 0) {
		// 다음 스테이지..
		GM->NextStage(GM->nextstage);
	}
}

FM_Model1* SpaceDivideFreeMem;
class SpaceDivide {
public:
	SpaceDivide* space[3][3][3] = { {{}} };
	shp::cube6f Range;
	int depth = 0;
	float W_inOneChunk = 10.0f; // 한 정육면체인 청크의 한 모서리의 길이.

	SpaceDivide() {}
	virtual ~SpaceDivide() {}

	//x, y, z의 최대값
	float GetMaxLen(int depth) {
		if (depth == 0) return 1;
		return W_inOneChunk * (3 * GetMaxLen(depth - 1) + 1);
	}

	SpaceDivide* GetSpaceDivide_Chunk(float x, float y, float z) {
		float len = GetMaxLen(depth);
		float plen = GetMaxLen(depth - 1); //최적화
		int sx, sy, sz;
		if (x < -plen) {
			sx = 0;
		}
		else if (x > plen) {
			sx = 2;
		}
		else {
			sx = 1;
		}

		if (y < -plen) {
			sy = 0;
		}
		else if (y > plen) {
			sy = 2;
		}
		else {
			sy = 1;
		}

		if (z < -plen) {
			sz = 0;
		}
		else if (z > plen) {
			sz = 2;
		}
		else {
			sz = 1;
		}

		if (space[sx][sy][sz] == nullptr) {
			return nullptr;
			/*space[sx][sy][sz] = (SpaceDivide*)SpaceDivideFreeMem._New(sizeof(SpaceDivide));
			return space[sx][sy][sz];*/
		}
		else {
			return space[sx][sy][sz]->GetSpaceDivide_Chunk(
				x + len - sx * 2 * plen,
				y + len - sy * 2 * plen,
				z + len - sz * 2 * plen);
		}
	}

	SpaceDivide* AddSpcaeDivide_Chunk(float x, float y, float z) {
		float len = GetMaxLen(depth);
		float plen = GetMaxLen(depth - 1); //최적화
		int sx, sy, sz;
		if (x < -plen) {
			sx = 0;
		}
		else if (x > plen) {
			sx = 2;
		}
		else {
			sx = 1;
		}

		if (y < -plen) {
			sy = 0;
		}
		else if (y > plen) {
			sy = 2;
		}
		else {
			sy = 1;
		}

		if (z < -plen) {
			sz = 0;
		}
		else if (z > plen) {
			sz = 2;
		}
		else {
			sz = 1;
		}


		if (space[sx][sy][sz] == nullptr) {
			space[sx][sy][sz] = (SpaceDivide*)SpaceDivideFreeMem->_New(sizeof(SpaceDivide));
			if (depth == 0) {
				return space[sx][sy][sz];
			}
			else {
				space[sx][sy][sz]->depth = depth - 1;
				space[sx][sy][sz]->Range = shp::cube6f(
					-len + sx * 2 * plen, -len + sy * 2 * plen, -len + sz * 2 * plen,
					-len + (sx + 1) * 2 * plen, -len + (sy + 1) * 2 * plen, -len + (sz + 1) * 2 * plen);
				space[sx][sy][sz]->W_inOneChunk = W_inOneChunk;
				return space[sx][sy][sz]->AddSpcaeDivide_Chunk(
					x + len - sx * 2 * plen,
					y + len - sy * 2 * plen,
					z + len - sz * 2 * plen);
			}
		}
		else {
			return space[sx][sy][sz]->GetSpaceDivide_Chunk(
				x + len - sx * 2 * plen,
				y + len - sy * 2 * plen,
				z + len - sz * 2 * plen);
		}
	}
};

GLvoid Reshape(int w, int h);

GLSkyBox skybox;

GLShapeInstance xcoord, ycoord, zcoord;

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	errno_t err;

	err = fopen_s(&fptr, file, "rb"); /* Open file for reading */
	if (err) /* Return NULL on failure */
		return NULL;
	fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
	length = ftell(fptr); /* Find out how many bytes into the file we are */
	buf = (char*)malloc(length + 1); /* Allocate a buffer for the entire length of the file and a null terminator */
	fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
	fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
	fclose(fptr); /* Close the file */
	buf[length] = 0; /* Null terminator */

	return buf; /* Return the buffer */
}

GLuint Setshader(const char* vshader, const char* fshader) {
	char* vertexsource = filetobuf(vshader);

	GLuint VertexShader;
	GLuint FragmentShader;

	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &vertexsource, NULL);
	glCompileShader(VertexShader);

	GLint result;
	char errorLog[512];
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(VertexShader, 512, NULL, errorLog);
		cout << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
		return false;
	}

	char* fragmentsource = filetobuf(fshader);
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &fragmentsource, NULL);
	glCompileShader(FragmentShader);

	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(FragmentShader, 512, NULL, errorLog);
		cout << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
		return false;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, VertexShader);
	glAttachShader(program, FragmentShader);
	glLinkProgram(program);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	glUseProgram(program);

	return program;
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수 
{
	glViewport(0, 0, w, h);
}

//bool anim = false;
int addr = 1;
float t = 0;
float rotT = 0;
float memCheckFlow[2] = { 0, 1000 };

GameManager gameManager;

void RunMessageLoop() {
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool disableDisplay = true;
HRESULT Initialize() {
	HRESULT hr;
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(LONG_PTR);
	wcex.hInstance = hInst;
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	wcex.lpszClassName = L"3DOpenGLStandard0";
	RegisterClassEx(&wcex);

	//CreateWindow
	hWndMain = CreateWindow(L"3DOpenGLStandard0", L"3DOpenGLStandard0",
		WS_POPUP, 0, 0,
		maxW, maxH,
		NULL, NULL, hInst, NULL);
	hr = hWndMain ? S_OK : E_FAIL;
	if (SUCCEEDED(hr)) {
		ShowWindow(hWndMain, SW_SHOWNORMAL);
		UpdateWindow(hWndMain);
	}

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) // glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	return hr;
}

void Init() {
	Music::Init();
	Music::AddSound("Resources\\Sound\\CrowNight of Owl.mp3", true, true);
	Music::ConnectSound(0, 0);

	Music::AddSound("Resources\\Sound\\enemy_dead.wav", false, false);
	Music::ConnectSound(1, 1);

	Music::AddSound("Resources\\Sound\\item.wav", false, false);
	Music::ConnectSound(2, 2);

	Music::AddSound("Resources\\Sound\\sword0.wav", false, false);
	Music::ConnectSound(3, 3);

	float viewDistance = 8.0f;
	CamPos = glm::vec3(0, 0, -5);
	viewDir = glm::vec3(0, 0, 1);
	SetCameraTransform(CamPos, viewDir);

	shaderID = Setshader("Shaders\\ST3_VShader.glsl", "Shaders\\ST3_FShader.glsl");
	LightSourceShaderID = Setshader("Shaders\\ST3_LightSource_VShader.glsl", "Shaders\\ST3_LightSource_FShader.glsl");
	textShaderID = Setshader("Shaders\\text_VShader.glsl", "Shaders\\text_FShader.glsl");
	ScreenShaderID = Setshader("Shaders\\screen_VShader.glsl", "Shaders\\screen_FShader.glsl");
	skybox.Init();

	lightSys.dirLight = { glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.01f, 0.01f, 0.01f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.5f, 0.5f, 0.5f) };

	float intencity = 1.0f;
	float rad = 50.0f;
	//lightSys.AddPointLight(glm::vec3(0, 0, 0), 30, intencity, glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
	/*lightSys.AddPointLight(glm::vec3(rad, 0, rad), 30, intencity, glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
	lightSys.AddPointLight(glm::vec3(rad, 0, -rad), 30, intencity, glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
	lightSys.AddPointLight(glm::vec3(-rad, 0, -rad), 30, intencity, glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
	lightSys.AddPointLight(glm::vec3(-rad, 0, rad), 30, intencity, glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));*/

	AddTexture("Resources\\Texture\\DefaultTexure.jpg"); //0
	AddTexture("Resources\\Texture\\bottom0.jpg"); //1
	AddTexture("Resources\\Texture\\HeartIcon.jpg"); //2
	AddTexture("Resources\\Texture\\KeyIcon.jpg"); //3
	AddTexture("Resources\\Texture\\DialogBox.jpg"); //4
	AddTexture("Resources\\Mesh\\stones\\stone.jpg"); // 5
	AddTexture("Resources\\Texture\\PotionIcon.jpg"); //6

	//0
	{
		GLShapeOrigin o;
		loadOBJ(&o, "Resources\\BasicCube.obj");
		InitShapeOrigin_OBJ(&o);
		AddShape(o, textureArr[0]);
	}

	//1
	{
		GLShapeOrigin o;
		loadOBJ(&o, "Resources\\Bottom.obj");
		InitShapeOrigin_OBJ(&o);
		AddShape(o, textureArr[1]);
	}

	//2
	{
		GLShapeOrigin o;
		loadOBJ(&o, "Resources\\selectionObj.obj");
		InitShapeOrigin_OBJ(&o);
		AddShape(o, textureArr[0]);
	}


	AddShape_FromFileName("Resources\\Mesh\\boat\\boat.obj", "Resources\\Mesh\\boat\\boat.jpg"); // 3

	AddShape_FromFileName("Resources\\Mesh\\bread\\bread.obj", "Resources\\Mesh\\bread\\bread.jpg"); // 4
	AddShape_FromFileName("Resources\\Mesh\\chair\\chair.obj", "Resources\\Mesh\\chair\\chair.jpg"); //5
	AddShape_FromFileName("Resources\\Mesh\\closet\\closet.obj", "Resources\\Mesh\\closet\\closet.jpg");//6
	AddShape_FromFileName("Resources\\Mesh\\desk\\desk.obj", "Resources\\Mesh\\desk\\desk.jpg");//7
	AddShape_FromFileName("Resources\\Mesh\\door\\door.obj", "Resources\\Mesh\\door\\door.jpg");//8

	AddShape_FromFileName("Resources\\Mesh\\mountain\\mountain.obj", "Resources\\Mesh\\mountain\\mountain.jpg");//9

	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\tree0.obj", "Resources\\Mesh\\NaturePack\\tree0.jpg");//10
	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\tree1.obj", "Resources\\Mesh\\NaturePack\\tree1.jpg");//11
	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\tree2.obj", "Resources\\Mesh\\NaturePack\\tree2.jpg");//12
	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\tree3.obj", "Resources\\Mesh\\NaturePack\\tree3.jpg");//13
	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\tree4.obj", "Resources\\Mesh\\NaturePack\\tree4.jpg");//14

	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\grass0.obj", "Resources\\Mesh\\NaturePack\\grass0.jpg");//15

	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\log0.obj", "Resources\\Mesh\\NaturePack\\log0.jpg");//16
	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\log1.obj", "Resources\\Mesh\\NaturePack\\log1.jpg");//17

	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\mush0.obj", "Resources\\Mesh\\NaturePack\\mush0.jpg");//18
	AddShape_FromFileName("Resources\\Mesh\\NaturePack\\mush1.obj", "Resources\\Mesh\\NaturePack\\mush1.jpg");//19

	AddShape_FFN_withTex("Resources\\Mesh\\stones\\smallStone0.obj", textureArr[5]); // 20
	AddShape_FFN_withTex("Resources\\Mesh\\stones\\smallStone1.obj", textureArr[5]); // 21
	AddShape_FFN_withTex("Resources\\Mesh\\stones\\smallStone2.obj", textureArr[5]); // 22
	AddShape_FFN_withTex("Resources\\Mesh\\stones\\smallStone3.obj", textureArr[5]); // 23
	AddShape_FFN_withTex("Resources\\Mesh\\stones\\smallStone4.obj", textureArr[5]); // 24
	AddShape_FFN_withTex("Resources\\Mesh\\stones\\smallStone5.obj", textureArr[5]); // 25
	AddShape_FFN_withTex("Resources\\Mesh\\stones\\smallStone6.obj", textureArr[5]); // 26
	AddShape_FFN_withTex("Resources\\Mesh\\stones\\BigRock0.obj", textureArr[5]); // 27
	AddShape_FFN_withTex("Resources\\Mesh\\stones\\BigRock1.obj", textureArr[5]); // 28
	AddShape_FFN_withTex("Resources\\Mesh\\stones\\BigRock2.obj", textureArr[5]); // 29
	AddShape_FromFileName("Resources\\Mesh\\Gate\\Gate.obj", "Resources\\Mesh\\Gate\\Gate.jpg"); // 30

	UIStart = 31;
	//31
	{
		GLShapeOrigin o = InitShapeOrigin_UI(shp::rect4f(-50, -50, 50, 50));
		AddShape(o, -1);
	}

	//32
	{
		GLShapeOrigin o = InitShapeOrigin_UI(shp::rect4f(-maxW / 2, -maxH / 6, maxW / 2, maxH / 6));
		AddShape(o, textureArr[4]);
	}

	ItemStart = 33;
	AddShape_FromFileName("Resources\\Mesh\\Key\\Key.obj", "Resources\\Mesh\\Key\\Key.jpg"); //33
	AddShape_FromFileName("Resources\\Mesh\\Potion\\Potion.obj", "Resources\\Mesh\\Potion\\Potion.jpg"); //34
	AddShape_FromFileName("Resources\\Mesh\\Heart\\Heart.obj", "Resources\\Mesh\\Heart\\Heart.jpg"); //35

	MonsterStart = 36;
	AddShape_FromFileName("Resources\\Mesh\\Snake\\Snake_Head.obj", "Resources\\Mesh\\Snake\\Snake_Head.jpg");


	AddAnimShape(shapeArr[0], textureArr[0]);
	AddAnimShape(shapeArr[2], textureArr[0]);

	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_Face.obj", "Resources\\Mesh\\Player\\Player_Face.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_Chest.obj", "Resources\\Mesh\\Player\\Player_Chest.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_RightArm.obj", "Resources\\Mesh\\Player\\Player_Arm.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_LeftArm.obj", "Resources\\Mesh\\Player\\Player_Arm.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_RightHand.obj", "Resources\\Mesh\\Player\\Player_Hand.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_LeftHand.obj", "Resources\\Mesh\\Player\\Player_Hand.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_RightLeg.obj", "Resources\\Mesh\\Player\\Player_Leg.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_LeftLeg.obj", "Resources\\Mesh\\Player\\Player_Leg.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_RightSubLeg.obj", "Resources\\Mesh\\Player\\Player_Leg.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_LeftSubLeg.obj", "Resources\\Mesh\\Player\\Player_Leg.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_RightFoot.obj", "Resources\\Mesh\\Player\\Foot.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_LeftFoot.obj", "Resources\\Mesh\\Player\\Foot.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Player\\Player_Sword.obj", "Resources\\Mesh\\Player\\Player_Sword.jpg");

	AddAnimShape_FromFileName("Resources\\Mesh\\Snake\\Snake_Head.obj", "Resources\\Mesh\\Snake\\Snake_Head.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Snake\\Snake_Neck.obj", "Resources\\Mesh\\Snake\\Snake_Body.jpg");
	AddAnimShape_FromFileName("Resources\\Mesh\\Snake\\Snake_Tail.obj", "Resources\\Mesh\\Snake\\Snake_Body.jpg");


	gameManager.MainShader = shaderID;
	gameManager.Init();

	float lim = 0.03f;
	xcoord.shapeorigin_ID = 0;
	xcoord.SetShader(LightSourceShaderID);
	xcoord.SetTexture('d', textureArr[0]);
	xcoord.SetTexture('a', textureArr[0]);
	xcoord.SetTexture('s', textureArr[0]);
	xcoord.SetScale(glm::vec3(100, lim, lim));
	xcoord.SetTranslate(glm::vec3(0, 0, 0));

	ycoord.shapeorigin_ID = 0;
	ycoord.SetShader(LightSourceShaderID);
	ycoord.SetTexture('d', textureArr[0]);
	ycoord.SetTexture('a', textureArr[0]);
	ycoord.SetTexture('s', textureArr[0]);
	ycoord.SetScale(glm::vec3(lim, 100, lim));
	ycoord.SetTranslate(glm::vec3(0, 0, 0));

	zcoord.shapeorigin_ID = 0;
	zcoord.SetShader(LightSourceShaderID);
	zcoord.SetTexture('d', textureArr[0]);
	zcoord.SetTexture('a', textureArr[0]);
	zcoord.SetTexture('s', textureArr[0]);
	zcoord.SetScale(glm::vec3(lim, lim, 100));
	zcoord.SetTranslate(glm::vec3(0, 0, 0));
	//{
	//	GLShapeOrigin o;
	//	loadOBJ(&o, "Resources\\Mesh\\bread\\bread.obj");
	//	InitShapeOrigin_OBJ(&o);
	//	AddShape(o, AddTexture("Resources\\Mesh\\bread\\bread.jpg"));
	//}

	for (int i = 0; i < lightSys.pointlight_up; ++i) {
		LightSource[i].shapeorigin_ID = 0;
		float siz = 0.2f;
		LightSource[i].ShaderProgram = LightSourceShaderID;
		LightSource[i].SetTranslate(lightSys.pointLightArr[i].position);
		LightSource[i].SetScale(glm::vec3(siz, siz, siz));
		LightSource[i].SetTexture('d', textureArr[0]);
	}

	SetViewTransform();

	CheckRemainMemorySize();

	disableDisplay = false;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//HeapSetInformation(NULL,
	//	HeapEnableTerminationOnCorruption, NULL, 0);
	if (FT_Init_FreeType(&library)) {
		MessageBox(NULL, TEXT("FreeType 라이브러리 초기화 실패"), TEXT("에러"), MB_OK);
		return -1;
	}

	srand((unsigned int)time(0));

	CheckRemainMemorySize();
	constexpr int ShapeHeapAllocSiz = 500000;
	byte8* FM0_Shape_Heap = new byte8[ShapeHeapAllocSiz];
	ShapeFreeMem = new FM_Model0();
	ShapeFreeMem->SetHeapData(FM0_Shape_Heap, ShapeHeapAllocSiz);

	constexpr int InstanceHeapAllocSiz = 300000;
	byte8* FM0_Instance_Heap = new byte8[InstanceHeapAllocSiz];
	InstanceFreeMem = new FM_Model0();
	InstanceFreeMem->SetHeapData(FM0_Instance_Heap, InstanceHeapAllocSiz);

	constexpr int SpaceDivideHeapAllocSiz = 50000;
	byte8* FM0_SpaceDivide_Heap = new byte8[SpaceDivideHeapAllocSiz];
	SpaceDivideFreeMem = new FM_Model1();
	SpaceDivideFreeMem->SetHeapData(FM0_SpaceDivide_Heap, SpaceDivideHeapAllocSiz);

	constexpr int GameObjAllocSiz = 500000;
	byte8* FM0_GameObj_Heap = new byte8[GameObjAllocSiz];
	GameObjFM = new FM_Model1();
	GameObjFM->SetHeapData(FM0_GameObj_Heap, GameObjAllocSiz);
	gameManager.SetFM(GameObjFM);

	/*if (FT_Init_FreeType(&library)) {
		MessageBox(NULL, TEXT("FreeType 라이브러리 초기화 실패"), TEXT("에러"), MB_OK);
		return -1;
	}*/

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		if (SUCCEEDED(Initialize())) {
			Init();

			RunMessageLoop();
		}
		CoUninitialize();
	}

	//FT_Done_FreeType(library);

	delete ShapeFreeMem;
	delete InstanceFreeMem;
	delete SpaceDivideFreeMem;
	delete GameObjFM;

	FT_Done_FreeType(library);

	return 0;
}

void Update(float delta);
void Update(float delta) {
	if (disableDisplay) return;
	t += 0.05f;
	rotT += 0.001f;


	memCheckFlow[0] += 1;
	if (memCheckFlow[0] > memCheckFlow[1]) {
		memCheckFlow[0] = 0;
		//ShapeFreeMem.ClearAll();
		ShapeFreeMem->PrintState();
	}

	for (int i = 0; i < lightSys.pointlight_up; ++i) {
		LightSource[i].SetTranslate(lightSys.pointLightArr[i].position);
	}
}

clock_t savec = 0, prec = 0;
unsigned int stackTime;
DWORD WINAPI TimeLoop(LPVOID lpParameter)
{
	// The new thread will start here
	while (1) {
		if (disableDisplay) continue;

		savec = prec;
		prec = clock();
		stackTime += prec - savec;
		if (stackTime > 15) {
			Music::Update();
			float delta = stackTime / 1000.0f;
			Update(delta);
			gameManager.Update(delta);
			InvalidateRect(hWndMain, &DisplayRt, FALSE);
			stackTime = 0;
		}
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	static HANDLE hTimeLoopThread = NULL;
	PAINTSTRUCT ps;

	gameManager.Event(hwnd, message, wParam, lParam);

	if (message == WM_CREATE) {
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		hWndMain = hwnd;
		PIXELFORMATDESCRIPTOR pfd;
		int nPixelFormat;
		hdc = GetDC(hwnd);
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		nPixelFormat = ChoosePixelFormat(hdc, &pfd);
		SetPixelFormat(hdc, nPixelFormat, &pfd);
		hrc = wglCreateContext(hdc);
		wglMakeCurrent(hdc, hrc);

		hTimeLoopThread = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			TimeLoop, // Thread start address
			NULL,    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);   // Thread id
		if (hTimeLoopThread == NULL)
		{
			// Thread creation failed.
			// More details can be retrieved by calling GetLastError()
			return 1;
		}

		result = 1;
	}
	else {
		//OpenGLUI::AllEvent(hwnd, message, wParam, lParam);

		bool wasHandled = false;
		switch (message) {
		case WM_SIZE:
		{
			GetClientRect(hwnd, &DisplayRt);
			int w = DisplayRt.right - DisplayRt.left;
			int h = DisplayRt.bottom - DisplayRt.top;

			glViewport(0, 0, w, h);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			/*glOrtho(-1, 1, -1, 1, 1, -1);*/
			//윈도우 크기를 설정(OpenGL상)
			//gluOrtho2D(0, w, h, 0);
			//윈도우 크기를 일정하게 설정

			GetWindowRect(hwnd, &WindowRt);
			w = WindowRt.right - WindowRt.left;
			h = WindowRt.bottom - WindowRt.top;
			MoveWindow(hwnd, WindowRt.left, WindowRt.top, w, h, TRUE);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			result = 0;
			wasHandled = true;
			break;
		}
		case WM_PAINT: {
			DoDisplay();
			break;
		}
		case WM_KEYDOWN:
		{
			if (wParam == 'W') {
				WKeyPressed = true;
			}
			if (wParam == 'S') {
				SKeyPressed = true;
			}
			if (wParam == 'A') {
				AKeyPressed = true;
			}
			if (wParam == 'D') {
				DKeyPressed = true;
			}
			if (wParam == VK_ESCAPE) {
				DestroyWindow(hwnd);
			}
		}
		break;
		case WM_KEYUP:
			if (wParam == 'W') {
				WKeyPressed = false;
			}
			if (wParam == 'S') {
				SKeyPressed = false;
			}
			if (wParam == 'A') {
				AKeyPressed = false;
			}
			if (wParam == 'D') {
				DKeyPressed = false;
			}
			break;
		case WM_CHAR:
			break;
		case WM_LBUTTONDOWN:
			break;
		case WM_RBUTTONDOWN:
			break;
		case WM_MOUSEMOVE:
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			break;
		case WM_DESTROY:
		{
			CloseHandle(hTimeLoopThread);
			wglMakeCurrent(hdc, NULL);
			wglDeleteContext(hrc);
			KillTimer(hwnd, 0);
			ReleaseDC(hwnd, hdc);
			PostQuitMessage(0);
			Music::Release();
			result = 1;
			wasHandled = true;
			break;
		}

		}

		if (!wasHandled) {
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
	return result;
}

GLuint textVAO;
GLuint textVBO;
void DrawString(HDC hdc, const char* fontname, const int fontsiz, const TCHAR* str, shp::rect4f loc, Color4f color,
	TextSortStruct tss, bool bDebug) {
	glPointSize(1);
	FT_Face face;
	int error;
	int x, y;
	int penx;
	int peny;
	int idx;
	int len = lstrlen(str);

	if (len == 0) return;

	error = FT_New_Face(library, fontname, 0, &face);
	error = FT_Set_Char_Size(face, fontsiz * 64, 0, GetDeviceCaps(hdc, LOGPIXELSX), GetDeviceCaps(hdc, LOGPIXELSX));

	int lineh = 0;
	int linetop = 0;
	if (tss.LineHeightUnity) {
		//한줄의 높이의 길이 구하기
		FT_Load_Char(face, L'A', FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
		lineh = face->glyph->bitmap.rows * tss.LineHeightrate;
		linetop = face->glyph->bitmap_top;
	}

	//텍스트의 rect를 구한다.
	shp::rect4f textRT;
	int additionX = 0;
	int additionY = 0;
	textRT.fy = 9999;
	textRT.ly = -9999;
	for (idx = 0; idx < len; idx++) {
		FT_Load_Char(face, str[idx], FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);

		if (idx == 0) {
			textRT.fx = face->glyph->bitmap_left;
		}

		if (idx == len - 1) {
			textRT.lx = additionX + face->glyph->bitmap_left + face->glyph->bitmap.width;
		}

		if (textRT.fy > additionY - face->glyph->bitmap_top) {
			if (tss.LineHeightUnity) {
				textRT.fy = additionY - linetop;
			}
			else {
				textRT.fy = additionY - face->glyph->bitmap_top;
			}
		}

		if (textRT.ly < additionY - face->glyph->bitmap_top + face->glyph->bitmap.rows) {
			if (tss.LineHeightUnity) {
				textRT.ly = additionY - linetop + lineh;
			}
			else {
				textRT.ly = additionY - face->glyph->bitmap_top + face->glyph->bitmap.rows;
			}
		}

		additionX += face->glyph->advance.x / 64;
		additionY += face->glyph->advance.y / 64;
	}

	int limitLine = 0;
	if (tss.drawNextLines == false) {
		//x 정렬
		if (tss.positioning.x > 0) {
			textRT.moveValue("lx", loc.lx);
		}
		else if (tss.positioning.x == 0) {
			textRT.moveValue("cx", loc.getCenter().x);
		}
		else {
			textRT.moveValue("fx", loc.fx);
		}
		//y정렬
		if (tss.positioning.y > 0) {
			textRT.moveValue("ly", loc.ly);
		}
		else if (tss.positioning.y == 0) {
			textRT.moveValue("cy", loc.getCenter().y);
		}
		else {
			textRT.moveValue("fy", loc.fy);
		}

		if (bDebug) {
			//입력된 범위 표시
			glColor4f(1, 0, 0, 1);
			glBegin(GL_LINE_LOOP);
			glVertex2f(loc.fx, loc.fy);
			glVertex2f(loc.lx, loc.fy);
			glVertex2f(loc.lx, loc.ly);
			glVertex2f(loc.fx, loc.ly);
			glEnd();

			//실제 들어갈 수 있는 범위 표시
			glColor4f(0, 1, 0, 1);
			glBegin(GL_LINE_LOOP);
			glVertex2f(textRT.fx, textRT.fy);
			glVertex2f(textRT.lx, textRT.fy);
			glVertex2f(textRT.lx, textRT.ly);
			glVertex2f(textRT.fx, textRT.ly);
			glEnd();
		}
	}
	else {
		int n = textRT.getw() / loc.getw() + 1;
		float h = textRT.geth();
		if (n > 0) {
			textRT.moveValue("fx", loc.fx);
		}
		else {
			//x 정렬
			if (tss.positioning.x > 0) {
				textRT.moveValue("lx", loc.lx);
			}
			else if (tss.positioning.x == 0) {
				textRT.moveValue("cx", loc.getCenter().x);
			}
			else {
				textRT.moveValue("fx", loc.fx);
			}
		}

		if (tss.positioning.y > 0) {
			textRT.moveValue("ly", loc.ly - (n)*h);
		}
		else if (tss.positioning.y == 0) {
			textRT.moveValue("cy", loc.getCenter().y - (n)*h / 2 + h / 2);
		}
		else {
			textRT.moveValue("fy", loc.fy);
		}

		limitLine = n;

		if (bDebug) {
			//입력된 범위 표시
			glColor4f(1, 0, 0, 1);
			glBegin(GL_LINE_LOOP);
			glVertex2f(loc.fx, loc.fy);
			glVertex2f(loc.lx, loc.fy);
			glVertex2f(loc.lx, loc.ly);
			glVertex2f(loc.fx, loc.ly);
			glEnd();

			//실제 들어갈 수 있는 범위 표시
			glColor4f(0, 1, 0, 1);
			glBegin(GL_LINE_LOOP);
			glVertex2f(textRT.fx, textRT.fy);
			glVertex2f(textRT.lx, textRT.fy);
			glVertex2f(textRT.lx, textRT.ly);
			glVertex2f(textRT.fx, textRT.ly);
			glEnd();

			//실제 들어갈 수 있는 범위 표시
			glColor4f(0, 1, 0, 1);
			glBegin(GL_LINE_LOOP);
			glVertex2f(loc.fx, textRT.fy);
			glVertex2f(loc.lx, textRT.fy);
			glVertex2f(loc.lx, textRT.ly + n * h);
			glVertex2f(loc.fx, textRT.ly + n * h);
			glEnd();
		}
	}

	//그리기 초기 좌표 지정
	penx = textRT.fx;
	peny = textRT.ly;

	int drawLine = 0;
	int addx = 0;
	//glColor4f(color.r, color.g, color.b, color.a);
	if (tss.OutHide) {
		for (idx = 0; idx < lstrlen(str); idx++) {
			FT_Load_Char(face, str[idx], FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
			int wid = face->glyph->bitmap.width;
			int hei = face->glyph->bitmap.rows;
			int left, top;
			left = face->glyph->bitmap_left;
			top = face->glyph->bitmap_top;

			shp::vec2f p1 = shp::vec2f(penx + left, peny - top);
			shp::vec2f p2 = shp::vec2f(penx + left + wid, peny - top + hei);
			shp::rect4f prt = shp::rect4f(penx + left + wid, peny - top + hei - 1, penx + left + wid + 1, peny - top + hei);
			if (shp::bRectInRectRange(prt, loc, true, false) == false) {
				if (tss.drawNextLines) {
					drawLine++;
					if (drawLine < limitLine - 1) {
						penx = textRT.fx;
						peny = peny + textRT.geth();
					}
					else if (drawLine == limitLine - 1) {
						float f = textRT.getw() - addx;
						if (tss.positioning.x > 0) {
							penx = loc.lx - f;
						}
						else if (tss.positioning.x == 0) {
							penx = loc.getCenter().x - f / 2;
						}
						else {
							penx = loc.fx;
						}

						peny = peny + textRT.geth();
					}
				}
				else {
					penx += face->glyph->advance.x / 64;
					peny += face->glyph->advance.y / 64;
					addx += face->glyph->advance.x / 64;
					continue;
				}
			}

			//use Shader
			glUseProgram(textShaderID);

			//uniform projection
			unsigned int uid = glGetUniformLocation(textShaderID, "ScreenSiz");
			if (uid != -1) {
				glUniform2f(uid, maxW, maxH);
			}

			//uniform color
			uid = glGetUniformLocation(textShaderID, "textColor");
			if (uid != -1) {
				glUniform3f(uid, color.r, color.g, color.b);
			}

			glLineWidth(1);
			//create array

			for (int c = 0; c < face->glyph->outline.n_contours; c++) {
				int cs = (c == 0 ? 0 : face->glyph->outline.contours[c - 1] + 1);
				int ce = face->glyph->outline.contours[c];
				//glBegin(GL_LINE_LOOP);
				size_t posSiz = ce - cs + 1;
				glm::vec3* drawArr = new glm::vec3[posSiz];

				for (int p = cs; p <= ce; p++) {
					FT_Vector pt = face->glyph->outline.points[p];
					x = pt.x >> 6;
					y = pt.y >> 6;
					drawArr[p - cs] = glm::vec3(penx + x, peny - y, 1);
					//glVertex2f(penx + x, peny - y);
				}

				glGenVertexArrays(1, &textVAO);
				glGenBuffers(1, &textVBO);

				glBindVertexArray(textVAO);


				glBindBuffer(GL_ARRAY_BUFFER, textVBO);
				glBufferData(GL_ARRAY_BUFFER, posSiz * 3 * sizeof(GLfloat), drawArr, GL_STATIC_DRAW);

				// position attribute
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

				glBindVertexArray(textVAO);
				glDrawArrays(GL_LINE_LOOP, 0, posSiz);

				//glDeleteVertexArrays(posSiz * 3 * sizeof(GLfloat), &textVAO);
				delete[] drawArr;
				glDeleteBuffers(1, &textVBO);
				glDeleteVertexArrays(1, &textVAO);
				//glEnd();
			}

			//arrayDraw

			penx += face->glyph->advance.x / 64;
			peny += face->glyph->advance.y;
			addx += face->glyph->advance.x / 64;
		}
	}
	else {
		for (idx = 0; idx < lstrlen(str); idx++) {
			FT_Load_Char(face, str[idx], FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);

			int wid = face->glyph->bitmap.width;
			int hei = face->glyph->bitmap.rows;
			int left, top;
			left = face->glyph->bitmap_left;
			top = face->glyph->bitmap_top;
			if (tss.drawNextLines) {
				shp::vec2f p1 = shp::vec2f(penx + left, peny - top);
				shp::vec2f p2 = shp::vec2f(penx + left + wid, peny - top + hei);
				shp::rect4f prt = shp::rect4f(penx + left + wid, peny - top + hei - 1, penx + left + wid + 1, peny - top + hei);
				if (shp::bRectInRectRange(prt, loc, true, false) == false) {
					if (drawLine < limitLine - 1) {
						penx = textRT.fx;
						peny = peny + textRT.geth();
					}
					else if (drawLine == limitLine - 1) {
						float f = textRT.getw() - addx;
						if (tss.positioning.x > 0) {
							penx = loc.lx - f;
						}
						else if (tss.positioning.x == 0) {
							penx = loc.getCenter().x - f / 2;
						}
						else {
							penx = loc.fx;
						}

						peny = peny + textRT.geth();
					}
					drawLine++;
				}
			}

			for (int c = 0; c < face->glyph->outline.n_contours; c++) {
				int cs = (c == 0 ? 0 : face->glyph->outline.contours[c - 1] + 1);
				int ce = face->glyph->outline.contours[c];
				glBegin(GL_LINE_LOOP);
				for (int p = cs; p <= ce; p++) {
					FT_Vector pt = face->glyph->outline.points[p];
					x = pt.x >> 6;
					y = pt.y >> 6;
					glVertex2f(penx + x, peny - y);
				}
				glEnd();
			}

			penx += face->glyph->advance.x / 64;
			peny += face->glyph->advance.y;
			addx += face->glyph->advance.x / 64;
		}
	}

	FT_Done_Face(face);
}

bool objD = false;
void DoDisplay() {
	if (disableDisplay) return;

	ShowCursor(FALSE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//glFrontFace(GL_CW);
	skybox.Draw();
	for (int i = 0; i < lightSys.pointlight_up; ++i) {
		LightSource[i].Draw();
	}

	gameManager.Render();

	if (gameManager.gameMode != GameMode::PlayMode) {
		xcoord.Draw();
		ycoord.Draw();
		zcoord.Draw();
	}


	//glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glFlush();

	SwapBuffers(hdc);
}