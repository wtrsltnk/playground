#include <openglapp.hpp>

#include <Windowsx.h>
#include <glad/glad_wgl.h>
#include <spdlog/spdlog.h>

void updateKeyStateOnKeyDown(
    KeyButtonStates &state);

void updateKeyStateOnKeyUp(
    KeyButtonStates &state);

KeyboardButtons mapKey(
    WPARAM wParam);

LRESULT WINAPI wndProc(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    OpenGLApp *app;

    if (msg == WM_NCCREATE)
    {
        app = reinterpret_cast<OpenGLApp *>(((LPCREATESTRUCT)lParam)->lpCreateParams);

        if (app != nullptr)
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<long long>(app));
        }
    }
    else
    {
        app = reinterpret_cast<OpenGLApp *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    switch (msg)
    {
        case WM_KEYDOWN:
        {
            if (app != nullptr)
            {
                auto key = mapKey(wParam);

                if (key != KeyUnknown)
                {
                    updateKeyStateOnKeyDown(app->KeyStates[key]);
                }
            }
        }
        break;

        case WM_KEYUP:
        {
            if (app != nullptr)
            {
                auto key = mapKey(wParam);

                if (key != KeyUnknown)
                {
                    updateKeyStateOnKeyUp(app->KeyStates[key]);
                }
            }
        }
        break;

        case WM_LBUTTONDOWN:
        {
            if (app != nullptr)
            {
                updateKeyStateOnKeyDown(app->ButtonStates[MouseLeftButton]);
            }
        }
        break;

        case WM_LBUTTONUP:
        {
            if (app != nullptr)
            {
                updateKeyStateOnKeyUp(app->ButtonStates[MouseLeftButton]);
            }
        }
        break;

        case WM_RBUTTONDOWN:
        {
            if (app != nullptr)
            {
                updateKeyStateOnKeyDown(app->ButtonStates[MouseRightButton]);
            }
        }
        break;

        case WM_RBUTTONUP:
        {
            if (app != nullptr)
            {
                updateKeyStateOnKeyUp(app->ButtonStates[MouseRightButton]);
            }
        }
        break;

        case WM_MBUTTONDOWN:
        {
            if (app != nullptr)
            {
                updateKeyStateOnKeyDown(app->ButtonStates[MouseMiddleButton]);
            }
        }
        break;

        case WM_MBUTTONUP:
        {
            if (app != nullptr)
            {
                updateKeyStateOnKeyUp(app->ButtonStates[MouseMiddleButton]);
            }
        }
        break;

        case WM_MOUSEMOVE:
        {
            static LPARAM prevParam = lParam;

            if (app != nullptr)
            {
                app->mousePosX = GET_X_LPARAM(lParam);
                app->mousePosY = GET_Y_LPARAM(lParam);

                app->mouseDiffX = GET_X_LPARAM(prevParam) - app->mousePosX;
                app->mouseDiffY = GET_Y_LPARAM(prevParam) - app->mousePosY;
            }

            prevParam = lParam;
        }
        break;

        case WM_SIZE:
        {
            if (app != nullptr)
            {
                app->width = LOWORD(lParam);
                app->height = HIWORD(lParam);
            }
        }
        break;

        case WM_DESTROY:
        { /// standard exiting from winapi window
            PostQuitMessage(0);

            return 0;
        }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void OpenGLMessageCallback(
    unsigned source,
    unsigned type,
    unsigned id,
    unsigned severity,
    int length,
    const char *message,
    const void *userParam)
{
    (void)source;
    (void)type;
    (void)id;
    (void)length;
    (void)userParam;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            spdlog::critical("{} - {}", source, message);
            return;
        case GL_DEBUG_SEVERITY_MEDIUM:
            spdlog::error("{} - {}", source, message);
            return;
        case GL_DEBUG_SEVERITY_LOW:
            spdlog::warn("{} - {}", source, message);
            return;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            spdlog::trace("{} - {}", source, message);
            return;
    }

    spdlog::debug("Unknown severity level!");
    spdlog::debug(message);
}

void enableOpenGlDebug()
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);

    glDebugMessageControl(
        GL_DONT_CARE,
        GL_DONT_CARE,
        GL_DEBUG_SEVERITY_NOTIFICATION,
        0,
        NULL,
        GL_TRUE);
}

