#include "ANLTerrainGenerator.hpp"

anl::CKernel ANLTerrainGenerator::kernel;

ANLTerrainGenerator::ANLTerrainGenerator()
{
}

double ANLTerrainGenerator::at(const double x, const double y, const double z, const double scale)
{
    static CNoiseExecutor exec(kernel);

    const int seed = 0;
    const int seed2 = 1000;

    auto groundGradient = kernel.gradientBasis(kernel.constant(anl::InterpolationTypes::INTERP_QUINTIC), kernel.seed(seed));

    auto lowlandShapeFractal = kernel.simpleBillow(anl::BasisTypes::BASIS_GRADIENT, anl::InterpolationTypes::INTERP_QUINTIC, 2, 0.25, seed);
    auto lowlandClamp = kernel.clamp(lowlandShapeFractal, kernel.zero(), kernel.one());
    auto lowlandScale = kernel.scaleOffset(lowlandClamp, 0.125, -0.45);
    auto lowlandScaleY = kernel.scaleDomain(lowlandScale, kernel.zero());
    auto lowlandTerrain = kernel.translateDomain(groundGradient, lowlandScaleY);

    auto highlandShapeFractal = kernel.simplefBm(anl::BasisTypes::BASIS_GRADIENT, anl::InterpolationTypes::INTERP_QUINTIC, 4, 2, seed);
    auto highlandClamp = kernel.clamp(highlandShapeFractal, kernel.constant(-1), kernel.one());
    auto highlandScale = kernel.scaleOffset(highlandClamp, 0.25, 0);
    auto highlandScaleY = kernel.scaleDomain(highlandScale, kernel.zero());
    auto highlandTerrain = kernel.translateDomain(groundGradient, highlandScaleY);

    auto mountainShapeFractal = kernel.simpleRidgedMultifractal(anl::BasisTypes::BASIS_GRADIENT, anl::InterpolationTypes::INTERP_QUINTIC, 8, 1, seed);
    auto mountainClamp = kernel.clamp(mountainShapeFractal, kernel.constant(-1), kernel.one());
    auto mountainScale = kernel.scaleOffset(mountainClamp, 0.45, 0.15);
    auto mountainScaleY = kernel.scaleDomain(mountainScale, kernel.constant(0.25));
    auto mountainTerrain = kernel.translateDomain(groundGradient, mountainScaleY);

    auto terrainTypeFractal = kernel.simplefBm(anl::BasisTypes::BASIS_GRADIENT, anl::InterpolationTypes::INTERP_QUINTIC, 3, 0.125, seed);
    auto terrainTypeClamp = kernel.clamp(terrainTypeFractal, kernel.zero(), kernel.one());
    auto terrainTypeScaleY = kernel.scaleDomain(terrainTypeClamp, kernel.zero());
    auto highlandMountainSelect = kernel.select(highlandTerrain, mountainTerrain, terrainTypeScaleY, kernel.constant(0.55), kernel.constant(0.2));
    auto highlandLowlandSelect = kernel.select(lowlandTerrain, highlandTerrain, terrainTypeScaleY, kernel.constant(0.25), kernel.constant(0.15));
    auto groundSelect = kernel.select(kernel.zero(), kernel.one(), highlandLowlandSelect, kernel.point5(), kernel.zero());

    auto caveAttenuateBias = kernel.bias(highlandLowlandSelect, kernel.constant(0.45));
    auto caveShape1 = kernel.simpleRidgedMultifractal(anl::BasisTypes::BASIS_GRADIENT, anl::InterpolationTypes::INTERP_QUINTIC, 1, 4, seed);
    auto caveShape2 = kernel.simpleRidgedMultifractal(anl::BasisTypes::BASIS_GRADIENT, anl::InterpolationTypes::INTERP_QUINTIC, 1, 4, seed2);
    auto caveShapeAttenuate = kernel.multiply(caveShape1, caveAttenuateBias);
    auto caveShapeAttenuate2 = kernel.multiply(caveShapeAttenuate, caveShape2);
    auto cavePeturbFractal = kernel.simplefBm(anl::BasisTypes::BASIS_GRADIENT, anl::InterpolationTypes::INTERP_QUINTIC, 6, 3, seed);
    auto cavePeturbScale = kernel.scaleOffset(cavePeturbFractal, 0.5, 0.0);
    auto cavePeturb = kernel.translateDomain(caveShapeAttenuate2, cavePeturbScale);
    auto caveSelect = kernel.select(kernel.zero(), kernel.one(), cavePeturb, kernel.constant(0.48), kernel.zero());

    auto groundCaveMultiply = kernel.multiply(caveSelect, groundSelect);

    return exec.evaluateScalar(x, y, z, lowlandTerrain) * scale;
}
