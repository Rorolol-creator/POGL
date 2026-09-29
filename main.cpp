#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include "matrix4.hh"
#include "program.hh"
//#include "object_vbo.hh"
#include "particle.hh"
#include <cmath>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "fileloader.hh"
#include "material.hh"

#define TEST_OPENGL_ERROR()                                                    \
    do                                                                         \
    {                                                                          \
        GLenum err = glGetError();                                             \
        if (err != GL_NO_ERROR)                                                \
            std::cerr << "OpenGL ERROR!" << __LINE__ << "ERROR = " << err      \
                      << std::endl;                                            \
    } while (0)

GLuint vao;

static const GLfloat g_vertex_buffer_data[] = {
    -0.5, -0.5, 0.0, 0.5, -0.5, 0.0, -0.5, 0.5, 0.0, 0.5, 0.5, 0.0,
};
GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;
GLint view_location;
program* prog;

std::vector<GLuint> model_texture_ids;
std::vector<GLuint> model_vaos;          
std::vector<GLuint> model_vertex_counts; 


GLuint model_vbo_normals;
glm::mat4 model_matrix;

bool init_model(GLuint program_id, std::vector<glm::vec3>& vertices,
                std::vector<glm::vec3>& normals, std::vector<glm::vec2>& uvs,
                std::vector<Triangle>& faces, std::vector<Material>& materials,
                glm::vec3 center, float size)
{
    int num_materials = materials.size();
    if (num_materials == 0) num_materials = 1;

    std::vector<std::vector<glm::vec3>> per_mat_vertices(num_materials);
    std::vector<std::vector<glm::vec2>> per_mat_uvs(num_materials);

    for (auto& tri : faces) {
        int mat_id = tri.material;
        if (mat_id < 0 || mat_id >= num_materials) mat_id = 0;

        for (int i = 0; i < 3; i++) {
            int vi = tri.vertices[i];
            int uvi = tri.uvs[i];

            if (vi < 0 || vi >= (int)vertices.size()) continue;
            per_mat_vertices[mat_id].push_back(vertices[vi]);

            if (uvi >= 0 && uvi < (int)uvs.size())
                per_mat_uvs[mat_id].push_back(uvs[uvi]);
            else
                per_mat_uvs[mat_id].push_back(glm::vec2(0, 0));
        }
    }

    for (auto& mat : materials)
        mat.upload_texture();

    model_vaos.resize(num_materials);
    model_vertex_counts.resize(num_materials);
    model_texture_ids.resize(num_materials, 0);

    GLint pos_loc = glGetAttribLocation(program_id, "position");
    GLint tex_loc = glGetAttribLocation(program_id, "tex_coord");

    for (int m = 0; m < num_materials; m++) {
        model_vertex_counts[m] = per_mat_vertices[m].size();
        if (m < (int)materials.size() && materials[m].has_tex())
            model_texture_ids[m] = materials[m].get_texture_id();

        glGenVertexArrays(1, &model_vaos[m]);
        glBindVertexArray(model_vaos[m]);

        if (pos_loc != -1 && !per_mat_vertices[m].empty()) {
            GLuint vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, per_mat_vertices[m].size() * sizeof(glm::vec3),
                         per_mat_vertices[m].data(), GL_STATIC_DRAW);
            glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(pos_loc);
        }

        if (tex_loc != -1 && !per_mat_uvs[m].empty()) {
            GLuint vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, per_mat_uvs[m].size() * sizeof(glm::vec2),
                         per_mat_uvs[m].data(), GL_STATIC_DRAW);
            glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(tex_loc);
        }

        glBindVertexArray(0);
    }

    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::scale(model_matrix, glm::vec3(2.f / size));
    model_matrix = glm::translate(model_matrix, -center);

    return true;
}
float x, y = 0;
float z = 34;
float yaw = 0.0f, pitch = 0.0f;
float lastX, lastY, flameX, flameY, flameZ = 0;
float sensitivity = 0.02;
bool blend = true;
bool depth = false;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prog->use();

    GLint is_model_loc = glGetUniformLocation(prog->program_id, "is_model");
    GLint model_loc = glGetUniformLocation(prog->program_id, "model_matrix");
    GLint tex_uniform = glGetUniformLocation(prog->program_id, "tex");

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(is_model_loc, 1);
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));

    for (int m = 0; m < (int)model_vaos.size(); m++) {
        if (model_vertex_counts[m] == 0) continue;
        glActiveTexture(GL_TEXTURE0);
        if (model_texture_ids[m] != 0)
            glBindTexture(GL_TEXTURE_2D, model_texture_ids[m]);
        glUniform1i(tex_uniform, 0);
        glBindVertexArray(model_vaos[m]);
        glDrawArrays(GL_TRIANGLES, 0, model_vertex_counts[m]);
    }
    glBindVertexArray(0);

