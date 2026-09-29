#pragma once

#include "iostream"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <fstream>
#include <iostream>
#include <vector>

#define TEST_OPENGL_ERROR()                                                    \
    do                                                                         \
    {                                                                          \
        GLenum err = glGetError();                                             \
        if (err != GL_NO_ERROR)                                                \
            std::cerr << "OpenGL ERROR!" << __LINE__ << "ERROR = " << err      \
                      << std::endl;                                            \
    } while (0)

class program
{
public:
    std::string logs;
    bool ready;
    GLuint program_id;
    bool do_stuff(const std::string& vertex_shader_src,
                  const std::string& fragment_shader);
    std::string load(const std::string& filename);
    bool load_compile_shader(const GLenum shader_type,
                             const std::string& shader_src_filename,
                             GLuint& shader_id);

    bool attach_and_link_program(const std::vector<GLuint>& shaders_id,
                                 GLuint& program_id);
    program()
        : logs{ "" }
    {}
    ~program();
    static program* make_program(const std::string& vertex_shader_src,
                                 const std::string& fragment_shader_src);
    const char* get_log();
    bool is_ready();
    void use();
};
