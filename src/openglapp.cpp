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

                app->isResizedInCurrentFrame = true;
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

    HWND hwnd = CreateWindow(
        wc.lpszClassName,
        app.title,
        fullScreen ? WS_POPUP | WS_VISIBLE : WS_OVERLAPPEDWINDOW,
        fullScreen ? 0 : CW_USEDEFAULT,
        fullScreen ? 0 : CW_USEDEFAULT,
        app.width, app.height,
        NULL, NULL,
        wc.hInstance,
        (VOID *)&app);

    return embedApp((long)hwnd, app);
}

bool embedApp(
    long window,
    OpenGLApp &app)
{
    const char *windowClassName = "ChildOpenGLAppWindow";

    HWND hwnd = (HWND)window;
    HDC hdc = NULL;
    HGLRC hrc = NULL;
    HINSTANCE hInstance = GetModuleHandle(NULL);

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

            app->isResizedInCurrentFrame = false;
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

MouseButtons mapButton(
    WPARAM wParam)
{
    switch (wParam)
    {
        case VK_LBUTTON:
            return MouseLeftButton; // 0x01 	Left mouse button
        case VK_RBUTTON:
            return MouseRightButton; // 0x02 	Right mouse button
        case VK_CANCEL:
            return MouseUnknown; // 0x03 	Control-break processing
        case VK_MBUTTON:
            return MouseMiddleButton; // 0x04 	Middle mouse button
        case VK_XBUTTON1:
            return MouseUnknown; // 0x05 	X1 mouse button
        case VK_XBUTTON2:
            return MouseUnknown; // 0x06 	X2 mouse button
        default:
            return MouseUnknown;
    }
}

KeyboardButtons mapKey(
    WPARAM wParam)
{
    switch (wParam)
    {
        case VK_BACK:
            return KeyBackspace; // 0x08 	BACKSPACE key
        case VK_TAB:
            return KeyTab; // 0x09 	TAB key
        case VK_CLEAR:
            return KeyUnknown; // 0x0C 	CLEAR key
        case VK_RETURN:
            return KeyEnter; // 0x0D 	ENTER key
        case VK_SHIFT:
            return KeyUnknown; // 0x10 	SHIFT key
        case VK_CONTROL:
            return KeyUnknown; // 0x11 	CTRL key
        case VK_MENU:
            return KeyUnknown; // 0x12 	ALT key
        case VK_PAUSE:
            return KeyPause; // 0x13 	PAUSE key
        case VK_CAPITAL:
            return KeyCapsLock; // 0x14 	CAPS LOCK key
        // case VK_KANA: return KeyUnknown; // 0x15 	IME Kana mode
        // case VK_HANGUL: return KeyUnknown; // 0x15 	IME Hangul mode
        // case VK_IME_ON: return KeyUnknown; // 0x16 	IME On
        // case VK_JUNJA: return KeyUnknown; // 0x17 	IME Junja mode
        // case VK_FINAL: return KeyUnknown; // 0x18 	IME final mode
        // case VK_HANJA: return KeyUnknown; // 0x19 	IME Hanja mode
        // case VK_KANJI: return KeyUnknown; // 0x19 	IME Kanji mode
        // case VK_IME_OFF: return KeyUnknown; // 0x1A 	IME Off
        case VK_ESCAPE:
            return KeyEscape; // 0x1B 	ESC key
        // case VK_CONVERT: return KeyUnknown; // 0x1C 	IME convert
        // case VK_NONCONVERT: return KeyUnknown; // 0x1D 	IME nonconvert
        // case VK_ACCEPT: return KeyUnknown; // 0x1E 	IME accept
        // case VK_MODECHANGE: return KeyUnknown; // 0x1F 	IME mode change request
        case VK_SPACE:
            return KeySpace; // 0x20 	SPACEBAR
        case VK_PRIOR:
            return KeyPageUp; // 0x21 	PAGE UP key
        case VK_NEXT:
            return KeyPageDown; // 0x22 	PAGE DOWN key
        case VK_END:
            return KeyEnd; // 0x23 	END key
        case VK_HOME:
            return KeyHome; // 0x24 	HOME key
        case VK_LEFT:
            return KeyLeft; // 0x25 	LEFT ARROW key
        case VK_UP:
            return KeyUp; // 0x26 	UP ARROW key
        case VK_RIGHT:
            return KeyRight; // 0x27 	RIGHT ARROW key
        case VK_DOWN:
            return KeyDown; // 0x28 	DOWN ARROW key
        case VK_SELECT:
            return KeyUnknown; // 0x29 	SELECT key
        case VK_PRINT:
            return KeyUnknown; // 0x2A 	PRINT key
        case VK_EXECUTE:
            return KeyUnknown; // 0x2B 	EXECUTE key
        case VK_SNAPSHOT:
            return KeyPrintScreen; // 0x2C 	PRINT SCREEN key
        case VK_INSERT:
            return KeyInsert; // 0x2D 	INS key
        case VK_DELETE:
            return KeyDelete; // 0x2E 	DEL key
        case VK_HELP:
            return KeyUnknown; // 0x2F 	HELP key
        case VK_LWIN:
            return KeyLeftSuper; // 0x5B 	Left Windows key
        case VK_RWIN:
            return KeyRightSuper; // 0x5C 	Right Windows key
        case VK_APPS:
            return KeyUnknown; // 0x5D 	Applications key
        case VK_SLEEP:
            return KeyUnknown; // 0x5F 	Computer Sleep key
        case VK_NUMPAD0:
            return KeyKp0; // 0x60 	Numeric keypad 0 key
        case VK_NUMPAD1:
            return KeyKp1; // 0x61 	Numeric keypad 1 key
        case VK_NUMPAD2:
            return KeyKp2; // 0x62 	Numeric keypad 2 key
        case VK_NUMPAD3:
            return KeyKp3; // 0x63 	Numeric keypad 3 key
        case VK_NUMPAD4:
            return KeyKp4; // 0x64 	Numeric keypad 4 key
        case VK_NUMPAD5:
            return KeyKp5; // 0x65 	Numeric keypad 5 key
        case VK_NUMPAD6:
            return KeyKp6; // 0x66 	Numeric keypad 6 key
        case VK_NUMPAD7:
            return KeyKp7; // 0x67 	Numeric keypad 7 key
        case VK_NUMPAD8:
            return KeyKp8; // 0x68 	Numeric keypad 8 key
        case VK_NUMPAD9:
            return KeyKp9; // 0x69 	Numeric keypad 9 key
        case VK_MULTIPLY:
            return KeyUnknown; // 0x6A 	Multiply key
        case VK_ADD:
            return KeyUnknown; // 0x6B 	Add key
        case VK_SEPARATOR:
            return KeyUnknown; // 0x6C 	Separator key
        case VK_SUBTRACT:
            return KeyMinus; // 0x6D 	Subtract key
        case VK_DECIMAL:
            return KeyComma; // 0x6E 	Decimal key
        case VK_DIVIDE:
            return KeySlash; // 0x6F 	Divide key
        case VK_F1:
            return KeyF1; // 0x70 	F1 key
        case VK_F2:
            return KeyF2; // 0x71 	F2 key
        case VK_F3:
            return KeyF3; // 0x72 	F3 key
        case VK_F4:
            return KeyF4; // 0x73 	F4 key
        case VK_F5:
            return KeyF5; // 0x74 	F5 key
        case VK_F6:
            return KeyF6; // 0x75 	F6 key
        case VK_F7:
            return KeyF7; // 0x76 	F7 key
        case VK_F8:
            return KeyF8; // 0x77 	F8 key
        case VK_F9:
            return KeyF9; // 0x78 	F9 key
        case VK_F10:
            return KeyF10; // 0x79 	F10 key
        case VK_F11:
            return KeyF11; // 0x7A 	F11 key
        case VK_F12:
            return KeyF12; // 0x7B 	F12 key
        case VK_F13:
            return KeyF13; // 0x7C 	F13 key
        case VK_F14:
            return KeyF14; // 0x7D 	F14 key
        case VK_F15:
            return KeyF15; // 0x7E 	F15 key
        case VK_F16:
            return KeyF16; // 0x7F 	F16 key
        case VK_F17:
            return KeyF17; // 0x80 	F17 key
        case VK_F18:
            return KeyF18; // 0x81 	F18 key
        case VK_F19:
            return KeyF19; // 0x82 	F19 key
        case VK_F20:
            return KeyF20; // 0x83 	F20 key
        case VK_F21:
            return KeyF21; // 0x84 	F21 key
        case VK_F22:
            return KeyF22; // 0x85 	F22 key
        case VK_F23:
            return KeyF23; // 0x86 	F23 key
        case VK_F24:
            return KeyF24; // 0x87 	F24 key
        case VK_NUMLOCK:
            return KeyNumLock; // 0x90 	NUM LOCK key
        case VK_SCROLL:
            return KeyScrollLock; // 0x91 	SCROLL LOCK key
        case VK_LSHIFT:
            return KeyLeftShift; // 0xA0 	Left SHIFT key
        case VK_RSHIFT:
            return KeyRightShift; // 0xA1 	Right SHIFT key
        case VK_LCONTROL:
            return KeyLeftControl; // 0xA2 	Left CONTROL key
        case VK_RCONTROL:
            return KeyRightControl; // 0xA3 	Right CONTROL key
        case VK_LMENU:
            return KeyLeftAlt; // 0xA4 	Left ALT key
        case VK_RMENU:
            return KeyRightAlt; // 0xA5 	Right ALT key
        case VK_BROWSER_BACK:
            return KeyUnknown; // 0xA6 	Browser Back key
        case VK_BROWSER_FORWARD:
            return KeyUnknown; // 0xA7 	Browser Forward key
        case VK_BROWSER_REFRESH:
            return KeyUnknown; // 0xA8 	Browser Refresh key
        case VK_BROWSER_STOP:
            return KeyUnknown; // 0xA9 	Browser Stop key
        case VK_BROWSER_SEARCH:
            return KeyUnknown; // 0xAA 	Browser Search key
        case VK_BROWSER_FAVORITES:
            return KeyUnknown; // 0xAB 	Browser Favorites key
        case VK_BROWSER_HOME:
            return KeyUnknown; // 0xAC 	Browser Start and Home key
        case VK_VOLUME_MUTE:
            return KeyUnknown; // 0xAD 	Volume Mute key
        case VK_VOLUME_DOWN:
            return KeyUnknown; // 0xAE 	Volume Down key
        case VK_VOLUME_UP:
            return KeyUnknown; // 0xAF 	Volume Up key
        case VK_MEDIA_NEXT_TRACK:
            return KeyUnknown; // 0xB0 	Next Track key
        case VK_MEDIA_PREV_TRACK:
            return KeyUnknown; // 0xB1 	Previous Track key
        case VK_MEDIA_STOP:
            return KeyUnknown; // 0xB2 	Stop Media key
        case VK_MEDIA_PLAY_PAUSE:
            return KeyUnknown; // 0xB3 	Play/Pause Media key
        case VK_LAUNCH_MAIL:
            return KeyUnknown; // 0xB4 	Start Mail key
        case VK_LAUNCH_MEDIA_SELECT:
            return KeyUnknown; // 0xB5 	Select Media key
        case VK_LAUNCH_APP1:
            return KeyUnknown; // 0xB6 	Start Application 1 key
        case VK_LAUNCH_APP2:
            return KeyUnknown; // 0xB7 	Start Application 2 key
        case VK_OEM_1:
            return KeyUnknown; // 0xBA 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ;: key
        case VK_OEM_PLUS:
            return KeyUnknown; // 0xBB 	For any country/region, the + key
        case VK_OEM_COMMA:
            return KeyUnknown; // 0xBC 	For any country/region, the , key
        case VK_OEM_MINUS:
            return KeyUnknown; // 0xBD 	For any country/region, the - key
        case VK_OEM_PERIOD:
            return KeyUnknown; // 0xBE 	For any country/region, the . key
        case VK_OEM_2:
            return KeyUnknown; // 0xBF 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the /? key
        case VK_OEM_3:
            return KeyUnknown; // 0xC0 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the `~ key
        case VK_OEM_4:
            return KeyUnknown; // 0xDB 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the [{ key
        case VK_OEM_5:
            return KeyUnknown; // 0xDC 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the \\| key
        case VK_OEM_6:
            return KeyUnknown; // 0xDD 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ]} key
        case VK_OEM_7:
            return KeyUnknown; // 0xDE 	Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '" key
        case VK_OEM_8:
            return KeyUnknown; // 0xDF 	Used for miscellaneous characters; it can vary by keyboard.
        case VK_OEM_102:
            return KeyUnknown; // 0xE2 	The <> keys on the US standard keyboard, or the \\| key on the non-US 102-key keyboard
        case VK_PROCESSKEY:
            return KeyUnknown; // 0xE5 	IME PROCESS key
        case VK_PACKET:
            return KeyUnknown; // 0xE7 	Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP
        case VK_ATTN:
            return KeyUnknown; // 0xF6 	Attn key
        case VK_CRSEL:
            return KeyUnknown; // 0xF7 	CrSel key
        case VK_EXSEL:
            return KeyUnknown; // 0xF8 	ExSel key
        case VK_EREOF:
            return KeyUnknown; // 0xF9 	Erase EOF key
        case VK_PLAY:
            return KeyUnknown; // 0xFA 	Play key
        case VK_ZOOM:
            return KeyUnknown; // 0xFB 	Zoom key
        case VK_NONAME:
            return KeyUnknown; // 0xFC 	Reserved
        case VK_PA1:
            return KeyUnknown; // 0xFD 	PA1 key
        case VK_OEM_CLEAR:
            return KeyUnknown; // 0xFE 	Clear key

        case 0x30:
            return Key0; // 0x30 	0 key
        case 0x31:
            return Key1; // 0x31 	1 key
        case 0x32:
            return Key2; // 0x32 	2 key
        case 0x33:
            return Key3; // 0x33 	3 key
        case 0x34:
            return Key4; // 0x34 	4 key
        case 0x35:
            return Key5; // 0x35 	5 key
        case 0x36:
            return Key6; // 0x36 	6 key
        case 0x37:
            return Key7; // 0x37 	7 key
        case 0x38:
            return Key8; // 0x38 	8 key
        case 0x39:
            return Key9; // 0x39 	9 key
        case 0x41:
            return KeyA; // 0x41 	A key
        case 0x42:
            return KeyB; // 0x42 	B key
        case 0x43:
            return KeyC; // 0x43 	C key
        case 0x44:
            return KeyD; // 0x44 	D key
        case 0x45:
            return KeyE; // 0x45 	E key
        case 0x46:
            return KeyF; // 0x46 	F key
        case 0x47:
            return KeyG; // 0x47 	G key
        case 0x48:
            return KeyH; // 0x48 	H key
        case 0x49:
            return KeyI; // 0x49 	I key
        case 0x4A:
            return KeyJ; // 0x4A 	J key
        case 0x4B:
            return KeyK; // 0x4B 	K key
        case 0x4C:
            return KeyL; // 0x4C 	L key
        case 0x4D:
            return KeyM; // 0x4D 	M key
        case 0x4E:
            return KeyN; // 0x4E 	N key
        case 0x4F:
            return KeyO; // 0x4F 	O key
        case 0x50:
            return KeyP; // 0x50 	P key
        case 0x51:
            return KeyQ; // 0x51 	Q key
        case 0x52:
            return KeyR; // 0x52 	R key
        case 0x53:
            return KeyS; // 0x53 	S key
        case 0x54:
            return KeyT; // 0x54 	T key
        case 0x55:
            return KeyU; // 0x55 	U key
        case 0x56:
            return KeyV; // 0x56 	V key
        case 0x57:
            return KeyW; // 0x57 	W key
        case 0x58:
            return KeyX; // 0x58 	X key
        case 0x59:
            return KeyY; // 0x59 	Y key
        case 0x5A:
            return KeyZ; // 0x5A 	Z key
        default:
            return KeyUnknown;
    }
}
