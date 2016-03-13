#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL\glew.h>

bool LoadShaderFile(const std::string& Path, std::string& Data);

GLuint CompilerAndLinkProgram(const std::string& vertexData, const std::string& fragData);
