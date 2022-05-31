#pragma once
#ifndef OBJloader_H
#define OBJloader_H

#include <vector>
#include <glm/fwd.hpp>
#include "mesh2.h"

bool loadOBJ(
	const char * path,
	std::vector<vertex> & out_vertex,
	std::vector<GLuint> & out_indices
	//std::vector < glm::vec3 > & out_vertices,
	//std::vector < glm::vec2 > & out_uvs,
	//std::vector < glm::vec3 > & out_normals
);

#endif
