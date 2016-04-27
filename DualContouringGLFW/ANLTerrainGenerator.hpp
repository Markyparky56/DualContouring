#pragma once
#include "VM\kernel.h"
#include "VM\vm.h"

using anl::CKernel;
using anl::CNoiseExecutor;
using anl::CInstructionIndex;

class ANLTerrainGenerator
{
public:
    ANLTerrainGenerator();

    double at(const double x, const double y, const double z, const double scale = 0.0);

private:
    static CKernel kernel;
};
