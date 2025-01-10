#ifndef OPENGLAPP_HPP
#define OPENGLAPP_HPP

#include <functional>

struct OpenGLApp
{
    /// The title of the window to create.
    const char *title = "OpenGL App";

    /// The width of the window to create. If this is 0, a fullscreen window is created.
    int width = 0;

    /// The height of the window to create. If this is 0, a fullscreen window is created.
    int height = 0;

    /// This function is used to check if the app is still running.
    std::function<bool()> GameLoop;

    /// This function is used to cleanup when the app is closed.
    std::function<int()> Cleanup;
};

/// Open an app window. This will create a (fullscreen) window with an OpenGL 4.6 context.
bool openApp(
    OpenGLApp &app);

#endif // OPENGLAPP_HPP
