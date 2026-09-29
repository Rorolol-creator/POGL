#pragma once 

#include <vector>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "tiny_obj_loader.h"

#include "face.hh"
#include "material.hh"

void load_obj( const char *filename, const char *mtl_basedir, std::vector<glm::vec3> &vertices, std::vector<Triangle> &triangles, std::vector<glm::vec3> & vertnormals, std::vector<glm::vec2>& vertuvs, std::vector<Material>& materials);

