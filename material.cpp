#include "material.hh"

#include <stdexcept>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

using glm::vec2;
using glm::vec3;

vec3 Material::sample(const vec2 & uv) const {
	if (has_texture) {
		float u = uv.x * diffuse_texture.width();
		float v = (1.f-uv.y) * diffuse_texture.height();
		return vec3(diffuse_texture.linear_atXY(u,v,0,0),diffuse_texture.linear_atXY(u,v,0,1),diffuse_texture.linear_atXY(u,v,0,2));
	}
	else {
		return diffuse_colour;
	}
}

void Material::upload_texture() {
    if (!has_texture) return;

    int w = diffuse_texture.width();
    int h = diffuse_texture.height();
    std::vector<unsigned char> data(w * h * 3);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float u = (float)x;
            float v = (float)(h - 1 - y); 
            data[(y * w + x) * 3 + 0] = (unsigned char)(diffuse_texture.linear_atXY(u, v, 0, 0) * 255);
            data[(y * w + x) * 3 + 1] = (unsigned char)(diffuse_texture.linear_atXY(u, v, 0, 1) * 255);
            data[(y * w + x) * 3 + 2] = (unsigned char)(diffuse_texture.linear_atXY(u, v, 0, 2) * 255);
        }
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}