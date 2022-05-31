#pragma once

#include "mesh2.h"

mesh2 gen_mesh_floor(void);
mesh2 gen_mesh_circle(const float radius, unsigned int num_segments);
mesh2 gen_mesh_obj(const char * path, const char* pathTex, glm::vec3 position, glm::vec3 size, bool collisionEnable);
void GenerateMap(std::vector<mesh2>& mapMesh, glm::vec2& start_pos, glm::vec2& end_pos, std::string mapPath, const char* pathTex);
