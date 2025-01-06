#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <map>
#include <string>

#define GLSL100ES(src) "#version 100 es\n" #src
#define GLSL300ES(src) "#version 300 es\n" #src
#define GLSL310ES(src) "#version 310 es\n" #src
#define GLSL320ES(src) "#version 320 es\n" #src

#define GLSL110(src) "#version 110\n" #src
#define GLSL120(src) "#version 120\n" #src
#define GLSL130(src) "#version 130\n" #src
#define GLSL140(src) "#version 140\n" #src
#define GLSL150(src) "#version 150\n" #src
#define GLSL330CORE(src) "#version 330 core\n" #src
#define GLSL330(src) "#version 330\n" #src
#define GLSL400(src) "#version 400\n" #src
#define GLSL410(src) "#version 410\n" #src
#define GLSL420(src) "#version 420\n" #src
#define GLSL430(src) "#version 430\n" #src
#define GLSL440(src) "#version 440\n" #src
#define GLSL450(src) "#version 450\n" #src
#define GLSL460(src) "#version 460\n" #src

class Shader
{
public:
    Shader();

    virtual ~Shader();

    GLuint id() const;

    void bind() const;

    bool compile(
        const std::string &vertShaderStr,
        const std::string &fragShaderStr);

    void setUniform(
        const char *uniformName,
        const glm::mat4 &m);

    void setUniform(
        const char *uniformName,
        const glm::vec4 &v);

    void setUniform(
        const char *uniformName,
        const glm::vec3 &v);

    void setUniform(
        const char *uniformName,
        int i);

    void setUniform(
        const char *uniformName,
        float f);

private:
    GLuint _shaderId = 0;
    std::map<std::string, GLuint> _uniforms;

    GLuint ensureUniformId(
        const char *uniformName);
};

#endif // SHADER_HPP
