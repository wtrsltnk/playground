
#include <openglapp.hpp>
#include <shader.hpp>
#include <vertexbuffer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

int main(
    int argc,
    const char *argv[])
{
    spdlog::set_level(spdlog::level::debug);

    OpenGLApp app;

    app.title = "Playground";
    app.width = 800;
    app.height = 640;

    if (!openApp(app)) return 1;

    struct Vertex
    {
        float pos[3];
        float uv[2];
    };

    VertextBuffer<Vertex> vb;

    if (!vb.setup<float, 3, float, 2>("pos", "uv"))
    {
        spdlog::error("failed to setup vertex buffer");

        return 2;
    }

    vb.add({
        .pos = {-0.5f, -0.5f, 0.0f},
        .uv = {0.0f, 0.0f},
    });

    vb.add({
        .pos = {0.5f, -0.5f, 0.0f},
        .uv = {1.0f, 0.0f},
    });

    vb.add({
        .pos = {0.0f, 0.5f, 0.0f},
        .uv = {0.0f, 1.0f},
    });

    vb.upload();

    Shader shdr;

    const char *vs = GLSL460(
        layout(location = 0) in vec3 pos;
        layout(location = 1) in vec2 uv;

        uniform mat4 u_proj;
        uniform mat4 u_view;
        uniform mat4 u_model;

        void main() {
            gl_Position = u_proj * u_view * u_model * vec4(pos.x, pos.y, pos.z, 1.0);
        });

    const char *fs = GLSL460(
        layout(location = 0) out vec4 color;

        void main() {
            color = vec4(1.0, 0.0, 0.0, 1.0);
        });

    if (!shdr.compile(vs, fs))
    {
        spdlog::error("failed to compile shader");

        return 3;
    }

    shdr.setUniform("u_proj", glm::perspective(120.0f, app.width / std::max(1.0f, float(app.height)), 0.1f, 100.0f));
    shdr.setUniform("u_view", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f)));
    shdr.setUniform("u_model", glm::mat4(1.0f));

    while (app.GameLoop())
    {
        if (app.PressedKeyInCurrentFrame(KeyboardButtons::KeyEscape))
        {
            break;
        }

        if (app.isResizedInCurrentFrame)
        {
            glViewport(0, 0, app.width, app.height);
            shdr.setUniform("u_proj", glm::perspective(120.0f, app.width / std::max(1.0f, float(app.height)), 0.1f, 100.0f));
        }

        glClearColor(0.39f, 0.58f, 0.92f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shdr.bind();
        vb.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    return app.Cleanup();
}