bool openApp(
    OpenGLApp &app)
{
    const char *windowClassName = "FullOpenGLAppWindow";

    HWND hwnd = NULL;
    HDC hdc = NULL;
    HGLRC hrc = NULL;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_CLASSDC,
        .lpfnWndProc = wndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInstance,
        .hIcon = LoadIcon(nullptr, IDI_WINLOGO),
        .hCursor = LoadCursor(nullptr, IDC_HAND),
        .hbrBackground = NULL,
        .lpszMenuName = NULL,
        .lpszClassName = windowClassName,
        .hIconSm = NULL,
    };
    RegisterClassEx(&wc);

    bool fullScreen = app.width == 0 || app.height == 0;

    if (fullScreen)
    {
        app.width = GetSystemMetrics(SM_CXSCREEN);
        app.height = GetSystemMetrics(SM_CYSCREEN);
    }

    hwnd = CreateWindow(
        wc.lpszClassName,
        app.title,
        fullScreen ? WS_POPUP | WS_VISIBLE : WS_OVERLAPPEDWINDOW,
        fullScreen ? 0 : CW_USEDEFAULT,
        fullScreen ? 0 : CW_USEDEFAULT,
        app.width, app.height,
        NULL, NULL,
        wc.hInstance,
        (VOID *)&app);

    hdc = GetDC(hwnd);

    if (hdc == 0)
    {
        spdlog::error("failed to get device context");

        return false;
    }

    static PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR), // Size Of This Pixel Format Descriptor
            1,                             // Version Number
            PFD_DRAW_TO_WINDOW |           // Format Must Support Window
                PFD_SUPPORT_OPENGL |       // Format Must Support CodeGL
                PFD_DOUBLEBUFFER,          // Must Support Double Buffering
            PFD_TYPE_RGBA,                 // Request An RGBA Format
            32,                            // Select Our Color Depth
            0, 0, 0, 0, 0, 0,              // Color Bits Ignored
            0,                             // No Alpha Buffer
            0,                             // Shift Bit Ignored
            0,                             // No Accumulation Buffer
            0, 0, 0, 0,                    // Accumulation Bits Ignored
            24,                            // 24Bit Z-Buffer (Depth Buffer)
            0,                             // No Stencil Buffer
            0,                             // No Auxiliary Buffer
            PFD_MAIN_PLANE,                // Main Drawing Layer
            0,                             // Reserved
            0, 0, 0                        // Layer Masks Ignored
        };

    auto pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == false)
    {
        spdlog::error("failed to choose pixel format");

        return false;
    }

    if (SetPixelFormat(hdc, pixelFormat, &pfd) == FALSE)
    {
        spdlog::error("failed to set pixel format");

        return false;
    }

    auto tempRc = wglCreateContext(hdc);

    if (tempRc == 0)
    {
        spdlog::error("failed to create render context");

        return false;
    }

    if (!wglMakeCurrent(hdc, tempRc))
    {
        spdlog::error("failed to set current render context");

        return false;
    }

    gladLoadWGL(hdc);

    GLint attribList[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB,
            4,
            WGL_CONTEXT_MINOR_VERSION_ARB,
            6,
            WGL_CONTEXT_FLAGS_ARB,
            0,
            0,
        };
    hrc = wglCreateContextAttribsARB(hdc, NULL, attribList);

    if (hrc == 0)
    {
        spdlog::error("failed to create opengl 4.6 render context");

        return false;
    }

    wglDeleteContext(tempRc);

    if (!wglMakeCurrent(hdc, hrc))
    {
        spdlog::error("failed to set current render context");

        wglDeleteContext(hrc);

        return false;
    }

    gladLoadGL();
    enableOpenGlDebug();

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    if (fullScreen) ShowCursor(false);

    glViewport(0, 0, app.width, app.height);

    spdlog::info("window with opengl 4.6 context created");

    app.GameLoop = [hwnd, hdc]() -> bool {
        MSG msg = {};

        SwapBuffers(hdc);

        auto app = reinterpret_cast<OpenGLApp *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        if (app != nullptr)
        {
            for (int i = 0; i < KeyboardButtonsCount; i++)
            {
                if (app->KeyStates[i] == KeyButtonStates::KeyButtonStatePressed) app->KeyStates[i] = KeyButtonStates::KeyButtonStateDown;
                if (app->KeyStates[i] == KeyButtonStates::KeyButtonStateReleased) app->KeyStates[i] = KeyButtonStates::KeyButtonStateUp;
            }
        }

        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                spdlog::info("exiting app with return code {}", msg.wParam);

                return false;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return true;
    };

    app.Cleanup = [hwnd, windowClassName, hInstance]() -> int {
        ShowCursor(true);
        DestroyWindow(hwnd);
        UnregisterClass(windowClassName, hInstance);

        return 0;
    };

    return true;
}

void updateKeyStateOnKeyDown(
    KeyButtonStates &state)
{
    if (state == KeyButtonStateUp)
    {
        state = KeyButtonStatePressed;
    }
    else if (state == KeyButtonStateDown || state == KeyButtonStatePressed)
    {
        state = KeyButtonStateDown;
    }
}

void updateKeyStateOnKeyUp(
    KeyButtonStates &state)
{
    if (state == KeyButtonStateDown)
    {
        state = KeyButtonStateReleased;
    }
    else if (state == KeyButtonStateUp || state == KeyButtonStateReleased)
    {
        state = KeyButtonStateUp;
    }
}

KeyboardButtons mapKey(
    WPARAM wParam)
{
    switch (wParam)
    {
        case VK_ESCAPE:
            return KeyEscape;
        default:
            return KeyUnknown;
    }
}
