#include "particle.hh"
#include <cmath>
#include <cstdlib>
#include "perlin3D.hh"


namespace particle
{
    float Particles::delta = 0.001;
    int Particles::particles_per_frame = 40;
    float Particles::time = 0;
    bool Particles::running = true;
    bool Particles::on = true;
    unsigned int Particles::lastUsedParticle = 0;
    Particle Particles::particles[Particles::MaxParticles];

    GLfloat
        Particles::g_particule_position_size_data[4 * Particles::MaxParticles];

    GLfloat Particles::g_particule_color_data[4 * Particles::MaxParticles];
    int Particles::ParticlesCount = 0;
    int Particles::FirstUnusedParticle()
    {
        for (unsigned int i = lastUsedParticle; i < MaxParticles; ++i)
        {
            if (particles[i].Life <= 0.0f)
            {
                lastUsedParticle = i;
                return i;
            }
        }
        for (unsigned int i = 0; i < lastUsedParticle; ++i)
        {
            if (particles[i].Life <= 0.0f)
            {
                lastUsedParticle = i;
                return i;
            }
        }
        lastUsedParticle = 0;
        return -1;
    }

    void Particles::SortParticles()
    {
        std::sort(&particles[0], &particles[MaxParticles]);
    }

    void Particles::update()
    {
        if (!running)
            return;
        ParticlesCount = 0;
        for (int i = 0; i < MaxParticles; i++)
        {
            Particle& p = particles[i]; 

            if (p.Life > 0.0f)
            {
                p.Life -= delta * 2;
                if (p.Life > 0.0f)
                {
                    p.DistanceToCam = p.Position[Z_];

                    float white[3] = { 1.0f, 1.0f, 1.0f };
                    float yellow[3] = { 1.0f, 0.9f, 0.0f };
                    float red[3] = { 1.0f, 0.1f, 0.0f };
                    float black[3] = { 0.0f, 0.0f, 0.0f };
                    float life;
                    float maxi;
                    if (p.Life > 0.9 * p.MaxLife) // white yellow
                    {
                        life = p.Life - 0.9 * p.MaxLife;
                        maxi = p.MaxLife - 0.9 * p.MaxLife;
                        p.Color[RED] = white[RED] * (life / maxi)
                            + yellow[RED] * (1.0 - life / maxi);
                        p.Color[GREEN] = white[GREEN] * (life / maxi)
                            + yellow[GREEN] * (1.0 - life / maxi);
                        p.Color[BLUE] = white[BLUE] * (life / maxi)
                            + yellow[BLUE] * (1.0 - life / maxi);
                    }
                    else if (p.Life > 0.7 * p.MaxLife)
                    {
                        life = p.Life - 0.7 * p.MaxLife;
                        maxi = 0.9 * p.MaxLife - 0.7 * p.MaxLife;
                        p.Color[RED] = yellow[RED] * (life / maxi)
                            + red[RED] * (1.0 - life / maxi);
                        p.Color[GREEN] = yellow[GREEN] * (life / maxi)
                            + red[GREEN] * (1.0 - life / maxi);
                        p.Color[BLUE] = yellow[BLUE] * (life / maxi)
                            + red[BLUE] * (1.0 - life / maxi);
                    }
                    else
                    {
                        life = p.Life;
                        maxi = 0.7 * p.MaxLife;
                        p.Color[RED] = red[RED] * (life / maxi)
                            + black[RED] * (1.0 - life / maxi);
                        p.Color[GREEN] = red[GREEN] * (life / maxi)
                            + black[GREEN] * (1.0 - life / maxi);
                        p.Color[BLUE] = red[BLUE] * (life / maxi)
                            + black[BLUE] * (1.0 - life / maxi);
                    }

                    p.Velocity[Y_] *= (1.0 - delta);
                    float noise =2*fireNoise(p.Position[X_] * 0.5 + 10, p.Position[Y_] * 0.3,
                                  time, 4, 0.5, 2, 8);
                    p.Velocity[X_] *= 0.98;
                    p.Velocity[X_] += (noise - 1.f); // * delta;
                    p.Position[X_] +=
                        p.Velocity[X_] * delta; // + (rand() % 3) * delta;
                    p.Position[Y_] += p.Velocity[Y_] * delta;
                    p.Position[Z_] += p.Velocity[Z_] * delta;
                    p.Size -= delta * 10;
                    if (p.Size <= 0.0)
                        p.Life = 0;
                    // p.Color[ALPHA] += delta * 100.0f;
                    p.Color[ALPHA] = 0.9f;
                    if (p.Color[ALPHA] > 1.0)
                        p.Color[ALPHA] = 1.0;

                    g_particule_position_size_data[4 * ParticlesCount + 0] =
                        p.Position[X_];
                    g_particule_position_size_data[4 * ParticlesCount + 1] =
                        p.Position[Y_];
                    g_particule_position_size_data[4 * ParticlesCount + 2] =
                        p.Position[Z_];

                    g_particule_position_size_data[4 * ParticlesCount + 3] =
                        p.Size;

                    g_particule_color_data[4 * ParticlesCount + 0] =
                        p.Color[RED];
                    g_particule_color_data[4 * ParticlesCount + 1] =
                        p.Color[GREEN];
                    g_particule_color_data[4 * ParticlesCount + 2] =
                        p.Color[BLUE];
                    g_particule_color_data[4 * ParticlesCount + 3] =
                        p.Color[ALPHA];
                }
                else
                {
                    p.DistanceToCam = -1.0;
                }

                ParticlesCount++;
            }
        }
    }

