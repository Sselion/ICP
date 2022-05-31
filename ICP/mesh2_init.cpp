#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "OBJloader.h"
#include "mesh2.h"
#include "mesh2_init.h"
#include "texture.h"

float size = 1.0;

mesh2 gen_mesh_floor(void)
{
	mesh2 tmpmesh;
	std::vector<vertex> vertices = {
	{glm::vec3(10, -1, 10),glm::vec2(0.0f, 0.0f)},
	{glm::vec3(10, -1, -10),glm::vec2(1.0f, 0.0f)},
	{glm::vec3(-10, -1, -10),glm::vec2(1.0f, 1.0f)},
	{glm::vec3(-10, -1, 10),glm::vec2(0.0f, 1.0f)}
	};
	std::vector<GLuint> indices = { 0,1,2,0,2,3 };
	
	cv::Mat image = cv::imread("resources/TextureDouble_A.png", cv::IMREAD_UNCHANGED);
	tmpmesh.init(GL_TRIANGLES, vertices, indices, image);

	return tmpmesh;
}

mesh2 gen_mesh_circle(const float radius, unsigned int num_segments)
{
	mesh2 tmpmesh;
	float theta;
	vertex tmpvertex;

    if (num_segments < 3)
        num_segments = 3;

	tmpvertex.texcoord = glm::vec2(0.0f);

	for (unsigned int u = 0; u < num_segments; u++)
	{
		theta = 2.0f * glm::pi<float>() * u / num_segments;

		tmpvertex.position.x = radius * cosf(theta);
		tmpvertex.position.y = radius * sinf(theta);
		tmpvertex.position.z = 0.0;

		tmpmesh.vertices.push_back(tmpvertex);
		tmpmesh.indices.push_back(u);
	}

	tmpmesh.primitive_type = GL_LINE_LOOP;
	tmpmesh.textured = false;
	tmpmesh.init();

	return tmpmesh;
}

mesh2 gen_mesh_obj(const char* path, const char* pathTex, glm::vec3 position, glm::vec3 size, bool collisionEnable) {
	mesh2 tmpmesh;
	tmpmesh.center = position;
	tmpmesh.size = size;
	tmpmesh.collisionEnable = collisionEnable;

	std::vector<vertex> vertexies;
	std::vector<GLuint> indices;

	loadOBJ(path, vertexies, indices);
	cv::Mat image = cv::imread(pathTex, cv::IMREAD_UNCHANGED);
	tmpmesh.init(GL_TRIANGLES, vertexies, indices, image);

	return tmpmesh;

}

mesh2 gen_mesh_obj(const char* path, GLuint texid, glm::vec3 position, glm::vec3 size, bool collisionEnable) {
	mesh2 tmpmesh;
	tmpmesh.center = position;
	tmpmesh.size = size;
	tmpmesh.collisionEnable = collisionEnable;
	std::vector<vertex> vertexies;
	std::vector<GLuint> indices;

	loadOBJ(path, vertexies, indices);
	tmpmesh.init(GL_TRIANGLES, vertexies, indices, texid);

	return tmpmesh;

}

/// <summary>
/// 255 - Stone
/// 200 - Trees
/// 160 - House - wall
/// 60 - House - door
/// 120 - Path
/// 10 - Start
/// 11 - End
/// </summary>
/// <param name="map"></param>
void GenerateMap(std::vector<mesh2>& mapMesh, glm::vec2& start_pos, glm::vec2& end_pos, std::string mapPath, const char* pathTex) {
	std::vector<cv::Mat> components;
	cv::Mat map = cv::imread(mapPath);
	cv::split(map, components);

	mesh2 mapPart;
	mesh2 trees;
	int y, x;
	float z = 0;
	cv::Mat image = cv::imread(pathTex, cv::IMREAD_UNCHANGED);
	GLuint texid = tex_gen(image);
	//process pixels
	for (y = 0; y < map.cols; y++)
	{
		for (x = 0; x < map.rows; x++)
		{
			mapPart = gen_mesh_obj("resources/cube.obj", texid, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.25, 0.25, 0.25), false);

			switch (components[0].at<uchar>(y, x))
			{
			case 255: // 255 - Stone
				mapPart.translate_texcoord(0.5, 0.5);
				z = 0.5;
				break;
			case 200: // 200 - Trees
				trees = gen_mesh_obj("resources/trees.obj", texid, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.5, 0.5, 0.5), true);
				z = 1;
				trees.translate_texcoord(0.5, 0.0);
				trees.translate(glm::vec3(size * x, size * z, size * y));

				trees.init();

				mapMesh.push_back(trees);
				mapPart.translate_texcoord(0.25, 0.5);
				z = -0.5;
				break;
			case 160:// 160 - House - wall
				mapPart.collisionEnable = true;
				mapPart.translate_texcoord(0.75, 0.0);
				z = 0.5;
				break;
			case 100:// 121 - House - window
				mapPart.collisionEnable = true;
				mapPart.translate_texcoord(0.25, 0.0);
				z = 0.5;
				break;
			case 60: // 60 - House - door
				mapPart.collisionEnable = true;
				mapPart.translate_texcoord(0.5, 0.0);
				z = 0.5;
				break;
			case 120: // 120 - Path
				mapPart.translate_texcoord(0.25 * (std::rand() % (3 + 1)), 0.25);
				z = -0.5;
				break;
			case 10: // 10 - Start
				mapPart.translate_texcoord(0.5, 0.75);
				start_pos = glm::vec2(x,y);
				z = 0.5;
				break;
			case 11: // 11 - End
				mapPart.translate_texcoord(0.25, 0.75);
				end_pos = glm::vec2(x, y);
				z = 0.5;
				break;
			}

			mapPart.translate(glm::vec3(size * x, size * z, size * y));
			//mapPart.mesh_init_buffers();
			mapPart.init();

			mapMesh.push_back(mapPart);
		}
	}
}

