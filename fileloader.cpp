#define TINYOBJLOADER_IMPLEMENTATION
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <text/csv/istream.hpp>

//#include "light.h"
#include "fileloader.hh"
//#include "face.h"
//#include "material.h"
//#include "arguments.h"

using glm::vec2;
using glm::vec3;
using glm::uvec3;
using std::vector;

using ::text::csv::csv_istream;

void components_to_vec2s(const vector<float> components, vector<vec2>& vecs) {
    for(size_t vec_start = 0; vec_start < components.size(); vec_start+=2) {
        vecs.push_back(
            vec2(components[vec_start],
                components[vec_start+1]
            ));
    }
}

void components_to_vec3s(const vector<float> components, vector<vec3>& vecs) {
    for(size_t vec_start = 0; vec_start < components.size(); vec_start+=3) {
        vecs.push_back(
            vec3(components[vec_start],
                components[vec_start+1],
                components[vec_start+2]
            ));
    }
}

void load_materials(const vector<tinyobj::material_t> & objmaterials, const std::string & tex_dir, vector<Material> & materials) {

       materials.reserve(objmaterials.size()); // ← évite les réallocations/copies
    for (auto mat = objmaterials.begin(); mat < objmaterials.end(); ++mat) {
        vec3 diffuse_colour((*mat).diffuse[0], (*mat).diffuse[1], (*mat).diffuse[2]);
        if ((*mat).diffuse_texname.empty()) {
            materials.emplace_back(diffuse_colour);
        } else {
            materials.emplace_back(diffuse_colour, tex_dir + (*mat).diffuse_texname);
        }
    }
}

void load_triangles(const tinyobj::shape_t & shape, vector<Triangle> & triangles) {

    const vector<tinyobj::index_t> & indices = shape.mesh.indices;
    const vector<int> & mat_ids = shape.mesh.material_ids;

    std::cout << "Loading " << mat_ids.size() << " triangles..." << std::endl;

    for(size_t face_ind = 0; face_ind < mat_ids.size(); face_ind++) {
        triangles.push_back(
            Triangle(
                {indices[3*face_ind].vertex_index, indices[3*face_ind+1].vertex_index, indices[3*face_ind+2].vertex_index},
                {indices[3*face_ind].normal_index, indices[3*face_ind+1].normal_index, indices[3*face_ind+2].normal_index},
                {indices[3*face_ind].texcoord_index, indices[3*face_ind+1].texcoord_index, indices[3*face_ind+2].texcoord_index},
                mat_ids[face_ind]
                ));
    }
}

void load_obj( const char *filename, const char *mtl_basedir, vector<vec3> &vertices, vector<Triangle> &triangles, vector<vec3> & vertnormals, vector<vec2>& vertuvs, vector<Material>& materials) {

    tinyobj::attrib_t attrib;
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> objmaterials;
    std::string err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &objmaterials, &err,
        filename,
        mtl_basedir,
        true); 

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }
    if (!success) {
        exit(1);
    }

    components_to_vec3s(attrib.vertices, vertices);

    components_to_vec3s(attrib.normals, vertnormals);

    components_to_vec2s(attrib.texcoords, vertuvs);

    load_materials(objmaterials, mtl_basedir, materials);

    for(auto shape = shapes.begin(); shape < shapes.end(); ++shape) {
        load_triangles(*shape, triangles);
    }
}