    void Particles::main_particle(float x, float y, float z)
    {
        if (!running || !on)
            return;
        time += delta;
        for (int i = 0; i < particles_per_frame; i++)
        {
            int particleIndex = FirstUnusedParticle();

            if (particleIndex == -1)
                return;
            float theta = (float)rand() / RAND_MAX * 2 * M_PI;
            float phi = (float)rand() / RAND_MAX * 2 * M_PI;
            float life = (float)rand() / RAND_MAX;
            particles[particleIndex].MaxLife = life + (1 - life) * 0.5;
            particles[particleIndex].Life = particles[particleIndex].MaxLife;

            float radius = (float)(rand()) / RAND_MAX * 0.5;
            float posx = std::sin(phi) * std::cos(theta) * radius;
            float posy = std::cos(phi) * radius * 0.5;
            float posz = std::sin(phi) * std::sin(theta) * radius;

            particles[particleIndex].Position[X_] = posx + x - 0.8;
            particles[particleIndex].Position[Y_] = posy + y - 1.2;
            particles[particleIndex].Position[Z_] = posz + z - 0.3;

            GLfloat spread = 1.5f;
            std::vector<GLfloat> maindir = { 0.0, 10.0, 0.0 };

            std::vector<GLfloat> randomdir = {
                (rand() % 2000 + 1000.0f) / 1000.0f,
                (rand() % 2000 + 1000.0f) / 1000.0f,
                (rand() % 2000 + 1000.0f) / 1000.0f
            };

            particles[particleIndex].Velocity = {
                maindir[0] + randomdir[0] * spread,
                maindir[1] + randomdir[1] * spread,
                maindir[2] + randomdir[2] * spread
            };

            particles[particleIndex].Color[ALPHA] = (rand() % 256) / 3.0;

            particles[particleIndex].Size = (rand() % 1000) / 90.0 + 0.1f;
            particles[particleIndex].Color[RED] =
                particles[particleIndex].Size / 50.1;

            particles[particleIndex].Color[GREEN] =
                particles[particleIndex].Size / 60.1;
            particles[particleIndex].Color[BLUE] =
                particles[particleIndex].Size / 70.1;
            ParticlesCount++;
        }
    }
} // namespace particle
