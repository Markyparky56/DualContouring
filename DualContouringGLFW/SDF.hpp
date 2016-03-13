#pragma once
// Signed Distance Functions used to determine whether given points in space are within implicit objects
#include <glm\glm.hpp>
#include "OpenSimplexNoise.hpp"
using glm::vec3;
using glm::vec2;
using glm::length;

// Points within the sphere will have a negative density, points outwith the sphere will be positive
//inline float Sphere(const vec3& worldPosition, const vec3& origin, float radius)
//{
//    return length(worldPosition - origin) - radius;
//}

float FractalNoise(const int Octaves, const float Freq, const float Lac, const float persistence, const vec2& position);

// Can be used to combine multiple implicit functions into a single value
float DensityFunc(const vec3& worldPosition);