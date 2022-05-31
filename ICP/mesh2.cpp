// mesh.cpp 
// Author: JJ
#include <iostream>

#include <GL/glew.h> 
#include <GL/wglew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh2.h"
#include "texture.h"

void mesh2::init(GLenum primitive_type, std::vector<vertex>& vertices, std::vector<GLuint>& indices, cv::Mat& teximage)
{
	this->primitive_type = primitive_type;
	this->textured = true;
	this->vertices = vertices;
	this->indices = indices;
	this->tex_ID = tex_gen(teximage);

	init();
}

void mesh2::init(GLenum primitive_type, std::vector<vertex>& vertices, std::vector<GLuint>& indices, GLuint texid)
{
	this->primitive_type = primitive_type;
	this->textured = true;
	this->vertices = vertices;
	this->indices = indices;
	this->tex_ID = texid;

	init();
}
void mesh2::scale(glm::vec3 sc) {
	this->size *= sc;
	for (int i = 0; i < this->vertices.size(); i++)
	{
		vertices[i].position.x *= sc.x;
		vertices[i].position.y *= sc.y;
		vertices[i].position.z *= sc.z;
	}
}
void mesh2::translate(glm::vec3 sc) {
	this->center += sc;
	for (int i = 0; i < this->vertices.size(); i++)
	{
		vertices[i].position.x += sc.x;
		vertices[i].position.y += sc.y;
		vertices[i].position.z += sc.z;
	}
}
void mesh2::translate_texcoord(float x, float y) {
	for (int i = 0; i < this->vertices.size(); i++)
	{
		this->vertices[i].texcoord[0] += x;
		this->vertices[i].texcoord[1] += y;
	}
}

bool collision_detect(mesh2 objA, mesh2 objB) {
	// collision x-axis?
	bool collisionX = objA.center.x + objA.size.x >= objB.center.x - objB.size.x &&
		objA.center.x - objA.size.x <= objB.center.x + objB.size.x;
	// collision y-axis?
	bool collisionZ = objA.center.z + objA.size.z >= objB.center.z - objB.size.z &&
		objA.center.z - objA.size.z <= objB.center.z + objB.size.z;
	// collision only if on both axes
	return collisionX && collisionZ;
}

void mesh2_draw(mesh2& mesh)
{
	if (mesh.textured)
		glBindTexture(GL_TEXTURE_2D, mesh.tex_ID);

	mesh.mesh_VAO.bind();
	glDrawElements(mesh.primitive_type, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	mesh.mesh_VAO.unbind();

}

void mesh2::print_VertexData() {
	for (int i = 0; i < this->vertices.size(); i++)
	{
		std::cout << "vertex " << i << " :" << this->vertices[i].position.x <<" " << this->vertices[i].position.y << " " << this->vertices[i].position.z <<"t:" << this->vertices[i].texcoord.x << " " << this->vertices[i].texcoord.y << "\n";
			
	}
	std::cout << "indeces:  ";
	for (int i = 0; i < this->indices.size(); i++)
	{
		std::cout << this->indices[i] << " ";

	}
	std::cout << "\n";

}
