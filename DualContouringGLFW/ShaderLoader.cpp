#include "ShaderLoader.hpp"

bool LoadShaderFile(const std::string & Path, std::string & Data)
{
    std::ifstream file(Path.c_str());
    std::stringstream fileData;
    fileData << file.rdbuf();
    file.close();

    Data = fileData.str();

    return true;
}

GLuint CompilerAndLinkProgram(const std::string & vertexData, const std::string & fragData)
{
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint result = GL_FALSE;
    int infoLogLength;

    {
        // Compile vertex Shader
        printf("Compiling Vertex Shader\n");
        const GLchar* vertexSource = vertexData.c_str();
        glShaderSource(vertexShaderID, 1, &vertexSource, NULL);
        glCompileShader(vertexShaderID);

        // Check Vertex shader
        glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
        glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 1)
        {
            std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
            glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
            printf("%s\n", &vertexShaderErrorMessage[0]);
        }
    }
    {
        // Compile fragment Shader
        printf("Compiling fragment Shader\n");
        const GLchar* fragmentSource = fragData.c_str();
        glShaderSource(fragmentShaderID, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShaderID);

        // Check fragment shader
        glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
        glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 1)
        {
            std::vector<char> fragmentShaderErrorMessage(infoLogLength + 1);
            glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
            printf("%s\n", &fragmentShaderErrorMessage[0]);
        }
    }

    // Link the program
    printf("Linking program\n");
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Check the program
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 1)
    {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetProgramInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
        printf("%s\n", &programErrorMessage[0]);
    }

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
}
