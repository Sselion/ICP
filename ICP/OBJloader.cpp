#include <string>
#include <GL/glew.h> 
#include <glm/glm.hpp>

#include "OBJloader.h"
#include "mesh2.h"
#define array_cnt(a) ((unsigned int)(sizeof(a)/sizeof(a[0])))

bool loadOBJ(const char * path, std::vector<vertex>& out_vertex, std::vector<GLuint>& out_indices 
/*std::vector < glm::vec3 >& out_vertices, std::vector < glm::vec2 >& out_uvs, std::vector < glm::vec3 >& out_normals*/)
{
	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	out_vertex.clear();
	out_indices.clear();
	//out_vertices.clear();
	//out_uvs.clear();
	//out_normals.clear();

	FILE * file;
	fopen_s(&file, path, "r");
	if (file == NULL) {
		printf("Impossible to open the object file !\n");
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf_s(file, "%s", lineHeader, array_cnt(lineHeader));
		if (res == EOF) {
			break;
		}

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf_s(file, "%f %f\n", &uv.y, &uv.x);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by simple parser :( Try exporting with other options\n");
				return false;
			}


			glm::vec3 temp_vert = temp_vertices[vertexIndex[0] - 1];
			glm::vec2 temp_uv = temp_uvs[uvIndex[0] - 1];
			out_vertex.push_back({ temp_vert, temp_uv });
			out_indices.push_back(out_vertex.size()-1);

			temp_vert = temp_vertices[vertexIndex[1] - 1];
			temp_uv = temp_uvs[uvIndex[1] - 1];
			out_vertex.push_back({ temp_vert, temp_uv });
			out_indices.push_back(out_vertex.size() - 1);

			temp_vert = temp_vertices[vertexIndex[2] - 1];
			temp_uv = temp_uvs[uvIndex[2] - 1];
			out_vertex.push_back({ temp_vert, temp_uv });
			out_indices.push_back(out_vertex.size() - 1);

	/*		vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);*/
		}
	}

	//// unroll from indirect to direct vertex specification
	//// not necessary, not optimal

	//for (unsigned int u = 0; u < vertexIndices.size(); u++) {
	//	unsigned int vertexIndex = vertexIndices[u];
	//	glm::vec3 vertex = temp_vertices[vertexIndex - 1];
	//	out_vertices.push_back(vertex);
	//}
	//for (unsigned int u = 0; u < uvIndices.size(); u++) {
	//	unsigned int uvIndex = uvIndices[u];
	//	glm::vec2 uv = temp_uvs[uvIndex - 1];
	//	out_uvs.push_back(uv);
	//}
	//for (unsigned int u = 0; u < normalIndices.size(); u++) {
	//	unsigned int normalIndex = normalIndices[u];
	//	glm::vec3 normal = temp_normals[normalIndex - 1];
	//	out_normals.push_back(normal);
	//}

	fclose(file);
	return true;
}