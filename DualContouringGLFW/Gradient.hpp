#pragma once
// Gradient Noise Module
// Extension to Libnoise

#include "noise\module\modulebase.h"

namespace noise
{
    namespace module
    {
        const noise::NoiseQuality DEFAULT_GRADIENT_QUALITY = QUALITY_STD;
        const int DEFAULT_GRADIENT_SEED = 0;
        
        class Gradient : public Module
        {
        public:
            Gradient();

            noise::NoiseQuality GetNoiseQuality() const
            {
                return m_noiseQuality;
            }

            int GetSeed() const
            {
                return m_seed;
            }

            virtual int GetSourceModuleCount() const
            {
                return 0;
            }

            virtual double GetValue(double x, double y, double z) const;

            void SetNoiseQuality(noise::NoiseQuality noiseQuality)
            {
                m_noiseQuality = noiseQuality;
            }

            void SetSeed(int seed)
            {
                m_seed = seed;
            }

        protected:
            noise::NoiseQuality m_noiseQuality;
            int m_seed;
          
        };

    }
}