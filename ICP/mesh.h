#pragma once
// OpenCV 
#include <opencv2\opencv.hpp>

#include <vector>
#include <string>

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders.h"

// vertex definition
struct Vertex_old {
	glm::vec3 position;
	glm::vec2 texcoord;
	glm::vec3 normals;
};

// classes necessary for mesh2 construction
class VBO_old {
public:
	GLuint ID;

	VBO_old(std::vector<Vertex_old>& vertices) {
		glGenBuffers(1, &ID);
		glBindBuffer(GL_ARRAY_BUFFER, ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex_old), vertices.data(), GL_STATIC_DRAW);
	};

	void bind(void) { glBindBuffer(GL_ARRAY_BUFFER, ID); }
	void unbind(void) { glBindBuffer(GL_ARRAY_BUFFER, 0); }
	void clear(void) { glDeleteBuffers(1, &ID); }
};

class EBO_old
{
public:
	GLuint ID;

	EBO_old(std::vector<GLuint> indices) {
		glGenBuffers(1, &ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	};

	void bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); };
	void unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); };
	void clear() { glDeleteBuffers(1, &ID); };
};

class VAO_old {
public:
	GLuint ID;

	VAO_old(void) :ID(0) {};

	void init(void) { glGenVertexArrays(1, &ID); }

	// connect VBO Attribute to VAO
	void connect_attrib(VBO_old& VBO_old, GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset) {
		VBO_old.bind();
		glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
		glEnableVertexAttribArray(layout);
		VBO_old.unbind();
	}
	void bind(void) { glBindVertexArray(ID); }
	void unbind(void) { glBindVertexArray(0); }
	void clear(void) { glDeleteVertexArrays(1, &ID); }
};

class mesh
{
public:
	std::vector<Vertex_old> vertices;
	std::vector<GLuint> indices;
	VAO_old mesh_VAO;
	GLuint textureID = 0;

    bool indirect = false; 
	bool texture_used = false;
	bool normals_used = false;
	bool colors_used = false;

	GLenum primitive_type = GL_POINTS;
	bool textured = false;

	mesh() = default;

	void move_texture_points(float x, float y);
	void translate(float x, float y, float z);
	void setTexture(GLuint& texId);

	void init(void)
	{
		mesh_VAO.init();
		mesh_VAO.bind();

		VBO_old mesh_VBO(vertices);
		EBO_old mesh_EBO(indices);

		mesh_VAO.connect_attrib(mesh_VBO, 0, 3, GL_FLOAT, sizeof(Vertex_old), (void*)(0));
		if (textured)
			mesh_VAO.connect_attrib(mesh_VBO, 1, 2, GL_FLOAT, sizeof(Vertex_old), (void*)(0 + offsetof(Vertex_old, texcoord)));

		mesh_VAO.unbind();
		mesh_VBO.unbind();
		mesh_EBO.unbind();
	}
	void init(GLenum primitive_type, std::vector<Vertex_old>& vertices, std::vector<GLuint>& indices)
	{
		this->primitive_type = primitive_type;
		this->textured = false;
		this->vertices = vertices;
		this->indices = indices;

		init();
	}
	void init(GLenum primitive_type, std::vector<Vertex_old>& vertices, std::vector<GLuint>& indices, cv::Mat& teximage);


	void clear(void) {
		vertices.clear();
		indices.clear();
		primitive_type = GL_POINTS;
		if (textured)
		{
			textured = false;
			glDeleteTextures(1, &textureID);
			textureID = 0;
		}
		mesh_VAO.clear();
	}

	//~mesh() { if (texture_used) glDeleteTextures(1, &textureID); };

};

bool loadOBJ(mesh & loaded_mesh, std::string obj_path);
bool loadTexture(mesh & out_mesh, std::string texture_path);
bool mesh_init(mesh & out_mesh, std::string obj_path, std::string texture_path);

// complex
void mesh_draw(mesh & mesh);
void mesh2_draw(mesh& mesh);

