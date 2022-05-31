// mesh.cpp 
// Author: JJ
#include <GL/glew.h> 
#include <GL/wglew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh.h"
#include "texture.h"
#include "OBJloader.h"




bool loadOBJ(mesh & out_mesh, std::string obj_path)
{
	bool res;

	if (obj_path.size() == 0)
		return false;
	
	out_mesh.clear();
	out_mesh.primitive_type = GL_TRIANGLES;
	
	res = loadOBJ(obj_path.c_str(), out_mesh.vertices);
	if (!res)
		return false;

	if (out_mesh.vertices.size() > 0)
	{
		out_mesh.normals_used = true;
		out_mesh.texture_used = true;
	}
	out_mesh.init();
	return true;
}

bool loadTexture(mesh & out_mesh, std::string texture_path)
{
	if (texture_path.size() == 0)
	{
		out_mesh.texture_used = 0;
		return true;
	}

	out_mesh.textureID = textureInit(texture_path.c_str(), false, false);
	out_mesh.texture_used = true;
	
	return true;
}

bool mesh_init(mesh & out_mesh, std::string obj_path, std::string texture_path)
{
	if (!loadOBJ(out_mesh, obj_path))
		return false;
	if (!loadTexture(out_mesh, texture_path))
		return false;
	return true;
}

// complex method

//void mesh_draw(mesh & mesh)
//{
//	if (mesh.vertices.size() == 0)
//		return;
//
//	glVertexPointer(3, GL_FLOAT, 0, mesh.vertices.data());
//	glEnableClientState(GL_VERTEX_ARRAY);
//
//	if (mesh.normals_used)
//	{
//		glNormalPointer(GL_FLOAT, 0, mesh.normals.data());
//		glEnableClientState(GL_NORMAL_ARRAY);
//	}
//
//	if (mesh.texture_used)
//	{
//		//glEnable(GL_TEXTURE_2D);
//		//GLuint boundTexture = 0;
//		//glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture);
//		//glBindTexture(GL_TEXTURE_2D, mesh.textureID);
//		//glTexCoordPointer(2, GL_FLOAT, 0, mesh.texcoords.data());
//		//glBindTexture(GL_TEXTURE_2D, boundTexture);
//		//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//
//		glEnable(GL_TEXTURE_2D);
//		// now set the sampler to the correct texture unit
//		//glUniform1i(glGetUniformLocation(shader.ID, "texture_diffuse1"), 0);
//		glBindTexture(GL_TEXTURE_2D, mesh.textureID);
//		glTexCoordPointer(2, GL_FLOAT, 0, mesh.texcoords.data());
//		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	}
//	else
//	{
//		glDisable(GL_TEXTURE_2D);
//	}
//
//	if (mesh.colors_used)
//	{
//		glColorPointer(4, GL_FLOAT, 0, mesh.colors.data());
//		glEnableClientState(GL_COLOR_ARRAY);
//	}
//
//	// draw batch 
//	glDrawArrays(mesh.primitive_type, 0, mesh.vertices.size());
//
//	glDisableClientState(GL_VERTEX_ARRAY);
//	if (mesh.normals_used)
//		glDisableClientState(GL_NORMAL_ARRAY);
//	if (mesh.texture_used)
//		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//	if (mesh.colors_used)
//		glDisableClientState(GL_COLOR_ARRAY);
//}

void mesh2_draw(mesh& mesh)
{
	if (mesh.textured)
		glBindTexture(GL_TEXTURE_2D, mesh.textureID);

	mesh.mesh_VAO.bind();
	glDrawElements(mesh.primitive_type, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	mesh.mesh_VAO.unbind();

}
void mesh::move_texture_points(float x, float y) {

	for (int i = 0; i < this->vertices.size(); i++) {
		this->vertices[i].texcoord[0] += x;
		this->vertices[i].texcoord[1] += y;
	}
}

void mesh::translate(float x, float z, float y) {
	for (int i = 0; i < this->vertices.size(); i++) {
		this->vertices[i].position[0] += x;
		this->vertices[i].position[1] += y;
		this->vertices[i].position[2] += z;
	}
}

void mesh::setTexture(GLuint& texId) {
	this->texture_used = true;
	this->textureID = texId;
}