//    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glUniform1i(is_model_loc, 0);
    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(identity));

    particle::Particles::main_particle(flameX, flameY, flameZ);
    particle::Particles::update();
    particle::Particles::SortParticles();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, particle::Particles::MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particle::Particles::ParticlesCount * sizeof(GLfloat) * 4,
                    particle::Particles::g_particule_position_size_data);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, particle::Particles::MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particle::Particles::ParticlesCount * 4 * sizeof(GLfloat),
                    particle::Particles::g_particule_color_data);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, particle::Particles::ParticlesCount);
    glBindVertexArray(0);
   // glDepthMask(GL_TRUE);

    glutSwapBuffers();
}
void updateCamera()
{
    float dirX = cos(pitch) * sin(yaw);
    float dirY = sin(pitch);
    float dirZ = cos(pitch) * (-cos(yaw));

    mygl::matrix4 new_view = mygl::matrix4::identity();
    new_view.lookAt(
        x, y, z,                   
        x + dirX, y + dirY, z + dirZ, 
        0.0, 1.0, 0.0
    );
    glUniformMatrix4fv(view_location, 1, false, new_view.mat.data());
}

void depth_undepth()
{
    if (depth)
        glDisable(GL_DEPTH_TEST);
    else
        glEnable(GL_DEPTH_TEST);
    depth = !depth;
    glutPostRedisplay();
}

void blend_unblend()
{
    if (blend)
        glDisable(GL_BLEND);
    else
        glEnable(GL_BLEND);
    blend = !blend;
    glutPostRedisplay();
}

void keyboard(unsigned char key, int _xx, int _yy)
{
    int ppf;
    float dt;
    switch (key)
    {
    case '\e': // ESC key
        glutLeaveMainLoop();
        break;
    case 'w':
    case 'z':
        //z -= 1.0;
        x += cos(pitch) * sin(yaw);
        y += sin(pitch);
        z -= cos(pitch) * cos(yaw);
        break;
    case 's':
        x -= cos(pitch) * sin(yaw);
        y -= sin(pitch);
        z += cos(pitch) * cos(yaw);
        //z += 1.0;
        break;
    case 'q':
        //x -= 1.0;
        x -= cos(yaw);
        z -= sin(yaw);
        break;
    case 'd':
        //x += 1.0;
        x += cos(yaw);
        z += sin(yaw);
        break;
    case 'r':
        yaw -= 0.1f;
        break;
    case 't':
        yaw += 0.1f;
        break;
    case 'e':
        y += 1.0;
        break;
    case 'a':
        y -= 1.0;
        break;
    case ' ':
        particle::Particles::running = !particle::Particles::running;
        break;
    case 'o':
        particle::Particles::on = !particle::Particles::on;
        break;
    case 'k':
        particle::Particles::delta *= 1.1;
        ppf = particle::Particles::particles_per_frame * 1.1;
        if (particle::Particles::particles_per_frame == ppf)
            ppf++;
        particle::Particles::particles_per_frame = ppf;
        break;
    case 'j':
        dt = particle::Particles::delta * 0.9;
        ppf = particle::Particles::particles_per_frame * 0.9;
        if (dt > 0 && ppf > 0)
        {
            particle::Particles::delta = dt;
            particle::Particles::particles_per_frame = ppf;
        }
        break;
    case 'b':
        blend_unblend();
        break;
    case 'l':
        depth_undepth();
        break;
    }
    updateCamera();
    glutPostRedisplay();
}

void mouseMotion(int xx, int yy)
{
    if (xx > lastX)
        flameX += sensitivity;
    else if (xx < lastX)
        flameX -= sensitivity;
    if (yy > lastY)
        flameY -= sensitivity;
    else if (yy < lastY)
        flameY += sensitivity;
    float dx = (xx - lastX) * 0.01f;
    float dy = (yy - lastY) * 0.01f;
    lastX = xx;
    lastY = yy;

    yaw += dx;
    pitch -= dy;
    if (pitch > 1.4f)  pitch = 1.4f;
    if (pitch < -1.4f) pitch = -1.4f;
        glutPostRedisplay();
}

void mouseWheel(int wheel, int direction, int x, int y)
{
    sensitivity *= 1.0 + (float)direction / 10;
}

