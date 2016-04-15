#include "Bias.hpp"
#include <cmath>

noise::module::Bias::Bias()
    : Module(GetSourceModuleCount())
    , bias(0.5)
{
}

double noise::module::Bias::GetValue(double x, double y, double z) const
{
    assert(m_pSourceModule[0] != NULL);
    
    double value = m_pSourceModule[0]->GetValue(x, y, z);
    double biasValue = std::pow(value, std::log(bias)/std::log(0.5));
    
    return biasValue;
}
