#pragma once
#include "noise\noise.h"
#include "Gradient.hpp"
#include "Bias.hpp"
#include <glm/ext.hpp>
#include <random>

using namespace noise;
using glm::vec3;

// Terrain Generator Class
// Uses a combination of noise types to produce terrain

class TerrainGenerator
{
public:
    TerrainGenerator();
    double GetValue(const vec3& pos) const;
    
    inline void SetOctaves(const int NewOctaves) { octaves = NewOctaves; }
    inline void SetFreq(const float NewFreq) { freq = NewFreq; }
    inline void SetLac(const float NewLac) { lac = NewLac; }
    inline void SetPersistence(const float NewPersistence) { persistence = NewPersistence; }
    inline void SetScale(const float NewScale) { scale = NewScale; }
    
    inline int GetOctaves() const { return octaves; }
    inline float GetFreq() const { return freq; }
    inline float GetLac() const { return lac; }
    inline float GetPersistence() const { return persistence; }
    inline float GetScale() const { return scale; }
    
    
private:
    module::Billow baseFlatTerrain;
    module::ScaleBias flatTerrain;
    module::RidgedMulti mountainTerrain;
    module::Select terrain;
    module::Perlin terrainType;
    module::Turbulence finalTerain;

    int octaves;
    float freq;
    float lac;
    float persistence;
    float scale;
};
