#pragma once

#include <string>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "CImg.h"

class Material {
private:
	glm::vec3 diffuse_colour; //r, g, b albedo values
	bool has_texture;
	std::string diffuse_texture_file; //path to diffuse texture
	cimg_library::CImg<float> diffuse_texture;
    GLuint texture_id = 0; 

public:
	Material(Material&& other) = default;
	Material(const Material& other) 
    : diffuse_colour(other.diffuse_colour),
      has_texture(other.has_texture),
      diffuse_texture_file(other.diffuse_texture_file),
      diffuse_texture(other.has_texture ? cimg_library::CImg<float>(other.diffuse_texture_file.c_str()) : cimg_library::CImg<float>()),
      texture_id(other.texture_id)
	{
    	if (has_texture)
        	diffuse_texture.normalize(0.f, 1.f);
	}
	Material(const glm::vec3 & dc) : diffuse_colour(dc), has_texture(false) {}
	Material(const glm::vec3 & dc, const std::string & dtf) : diffuse_colour(dc), has_texture(true), diffuse_texture_file(dtf), diffuse_texture(diffuse_texture_file.c_str()) {
		std::cout << "Loaded texture " << diffuse_texture_file << "." << std::endl;
		diffuse_texture.normalize(0.f,1.f);
	}

	void upload_texture();
    GLuint get_texture_id() const { return texture_id; }
    bool has_tex() const { return has_texture; }
	glm::vec3 sample(const glm::vec2 & uv) const;
};

