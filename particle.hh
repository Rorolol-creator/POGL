#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <algorithm>
#include "matrix4.hh"
#define RED 0
#define GREEN 1
#define BLUE 2
#define ALPHA 3

// WIP TODO

namespace particle
{
    struct Particle
    {
        std::vector<GLfloat> Position, Velocity; // vec3
        std::vector<GLfloat> Color; // vec4 (rgba)
        float Life; // death timer
        float Size;
        float DistanceToCam;
        float MaxLife;

        Particle()
            : Life(0.0f)
            , Size(1.0f)
            , DistanceToCam(0.0f)
            , MaxLife(0.0f)
        {
            Position = { 0.0f, 0.0f, 0.0f };
            Velocity = { 0.0f, 1.0f, 0.0f };
            Color = { 255, 255, 255, 255 };
        }

        bool operator<(Particle& other)
        {

            return this->DistanceToCam > other.DistanceToCam;
        }
    };

    class Particles
    {
    public:
        static float time;
        static bool running;
        static bool on;
        static const int MaxParticles = 100000;
        static float delta;
        static int particles_per_frame;
        static unsigned int lastUsedParticle;
        static Particle particles[MaxParticles];
        static GLfloat g_particule_position_size_data[4 * MaxParticles];
        static GLfloat g_particule_color_data[4 * MaxParticles];
        static int ParticlesCount;
        // int newparticles = (int)(deltaTime * 10000.0);
        static int FirstUnusedParticle();
        static void main_particle(float x = 0, float y = 0, float z = 0);
        static void update();
        static void SortParticles();
    };
} // namespace particle
