#include "Gradient.hpp"

noise::module::Gradient::Gradient()
    : Module(GetSourceModuleCount())
    , m_noiseQuality(DEFAULT_GRADIENT_QUALITY)
    , m_seed(DEFAULT_GRADIENT_SEED)
{
}

double noise::module::Gradient::GetValue(double x, double y, double z) const
{
    return GradientCoherentNoise3D(x, y, z, m_seed, m_noiseQuality);
}
