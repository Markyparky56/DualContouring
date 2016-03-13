/*
 * OpenSimplex Noise in C++
 * Port by Mark A. Ropper
 * Original code (Public Domain) by Kurt Spencer https://gist.github.com/KdotJPG/b1270127455a94ac5d19
 */
#ifndef OPENSIMPLEXNOISE_H
#define OPENSIMPLEXNOISE_H

//#include <vector>
#include <array>
#include <cstdint>

using std::int8_t;
using std::array;

class OpenSimplexNoise
{
private:
    static const double STRETCH_CONSTANT_2D; 
    static const double SQUISH_CONSTANT_2D;
    static const double STRETCH_CONSTANT_3D;
    static const double SQUISH_CONSTANT_3D;
    static const double STRETCH_CONSTANT_4D;
    static const double SQUISH_CONSTANT_4D;

    static const double NORM_CONSTANT_2D;
    static const double NORM_CONSTANT_3D;
    static const double NORM_CONSTANT_4D;

    static const long long DEFAULT_SEED = 0;
    short perm[256]; 
    short permGradIndex3D[256];

    static array<int8_t, 16> gradients2D;
    static array<int8_t, 72> gradients3D;
    static array<int8_t, 256> gradients4D;

    double extrapolate(int xsb, int ysb, double dx, double dy);
    double extrapolate(int xsb, int ysb, int zsb, double dx, double dy, double dz);
    double extrapolate(int xsb, int ysb, int zsb, int wsb, double dx, double dy, double dz, double dw);
    static int fastFloor(double x);

    

public:
    OpenSimplexNoise();
    OpenSimplexNoise(const short perm[]);
    OpenSimplexNoise(long long seed);

    // 2D
    double eval(double x, double y);
    // 3D
    double eval(double x, double y, double z);
    // 4D
    double eval(double x, double y, double z, double w);
};

#endif // OPENSIMPLEXNOISE_H