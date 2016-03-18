#include "SDF.hpp"
#include "VM\kernel.h"
#include "VM\vm.h"
#include <glm/ext.hpp>

// ----------------------------------------------------------------------------

float Sphere(const vec3& worldPosition, const vec3& origin, float radius)
{
    return length(worldPosition - origin) - radius;
}

// ----------------------------------------------------------------------------

float Cuboid(const vec3& worldPosition, const vec3& origin, const vec3& halfDimensions)
{
    const vec3& local_pos = worldPosition - origin;
    const vec3& pos = local_pos;

    const vec3& d = glm::abs(pos) - halfDimensions;
    const float m = glm::max(d.x, glm::max(d.y, d.z));
    return glm::min(m, length(max(d, vec3(0.f))));
}

// ----------------------------------------------------------------------------

float FractalNoise(
    const int octaves,
    const float frequency,
    const float lacunarity,
    const float persistence,
    const vec2& position)
{
    const float SCALE = 1.f / 128.f;
    vec2 p = position * SCALE;
    float noise = 0.f;

    float amplitude = 1.f;
    p *= frequency;

    for (int i = 0; i < octaves; i++)
    {
        noise += simplex(p) * amplitude;
        p *= lacunarity;
        amplitude *= persistence;
    }

    // move into [0, 1] range
    return 0.5f + (0.5f * noise);
}

// ----------------------------------------------------------------------------

double OctavedOpenSimplex(const int octaves, 
                          const float freq, 
                          const float lac, 
                          const float persistence, 
                          const vec3& position, 
                          const float scale = 1.0f)
{
    static OpenSimplexNoise gen;
    float noise = 0.0f;
    vec3 p = position * scale;

    float amplitude = 1.0f;
    p *= freq;

    for (int i = 0; i < octaves; i++)
    {
        noise += gen.eval(p.x, p.y, p.z) * amplitude;
        p *= lac;
        amplitude *= persistence;
    }

    return noise;
}

// ----------------------------------------------------------------------------

float DensityFunc(const vec3& worldPosition)
{
    static anl::CKernel kernel;
    static anl::CNoiseExecutor vm(kernel);

    auto b = kernel.gradientBasis(kernel.constant(anl::InterpolationTypes::INTERP_QUINTIC), kernel.seed(0));


    const float MAX_HEIGHT = 64.0f;
    //const float noise = FractalNoise(4, 0.5343f, 2.2324f, 0.68324f, vec2(worldPosition.x, worldPosition.z));
    const float noise = OctavedOpenSimplex(1, 1.f, 1.f, 1.f, worldPosition, 0.1f);
    //const float noise = vm.e
    const float terrain = worldPosition.y - (MAX_HEIGHT * noise);

    const float cube = Cuboid(worldPosition, vec3(-4., 10.f, -4.f), vec3(12.f));
    const float sphere = Sphere(worldPosition, vec3(0.0f), 16.f);

    return glm::max(-sphere, terrain);// glm::max(-cube, glm::min(sphere, terrain));
}
