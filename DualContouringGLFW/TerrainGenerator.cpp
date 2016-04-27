#include "TerrainGenerator.hpp"

TerrainGenerator::TerrainGenerator()
    : octaves(1)
    , freq(1.0f)
    , lac(1.0f)
    , persistence(1.0f)
    , scale(1.0f)
{
    const int seed = std::random_device()();
    const int seed2 = std::random_device()();
    
    //baseFlatTerrain.SetSeed(seed);
    baseFlatTerrain.SetFrequency(2.0);
    flatTerrain.SetSourceModule(0, baseFlatTerrain);
    flatTerrain.SetScale(0.0625);
    flatTerrain.SetBias(-0.75);

    //terrainType.SetSeed(seed2);
    terrainType.SetFrequency(0.5);
    terrainType.SetPersistence(0.25);

    terrain.SetSourceModule(0, flatTerrain);
    terrain.SetSourceModule(1, mountainTerrain);
    terrain.SetControlModule(terrainType);
    terrain.SetBounds(0.0, 1000.0);
    terrain.SetEdgeFalloff(0.125);

    finalTerain.SetSourceModule(0, terrain);
    finalTerain.SetFrequency(1.0);
    finalTerain.SetPower(0.0625);
    //mountain
}

double TerrainGenerator::GetValue(const vec3& pos) const
{
    vec3 p = pos * scale;
    double value = 0.0f;
    float amplitude = 1.0f;
    p *= freq;
    
    for(int i = 0; i < octaves; i++)
    {
        value += finalTerain.GetValue(p.x, p.y, p.z) * amplitude;
        p *= lac;
        amplitude *= persistence;
    }
    
    return value;
}
