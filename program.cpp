#include "program.hh"
#include <string>

std::string program::load(const std::string& filename)
{
    std::ifstream input_src_file(filename, std::ios::in);
    std::string ligne;
    std::string file_content = "";
    if (input_src_file.fail())
    {
        std::cerr << "FAILURE: can not load " << filename << "\n";
        return "";
    }
    while (getline(input_src_file, ligne))
    {
        file_content = file_content + ligne + "\n";
    }
    file_content += '\0';
    input_src_file.close();
    return file_content;
}

bool program::load_compile_shader(const GLenum shader_type,
                                  const std::string& shader_src_filename,
                                  GLuint& shader_id)
{
    logs += "Loading " + shader_src_filename + "\n";
    GLint compile_status = GL_TRUE;
    std::string shader_src = load(shader_src_filename);
    const GLchar* sources[1];
    sources[0] = shader_src.c_str();
    shader_id = glCreateShader(shader_type);
    logs += "Created shader " + std::to_string(shader_id) + "\n";
    TEST_OPENGL_ERROR();
    glShaderSource(shader_id, 1, sources, 0);
    logs += "Sourced shader " + std::to_string(shader_id) + "\n";
    TEST_OPENGL_ERROR();
    glCompileShader(shader_id);
    logs += "Compiled shader " + std::to_string(shader_id) + "\n";
    TEST_OPENGL_ERROR();
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);
    logs += "Got shader " + std::to_string(shader_id) + "\n";
    if (compile_status != GL_TRUE)
    {
        logs += "Compile status was false \n";
        GLint log_size;
        char* shader_log;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_size);
        shader_log = (char*)std::malloc(
            log_size + 1); /* +1 pour le caractere de fin de chaine '\0' */
        if (shader_log != 0)
        {
            glGetShaderInfoLog(shader_id, log_size, &log_size, shader_log);
            logs += "FAILURE could not compile " + shader_src_filename
                + "\nshader_log: " + shader_log + "\n";
            std::cerr << "FAILURE can not compile shader "
                      << shader_src_filename << ": " << shader_log << std::endl;
            std::free(shader_log);
        }
        glDeleteShader(shader_id);
        return false;
    }
    return true;
}

bool program::attach_and_link_program(const std::vector<GLuint>& shaders_id,
                                      GLuint& program_id)
{
    GLint link_status = GL_TRUE;
    program_id = glCreateProgram();
    logs += "created program" + std::to_string(program_id) + "\n";
    TEST_OPENGL_ERROR();
    if (program_id == 0)
        return false;
    for (unsigned int i = 0; i < shaders_id.size(); i++)
    {
        glAttachShader(program_id, shaders_id[i]);
        logs += "attached shader " + std::to_string(shaders_id[i]);
        TEST_OPENGL_ERROR();
    }
    glLinkProgram(program_id);
    logs += "linked program" + std::to_string(program_id) + "\n";
    TEST_OPENGL_ERROR();
    glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
    logs += "got program" + std::to_string(program_id) + "\n";
    if (link_status != GL_TRUE)
    {
        logs += "link status was false\n";
        GLint log_size;
        char* program_log;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_size);
        program_log = (char*)std::malloc(
            log_size + 1); /* +1 pour le caractere de fin de chaine '\0' */
        if (program_log != 0)
        {
            glGetProgramInfoLog(program_id, log_size, &log_size, program_log);
            logs += "FAILURE could not link program "
                + std::to_string(program_id) + "\nshader_log: " + program_log
                + "\n";
            std::cerr << "FAILURE: Program can not be linked " << program_log
                      << std::endl;
            std::free(program_log);
        }
        for (unsigned int i = 0; i < shaders_id.size(); i++)
        {
            glDetachShader(program_id, shaders_id[i]);
            logs += "detached shader" + std::to_string(shaders_id[i]) + "\n";
            TEST_OPENGL_ERROR();
        }
        glDeleteProgram(program_id);
        logs += "deleted program" + std::to_string(program_id) + "\n";
        TEST_OPENGL_ERROR();
        program_id = 0;
        return false;
    }
    // glUseProgram(program_id);TEST_OPENGL_ERROR();
    return true;
}

bool program::do_stuff(const std::string& vertex_shader_src,
                       const std::string& fragment_shader)
{
    GLuint vertex_id, fragment_id;
    if (!load_compile_shader(GL_VERTEX_SHADER, vertex_shader_src, vertex_id))
    {
        return false;
    }
    if (!load_compile_shader(GL_FRAGMENT_SHADER, fragment_shader, fragment_id))
    {
        return false;
    }
    std::vector<GLuint> shaders_id;
    shaders_id.push_back(vertex_id);
    shaders_id.push_back(fragment_id);
    if (!attach_and_link_program(shaders_id, program_id))
    {
        for (unsigned int i = 0; i < shaders_id.size(); i++)
        {
            glDeleteShader(shaders_id[i]);
            TEST_OPENGL_ERROR();
        }
        return false;
    }

    for (unsigned int i = 0; i < shaders_id.size(); i++)
    {
        glDetachShader(program_id, shaders_id[i]);
        TEST_OPENGL_ERROR();
    }

    for (unsigned int i = 0; i < shaders_id.size(); i++)
    {
        glDeleteShader(shaders_id[i]);
        TEST_OPENGL_ERROR();
    }
    return true;
}

bool program::is_ready()
{
    return ready;
}

const char* program::get_log()
{
    return logs.c_str();
}

void program::use()
{
    if (ready)
        glUseProgram(program_id);
}

program* program::make_program(const std::string& vertex_shader_src,
                               const std::string& fragment_shader_src)
{
    auto prog = new program();
    prog->ready = prog->do_stuff(vertex_shader_src, fragment_shader_src);
    if (!prog->ready)
        std::cout << prog->get_log();
    return prog;
}
