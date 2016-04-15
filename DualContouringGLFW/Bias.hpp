#pragma once
// Bias Modifier Module
// Extension to Libnoise

#include "noise\module\modulebase.h"

namespace noise
{
    namespace module
    {
        class Bias : public Module
        {
        public:
            Bias();
            
            virtual int GetSourceModuleCount() const
            {
                return 1;
            }
            
            virtual double GetValue(double x, double y, double z) const;
            inline void SetBias(double b) { bias = b; }
            const double GetBias() { return bias; }
            
        private:
            double bias;
        };
    }
}