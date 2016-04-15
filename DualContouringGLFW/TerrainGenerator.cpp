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
    
    // Initialise Lowland (Billow)
    lowland.SetNoiseQuality(NoiseQuality::QUALITY_BEST);
    lowland.SetOctaveCount(2);
    lowland.SetFrequency(0.25);
    lowland.SetSeed(seed);
    lowlandClamp.SetBounds(0.0, 1.0);
    lowlandClamp.SetSourceModule(0, lowland);
    lowlandScale.SetSourceModule(0, lowlandClamp);
    lowlandScale.SetScale(0.125);
    
    // Initialise Highland (Perlin)
    highland.SetNoiseQuality(NoiseQuality::QUALITY_BEST);
    highland.SetOctaveCount(4);
    highland.SetFrequency(2);
    highland.SetSeed(seed);
    highlandClamp.SetBounds(-1.0, 1.0);
    highlandClamp.SetSourceModule(0, highland);
    highlandScale.SetSourceModule(0, highlandClamp);
    highlandScale.SetScale(0.25);

    // Initialise Mountains (Ridged Multi)
    mountain.SetNoiseQuality(NoiseQuality::QUALITY_BEST);
    mountain.SetOctaveCount(8);
    mountain.SetFrequency(1);
    mountain.SetSeed(seed);
    mountainClamp.SetBounds(-1.0, 1.0);
    mountainClamp.SetSourceModule(0, mountain);
    mountainScale.SetSourceModule(0, mountainClamp);
    mountainScale.SetScale(0.25);
    
    // Initialise TerrainType (Perlin)
    terrainType.SetNoiseQuality(NoiseQuality::QUALITY_BEST);
    terrainType.SetOctaveCount(3);
    terrainType.SetFrequency(0.125);
    terrainType.SetSeed(seed);
    terrainTypeClamp.SetSourceModule(0, terrainType);
    terrainTypeClamp.SetBounds(0.0, 1.0);
    
    // Selectors (And Zero/One Constants)
    one.SetConstValue(1.0);
    zero.SetConstValue(0.0);

    highlandMountainSelect.SetSourceModule(0, highlandScale);
    highlandMountainSelect.SetSourceModule(1, mountainScale);
    highlandMountainSelect.SetControlModule(terrainTypeClamp);
    highlandMountainSelect.SetEdgeFalloff(0.2);
    
    highlandLowlandSelect.SetSourceModule(0, lowlandScale);
    highlandLowlandSelect.SetSourceModule(1, highlandMountainSelect);
    highlandLowlandSelect.SetControlModule(terrainTypeClamp);
    highlandLowlandSelect.SetEdgeFalloff(0.15);
    
    groundSelect.SetSourceModule(0, zero);
    groundSelect.SetSourceModule(1, one);
    groundSelect.SetControlModule(highlandLowlandSelect);
    groundSelect.SetEdgeFalloff(0.0);
    
    // Initialise Caves (2 Ridged Multis + Perlin)
    caveBias.SetSourceModule(0, highlandLowlandSelect);
    caveBias.SetBias(0.45);
    cave1.SetNoiseQuality(NoiseQuality::QUALITY_BEST);
    cave1.SetOctaveCount(1);
    cave1.SetFrequency(4);
    cave1.SetSeed(seed);
    cave2.SetNoiseQuality(NoiseQuality::QUALITY_BEST);
    cave2.SetOctaveCount(1);
    cave2.SetFrequency(4);
    cave2.SetSeed(seed2);
    
    caveShape1.SetSourceModule(0, cave1);
    caveShape1.SetSourceModule(1, caveBias);  
    caveShape2.SetSourceModule(0, caveShape1);
    caveShape2.SetSourceModule(1, caveBias);        
    
    cavePeturb.SetNoiseQuality(NoiseQuality::QUALITY_BEST);
    cavePeturb.SetOctaveCount(6);
    cavePeturb.SetFrequency(3);
    cavePeturb.SetSeed(seed);
    
    cavePeturbScale.SetSourceModule(0, cavePeturb);
    cavePeturbScale.SetScale(0.5);
    
    caveSelect.SetSourceModule(0, zero);
    caveSelect.SetSourceModule(1, one);
    caveSelect.SetControlModule(cavePeturbScale);
    caveSelect.SetEdgeFalloff(0);
    
    // Ground Cave Multiply
    groundCaveMultiply.SetSourceModule(0, caveSelect);
    groundCaveMultiply.SetSourceModule(1, groundSelect);
}

double TerrainGenerator::GetValue(const vec3& pos) const
{
    vec3 p = pos * scale;
    double value = 0.0f;
    float amplitude = 1.0f;
    p *= freq;
    
    for(int i = 0; i < octaves; i++)
    {
        value += groundCaveMultiply.GetValue(p.x, p.y, p.z) * amplitude;
        p *= lac;
        amplitude *= persistence;
    }
    
    return value;
}
