#ifndef OPENGLAPP_HPP
#define OPENGLAPP_HPP

#include <functional>

struct OpenGLApp
{
    /// The window title
    const char *title = "OpenGL App";

    /// The width of the the window to create. If this is 0, a full screen window is created
    int width = 0;

    /// The height of the the window to create. If this is 0, a full screen window is created
    int height = 0;

    /// This function is used to check if the app is still running
    std::function<bool()> GameLoop;

    /// This function is used to cleanup when the app was closed
    std::function<int()> Cleanup;
};

/// Open a app window. This will create a (fullscreen) window with a OpenGL 4.6 context.
bool openApp(
    OpenGLApp &app);

#endif // OPENGLAPP_HPP
