#include "shader.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

Shader::Shader() = default;

Shader::~Shader() = default;

GLuint Shader::id() const
{
    return _shaderId;
}

void Shader::bind() const
{
    glUseProgram(_shaderId);
}

bool Shader::compile(
    const std::string &vertShaderStr,
    const std::string &fragShaderStr)
{
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *vertShaderSrc = vertShaderStr.c_str();
    const char *fragShaderSrc = fragShaderStr.c_str();

    GLint result = GL_FALSE;
    GLint logLength;

    // Compile vertex shader
    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    // Check vertex shader
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> vertShaderError(static_cast<size_t>((logLength > 1) ? logLength : 1));
        glGetShaderInfoLog(vertShader, logLength, NULL, &vertShaderError[0]);
        spdlog::error("error compiling vertex shader: {}", vertShaderError.data());

        return false;
    }

    // Compile fragment shader
    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    // Check fragment shader
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> fragShaderError(static_cast<size_t>((logLength > 1) ? logLength : 1));
        glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
        spdlog::error("error compiling fragment shader: {}", fragShaderError.data());

        return false;
    }

    _shaderId = glCreateProgram();
    glAttachShader(_shaderId, vertShader);
    glAttachShader(_shaderId, fragShader);
    glLinkProgram(_shaderId);

    glGetProgramiv(_shaderId, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
        glGetProgramiv(_shaderId, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> programError(static_cast<size_t>((logLength > 1) ? logLength : 1));
        glGetProgramInfoLog(_shaderId, logLength, NULL, &programError[0]);
        spdlog::error("error linking shader: {}", programError.data());

        return false;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    bind();

    return true;
}

GLuint Shader::ensureUniformId(
    const char *uniformName)
{
    bind();

    auto u = _uniforms.find(std::string(uniformName));

    if (u != _uniforms.end())
    {
        return u->second;
    }

    auto id = glGetUniformLocation(_shaderId, uniformName);

    _uniforms.insert({std::string(uniformName), id});

    return id;
}

void Shader::setUniform(
    const char *uniformName,
    const glm::mat4 &m)
{
    auto uid = ensureUniformId(uniformName);

    glUniformMatrix4fv(uid, 1, false, glm::value_ptr(m));
}

void Shader::setUniform(
    const char *uniformName,
    const glm::vec4 &v)
{
    auto uid = ensureUniformId(uniformName);

    glUniform4f(uid, v.r, v.g, v.b, v.a);
}

void Shader::setUniform(
    const char *uniformName,
    const glm::vec3 &v)
{
    auto uid = ensureUniformId(uniformName);

    glUniform3f(uid, v.x, v.y, v.z);
}

void Shader::setUniform(
    const char *uniformName,
    int i)
{
    auto uid = ensureUniformId(uniformName);

    glUniform1i(uid, i);
}

void Shader::setUniform(
    const char *uniformName,
    float f)
{
    auto uid = ensureUniformId(uniformName);

    glUniform1f(uid, f);
}
