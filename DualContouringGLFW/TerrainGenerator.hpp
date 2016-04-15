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
    // Ground: Gradient Basis
    module::Gradient ground;
    
    // Lowland: Billow
    module::Billow lowland;
    module::Clamp lowlandClamp;
    module::ScaleBias lowlandScale;
    
    // Highland: Perlin (fbm)
    module::Perlin highland;
    module::Clamp highlandClamp;
    module::ScaleBias highlandScale;
    
    // Mountains: Ridged Multi
    module::RidgedMulti mountain;
    module::Clamp mountainClamp;
    module::ScaleBias mountainScale;
    
    // Terrain Type: Perlin (fbm)
    module::Perlin terrainType;
    module::Clamp terrainTypeClamp;
    
    // Selectors
    module::Const one;
    module::Const zero;
    module::Select highlandMountainSelect;
    module::Select highlandLowlandSelect;
    module::Select groundSelect;
    module::Select caveSelect;

    // Caves: 2 Ridged Multis
    module::Bias caveBias;
    module::RidgedMulti cave1;
    module::RidgedMulti cave2;
    module::Multiply caveShape1;
    module::Multiply caveShape2;
    module::Perlin cavePeturb;
    module::ScaleBias cavePeturbScale;
    module::Multiply groundCaveMultiply; // Final value is sourced from this module
    
    int octaves;
    float freq;
    float lac;
    float persistence;
    float scale;
};
