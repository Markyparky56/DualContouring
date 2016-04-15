#include "SDF.hpp"
// #include "noise\noise.h"
#include <glm/ext.hpp>
// #include <random>
 #include <thread>
 #include <mutex>
// #include "Gradient.hpp" // Our simple gradient module
// #include "Bias.hpp" // Our custom module for applying a bias to a noise function

using namespace noise;

std::once_flag GeneratorInitFlag;

void InitaliseTerrainGenerator()
{
    GTerrainGenerator.SetOctaves(1);
    GTerrainGenerator.SetFreq(1.f);
    GTerrainGenerator.SetLac(1.f);
    GTerrainGenerator.SetPersistence(1.f);
    GTerrainGenerator.SetScale(1.f);
}

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
/*
// double TerrainNoise(const vec3& pos, 
                    // const int octaves, 
                    // const float freq, 
                    // const float lac, 
                    // const float persistence
                    // const float scale = 1.0f)
// {
    // // Ground: Gradient basis
    // module::Gradient ground;
    // // Lowland: Billow
    // module::Billow lowland;
    // module::Clamp lowlandClamp;
    // module::ScaleBias lowlandScale;
    // // Highland: Perlin
    // module::Perlin highland;
    // module::Clamp highlandClamp;
    // module::ScaleBias highlandScale;    
    // // Mountains: Ridged Multi
    // module::RidgedMulti mountain;
    // module::Clamp mountainClamp;
    // module::ScaleBias mountainScale;
    // // Terrain Type: Perlin
    // module::Perlin terrainType;
    // module::Clamp terrainTypeClamp;    
    // module::Select highlandMountainSelect;    
    // module::Select highlandLowlandSelect;    
    // module::Const one;
    // module::Const zero;    
    // module::Select groundSelect;    
    // // Caves: 2 Ridged Multis
    // module::Bias caveBias;
    // module::RidgedMulti cave1;
    // module::RidgedMulti cave2;    
    // module::Multiply caveShape1;
    // module::Multiply caveShape2;    
    // module::Perlin cavePeturb;
    // module::ScaleBias cavePeturbScale;
    // module::Select caveSelect;    
    // module::Multiply groundCaveMultiply;        
    // vec3 p = pos * scale;
    // double value = 0.0f;
    // float amplitude = 1.0f;
    // p *= freq;    
    // for(int i = 0; i < octaves; i++)
    // {
        // noise += groundCaveMultiply.GetValue(p.x, p.y, p.z) * amplitude;
        // p *= lac;
        // amplitude *= persistence;
    // }    
    // return value;
// }
*/

// ----------------------------------------------------------------------------

float DensityFunc(const vec3& worldPosition)
{
    std::call_once(GeneratorInitFlag, InitaliseTerrainGenerator);

    const float MAX_HEIGHT = 64.0f;
    //const float noise = FractalNoise(4, 0.5343f, 2.2324f, 0.68324f, vec2(worldPosition.x, worldPosition.z));
    //const float noise = OctavedOpenSimplex(1, 1.f, 1.f, 1.f, worldPosition, 0.1f);
    const float noise = static_cast<float>(GTerrainGenerator.GetValue(worldPosition));
    const float terrain = worldPosition.y - (MAX_HEIGHT * noise);

    //const float cube = Cuboid(worldPosition, vec3(-4., 10.f, -4.f), vec3(12.f));
    //const float sphere = Sphere(worldPosition, vec3(0.0f), 16.f);

    return /*glm::max(-sphere, */terrain/*)*/;// glm::max(-cube, glm::min(sphere, terrain));
}