bool init_glut(int& argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitContextVersion(4, 5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("Test OpenGL - POGL");
    glutDisplayFunc(display);
    glutMouseWheelFunc(mouseWheel);
    glutIdleFunc([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        glutPostRedisplay();
    });
    glutKeyboardFunc(keyboard);
    glutMotionFunc(mouseMotion);
    return true;
}

bool init_glew()
{
    return (glewInit() == GLEW_OK);
}

bool init_GL()
{
   // glEnable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    // glDepthFunc(GL_LESS);
    TEST_OPENGL_ERROR();
    // glDepthRange(0.0, 1.0);
    TEST_OPENGL_ERROR();
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    TEST_OPENGL_ERROR();
    // glEnable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    // glCullFace(GL_FRONT);
    TEST_OPENGL_ERROR();
    // glFrontFace(GL_CCW);
    TEST_OPENGL_ERROR();
    // glDepthMask(GL_FALSE);
    TEST_OPENGL_ERROR();
    glEnable(GL_BLEND);
    TEST_OPENGL_ERROR();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    TEST_OPENGL_ERROR();
    // glClearColor(1, 0.8, 0.9, 1.0);
  //  glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearColor(0.13, 0.19, 0.11, 1.0);

    TEST_OPENGL_ERROR();
    return true;
}

bool init_POV(GLuint program_id)
{
    TEST_OPENGL_ERROR();
    view_location = glGetUniformLocation(program_id, "model_view_matrix");
    TEST_OPENGL_ERROR();
    mygl::matrix4 model_view = mygl::matrix4::identity();
    model_view.lookAt(x, y, z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glUniformMatrix4fv(view_location, 1, false, model_view.mat.data());
    TEST_OPENGL_ERROR();

    GLint proj_location = glGetUniformLocation(program_id, "projection_matrix");
    TEST_OPENGL_ERROR();
    mygl::matrix4 proj = mygl::matrix4::identity();
    proj.frustrum(-1.0, 1.0, -1.0, 1.0, 5.0, 500.0);
    glUniformMatrix4fv(proj_location, 1, false, proj.mat.data());
    TEST_OPENGL_ERROR();
    return true;
}

bool init_object(GLuint program_id)
{

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLint vertex_location = glGetAttribLocation(program_id, "position");


    glGenBuffers(1, &particles_position_buffer);
    TEST_OPENGL_ERROR();

    glGenBuffers(1, &particles_color_buffer);
    TEST_OPENGL_ERROR();

    if (vertex_location != -1)
    {
        TEST_OPENGL_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER,
                     particle::Particles::MaxParticles * 4 * sizeof(GLfloat),
                     NULL, GL_STREAM_DRAW);
        TEST_OPENGL_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);

        TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER,
                     particle::Particles::MaxParticles * 4 * sizeof(GLfloat),
                     NULL, GL_STREAM_DRAW);
        TEST_OPENGL_ERROR();
        glVertexAttribPointer(
            vertex_location, 
            4, 
            GL_FLOAT, 
            GL_FALSE, 
            0, 
            (void*)0 
        );
        TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(vertex_location);
        TEST_OPENGL_ERROR();
    }

    GLint color_location = glGetAttribLocation(program_id, "color");
    TEST_OPENGL_ERROR();
    if (color_location != -1)
    {
        TEST_OPENGL_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER,
                     particle::Particles::MaxParticles * 4 * sizeof(GLfloat),
                     NULL, GL_STREAM_DRAW);
        TEST_OPENGL_ERROR();
        TEST_OPENGL_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);

        TEST_OPENGL_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER,
                     particle::Particles::MaxParticles * 4 * sizeof(GLfloat),
                     NULL, GL_STREAM_DRAW);
        TEST_OPENGL_ERROR();
        glVertexAttribPointer(
            color_location, 
            4, 
            GL_FLOAT, 
            GL_FALSE, 
            0, 
            (void*)0 
        );
        TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(color_location);
        TEST_OPENGL_ERROR();
    }

    glBindVertexArray(0);
    TEST_OPENGL_ERROR();
    return true;
}

int main(int argc, char* argv[])
{

    std::vector<glm::vec3> model_vertices;

    std::vector<glm::vec3> model_vertnormals;

    std::vector<glm::vec2> vertuvs;

    std::vector<Triangle> faces;

    std::vector<Material> materials;

    load_obj(argv[1], argv[2],model_vertices,faces,model_vertnormals, vertuvs, materials);
    glm::vec3 min_v = model_vertices[0], max_v = model_vertices[0];
    for (auto& v : model_vertices) {
        min_v = glm::min(min_v, v);
        max_v = glm::max(max_v, v);
    }

    init_glut(argc, argv);
    init_glew();
    init_GL();
    prog = program::make_program("vertex.shd", "fragment.shd");
    prog->use();
    init_object(prog->program_id);
    init_POV(prog->program_id);
    
    glm::vec3 center = (min_v + max_v) / 2.0f;
    float size = glm::length(max_v - min_v);
    init_model(prog->program_id, model_vertices, model_vertnormals, 
           vertuvs, faces, materials, center, size);
    std::cout << "vertuvs size: " << vertuvs.size() << std::endl;
    std::cout << "material size: " << materials.size() << std::endl;

    glutMainLoop();
}

