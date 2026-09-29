#pragma once

float lerp(float a, float b, float x);

float computePerlinAtCoord3D(int x, int y, int z, int nb_octaves = 5,
                             float persistence = 0.5, float lacunarity = 2,
                             int grid_size = 10);

float fireNoise(int x, int y, int z, int nb_octaves = 4,
                float persistence = 0.5, float lacunarity = 2,
                int grid_size = 8);
