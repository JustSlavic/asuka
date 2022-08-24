#include <stdio.h>

#include <defines.hpp>
#include <math/vector3.hpp>
#include <math/color.hpp>

#include <windows.h>

#include <gl/gl.h>


struct Vertex
{
    f32 x, y, z;
    color32 color;
};


// ===========================================


#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84

// Generate buffer object names
#define GL_GEN_BUFFERS(name) void name(GLsizei n, GLuint *buffers)
typedef GL_GEN_BUFFERS(OpenGL_GenBuffers);
GLOBAL OpenGL_GenBuffers *glGenBuffers;

// Bind a named buffer object
#define GL_BIND_BUFFER(name) void name(GLenum target, GLuint buffer)
typedef GL_BIND_BUFFER(OpenGL_BindBuffer);
GLOBAL OpenGL_BindBuffer *glBindBuffer;

// Creates and initializes a buffer object's data store
#define GL_BUFFER_DATA(name) void name(GLenum target, intptr size, const void *data, GLenum usage)
typedef GL_BUFFER_DATA(OpenGL_BufferData);
GLOBAL OpenGL_BufferData *glBufferData;

// Generate vertex array object names
#define GL_GEN_VERTEX_ARRAY(name) void name(GLsizei n, GLuint *arrays)
typedef GL_GEN_VERTEX_ARRAY(OpenGL_GenVertexArray);
GLOBAL OpenGL_GenVertexArray *glGenVertexArrays;

// Bind a vertex array object
#define GL_BIND_VERTEX_ARRAY(name) void name(GLuint array)
typedef GL_BIND_VERTEX_ARRAY(OpenGL_BindVertexArray);
GLOBAL OpenGL_BindVertexArray *glBindVertexArray;

// Define an array of generic vertex attribute data
#define GL_VERTEX_ATTRIB_POINTER(name) void name(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
typedef GL_VERTEX_ATTRIB_POINTER(OpenGL_VertexAttribPointer);
GLOBAL OpenGL_VertexAttribPointer *glVertexAttribPointer;

// Enable or disable a generic vertex attribute array
#define GL_ENABLE_VERTEX_ATTRIB_ARRAY(name) void name(GLuint index)
typedef GL_ENABLE_VERTEX_ATTRIB_ARRAY(OpenGL_EnableVertexAttribArray);
GLOBAL OpenGL_EnableVertexAttribArray *glEnableVertexAttribArray;

// Creates a shader object
#define GL_CREATE_SHADER(name) GLuint name(GLenum shaderType)
typedef GL_CREATE_SHADER(OpenGL_CreateShader);
GLOBAL OpenGL_CreateShader *glCreateShader;

// Replaces the source code in a shader object
#define GL_SHADER_SOURCE(name) void name(GLuint shader, GLsizei count, char const **string, GLint const *length)
typedef GL_SHADER_SOURCE(OpenGL_ShaderSource);
GLOBAL OpenGL_ShaderSource *glShaderSource;

// Compiles a shader object
#define GL_COMPILE_SHADER(name) void name(GLuint shader)
typedef GL_COMPILE_SHADER(OpenGL_CompileShader);
GLOBAL OpenGL_CompileShader *glCompileShader;

// Creates a program object
#define GL_CREATE_PROGRAM(name) GLuint name(void)
typedef GL_CREATE_PROGRAM(OpenGL_CreateProgram);
GLOBAL OpenGL_CreateProgram *glCreateProgram;

// Attaches a shader object to a program object
#define GL_ATTACH_SHADER(name) void name(GLuint program, GLuint shader)
typedef GL_ATTACH_SHADER(OpenGL_AttachShader);
GLOBAL OpenGL_AttachShader *glAttachShader;

// Links a program object
#define GL_LINK_PROGRAM(name) void name( GLuint program)
typedef GL_LINK_PROGRAM(OpenGL_LinkProgram);
GLOBAL OpenGL_LinkProgram *glLinkProgram;

// Installs a program object as part of current rendering state
#define GL_USE_PROGRAM(name) void name(GLuint program)
typedef GL_USE_PROGRAM(OpenGL_UseProgram);
GLOBAL OpenGL_UseProgram *glUseProgram;

// Return a parameter from a shader object. The following parameters are defined:
// GL_SHADER_TYPE -- params returns GL_VERTEX_SHADER if shader is a vertex shader object, and GL_FRAGMENT_SHADER if shader is a fragment shader object.
// GL_DELETE_STATUS -- params returns GL_TRUE if shader is currently flagged for deletion, and GL_FALSE otherwise.
// GL_COMPILE_STATUS -- For implementations that support a shader compiler, params returns GL_TRUE if the last compile operation on shader was successful, and GL_FALSE otherwise.
// GL_INFO_LOG_LENGTH -- For implementations that support a shader compiler, params returns the number of characters in the information log for shader including the null termination character (i.e., the size of the character buffer required to store the information log). If shader has no information log, a value of 0 is returned.
// GL_SHADER_SOURCE_LENGTH -- For implementations that support a shader compiler, params returns the length of the concatenation of the source strings that make up the shader source for the shader, including the null termination character. (i.e., the size of the character buffer required to store the shader source). If no source code exists, 0 is returned.
#define GL_GET_SHADERIV(name) void name(GLuint shader, GLenum pname, GLint *params);
typedef GL_GET_SHADERIV(OpenGL_GetShaderiv);
GLOBAL OpenGL_GetShaderiv *glGetShaderiv;

// Returns the information log for a shader object
#define GL_GET_SHADER_INFO_LOG(name) void name(GLuint shader, GLsizei maxLength, GLsizei *length, char *infoLog);
typedef GL_GET_SHADER_INFO_LOG(OpenGL_GetShaderInfoLog);
GLOBAL OpenGL_GetShaderInfoLog *glGetShaderInfoLog;

// Deletes a shader object
#define GL_DELETE_SHADER(name) void name(GLuint shader);
typedef GL_DELETE_SHADER(OpenGL_DeleteShader);
GLOBAL OpenGL_DeleteShader *glDeleteShader;

// Validates a program object
#define GL_VALIDATE_PROGRAM(name) void name(GLuint program);
typedef GL_VALIDATE_PROGRAM(OpenGL_ValidateProgram);
GLOBAL OpenGL_ValidateProgram *glValidateProgram;

// Returns a parameter from a program object
#define GL_GET_PROGRAMIV(name) void name(GLuint program, GLenum pname, GLint *params);
typedef GL_GET_PROGRAMIV(OpenGL_GetProgramiv);
GLOBAL OpenGL_GetProgramiv *glGetProgramiv;


void InitializeOpenGLFunctions()
{
    glGenBuffers = (OpenGL_GenBuffers *) wglGetProcAddress("glGenBuffers");
    glBindBuffer = (OpenGL_BindBuffer *) wglGetProcAddress("glBindBuffer");
    glBufferData = (OpenGL_BufferData *) wglGetProcAddress("glBufferData");
    glGenVertexArrays = (OpenGL_GenVertexArray *) wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (OpenGL_BindVertexArray *) wglGetProcAddress("glBindVertexArray");
    glVertexAttribPointer = (OpenGL_VertexAttribPointer *) wglGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (OpenGL_EnableVertexAttribArray *) wglGetProcAddress("glEnableVertexAttribArray");
    glCreateShader = (OpenGL_CreateShader *) wglGetProcAddress("glCreateShader");
    glShaderSource = (OpenGL_ShaderSource *) wglGetProcAddress("glShaderSource");
    glCompileShader = (OpenGL_CompileShader *) wglGetProcAddress("glCompileShader");
    glCreateProgram = (OpenGL_CreateProgram *) wglGetProcAddress("glCreateProgram");
    glAttachShader = (OpenGL_AttachShader *) wglGetProcAddress("glAttachShader");
    glLinkProgram = (OpenGL_LinkProgram *) wglGetProcAddress("glLinkProgram");
    glUseProgram = (OpenGL_UseProgram *) wglGetProcAddress("glUseProgram");
    glGetShaderiv = (OpenGL_GetShaderiv *) wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (OpenGL_GetShaderInfoLog *) wglGetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (OpenGL_DeleteShader *) wglGetProcAddress("glDeleteShader");
    glValidateProgram = (OpenGL_ValidateProgram *) wglGetProcAddress("glValidateProgram");
    glGetProgramiv = (OpenGL_GetProgramiv *) wglGetProcAddress("glGetProgramiv");
}


uint32 compile_shader(char const *source_code, GLenum shader_type)
{
    uint32 id = glCreateShader(shader_type);
    glShaderSource(id, 1, &source_code, NULL);
    glCompileShader(id);

    int32 successful;
    glGetShaderiv(id, GL_COMPILE_STATUS, &successful);
    if (successful == GL_FALSE)
    {
        int32 length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        // @todo: use transient memory for that
        char* message = new char[length + 1];
        memset(message, 0, length + 1);

        glGetShaderInfoLog(id, length, &length, message);

        printf("%s", message);

        glDeleteShader(id);
        delete[] message;

        return 0;
    }

    return id;
}


// ===========================================


GLOBAL BOOL Running;
GLOBAL BOOL Wireframe;

GLOBAL UINT CurrentClientWidth;
GLOBAL UINT CurrentClientHeight;
GLOBAL BOOL ViewportNeedsResize;


LRESULT CALLBACK MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result {};

    switch (message) {
        case WM_SIZE: {
            CurrentClientWidth  = LOWORD(lParam);
            CurrentClientHeight = HIWORD(lParam);
            ViewportNeedsResize = true;
            osOutputDebugString("Resize (%d, %d)\n", CurrentClientWidth, CurrentClientHeight);
            break;
        }
        case WM_MOVE: {
            break;
        }
        case WM_CLOSE: {
            Running = false;
            break;
        }
        case WM_DESTROY: {
            Running = false;
            break;
        }
        case WM_ACTIVATEAPP: {
            break;
        }
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            ASSERT_FAIL("Key handling happens in the main loop.");
            break;
        }
        default: {
            result = DefWindowProcA(Window, message, wParam, lParam);
        }
    }

    return result;
}


void Win32_ProcessPendingMessages() {
    MSG Message;
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
        if (Message.message == WM_QUIT) Running = false;
        TranslateMessage(&Message);

        switch (Message.message) {
            case WM_MOUSEMOVE:
            break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 VKCode  = (u32)Message.wParam;
                b32 WasDown = (Message.lParam & (1 << 30)) != 0;
                b32 IsDown  = (Message.lParam & (1 << 31)) == 0;
                if (WasDown != IsDown)
                {
                    if (VKCode == 'W')
                    {
                        if (IsDown)
                        {
                            TOGGLE(Wireframe);
                        }
                    }
                    if (VKCode == VK_ESCAPE)
                    {
                        Running = false;
                    }
                }
            }
            break;

            default:
            {
                DispatchMessageA(&Message);
            }
            break;
        }
    }
}


i32 Width(RECT Rect)
{
    i32 Result = Rect.right - Rect.left;
    return Result;
}


i32 Height(RECT Rect)
{
    i32 Result = Rect.bottom - Rect.top;
    return Result;
}


int WINAPI WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CmdLine,
    int CmdShow)
{
    int32 PrimaryMonitorWidth  = GetSystemMetrics(SM_CXSCREEN);
    int32 PrimaryMonitorHeight = GetSystemMetrics(SM_CYSCREEN);

    WNDCLASSA WindowClass {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "AsukaWindowClass";
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = NULL; // (HBRUSH) COLOR_WINDOW;

    ATOM ClassAtomResult = RegisterClassA(&WindowClass);
    if (!ClassAtomResult)
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! Could not register window class.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    CurrentClientWidth  = 800;
    CurrentClientHeight = 600;
    RECT WindowRectangle = { 0, 0, (LONG) CurrentClientWidth, (LONG) CurrentClientHeight };
    if (!AdjustWindowRect(&WindowRectangle, WS_OVERLAPPEDWINDOW, false))
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! AdjustWindowRect failed.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    HWND Window = CreateWindowExA(
        0,                                // ExStyle
        WindowClass.lpszClassName,        // ClassName
        "OpenGL Window",                   // WindowName
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Style
        (PrimaryMonitorWidth - Width(WindowRectangle)) / 2,   // X
        (PrimaryMonitorHeight - Height(WindowRectangle)) / 2, // Y
        Width(WindowRectangle),           // Width
        Height(WindowRectangle),          // Height
        0,                                // WndParent
        0,                                // Menu
        Instance,                         // Instance
        NULL);                            // Param

    if (!Window)
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! Could not create a window.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    HDC DeviceContext = GetDC(Window);
    HGLRC RenderContext = {};

    {
        PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
        DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
        DesiredPixelFormat.nVersion = 1;
        DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
        DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
        DesiredPixelFormat.cColorBits = 32;
        DesiredPixelFormat.cAlphaBits = 8;
        DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

        int SuggestedPixelFormatIndex = ChoosePixelFormat(DeviceContext, &DesiredPixelFormat);
        PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
        DescribePixelFormat(DeviceContext, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);

        if (SetPixelFormat(DeviceContext, SuggestedPixelFormatIndex, &SuggestedPixelFormat))
        {
            RenderContext = wglCreateContext(DeviceContext);
            if (!wglMakeCurrent(DeviceContext, RenderContext))
            {
                MessageBeep(MB_ICONERROR);
                MessageBoxA(0, "System error! Could not initialize OpenGL.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
                return 1;
            }
        }
        else
        {
            MessageBeep(MB_ICONERROR);
            MessageBoxA(0, "System error! Could not initialize OpenGL.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
            return 1;
        }
    }

    InitializeOpenGLFunctions();

    struct Vertex
    {
        vector3 position;
        color32 color;
    };

    Vertex vertices[] =
    {
        { { -0.5f, -0.5f, 0.0f }, color32::blue }, // bottom left
        { {  0.5f, -0.5f, 0.0f }, color32::green }, // bottom right
        { {  0.5f,  0.5f, 0.0f }, color32::red }, // top right
        { { -0.5f,  0.5f, 0.0f }, color32::yellow }, // top left
        { { -1.0f,  1.0f, 0.0f }, color32::gray },
        { {  1.0f,  1.0f, 0.0f }, color32::magenta },
        { {  1.0f, -1.0f, 0.0f }, color32::cyan },
        { { -1.0f, -1.0f, 0.0f }, color32{ 0.0f, 0.5f, 0.0f, 0.0f, } },
    };

    u32 vertex_buffer_id = 0;
    {
        glGenBuffers(1, &vertex_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    u32 indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0,  // second triangle
        4, 2, 3,
        4, 0, 3,
        5, 4, 2,
        6, 5, 2,
        6, 2, 1,
        0, 6, 1,
        6, 0, 7,
        4, 7, 0,
    };

    u32 index_buffer_id = 0;
    {
        glGenBuffers(1, &index_buffer_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }

    u32 vertex_array_id = 0;
    {
        glGenVertexArrays(1, &vertex_array_id);
        glBindVertexArray(vertex_array_id);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);

        uint32 attrib_index = 0;
        uint64 offset = 0;
        {
            uint32 count = 3; // Because it's vector3

            glEnableVertexAttribArray(attrib_index);
            glVertexAttribPointer(
                attrib_index,      // Index
                count,             // Count
                GL_FLOAT,          // Type
                GL_FALSE,          // Normalized?
                sizeof(Vertex),    // Stride
                (void *) offset);  // Offset

            attrib_index += 1;
            offset += (count * sizeof(float));
        }
        {
            uint32 count = 4; // Because it's color32

            glEnableVertexAttribArray(attrib_index);
            glVertexAttribPointer(
                attrib_index,      // Index
                count,             // Count
                GL_FLOAT,          // Type
                GL_FALSE,          // Normalized?
                sizeof(Vertex),    // Stride
                (void *) offset);  // Offset

            attrib_index += 1;
            offset += (count * sizeof(float));
        }
    }

    const char *vertex_shader = R"GLSL(
#version 400

in vec3 vertex_position;
in vec4 vertex_color;
out vec4 fragment_position;
out vec4 fragment_color;

void main()
{
    fragment_position = vec4(vertex_position, 1.0);
    fragment_color = vertex_color;
    gl_Position = vec4(vertex_position, 1.0);
}

)GLSL";

    const char *fragment_shader = R"GLSL(
#version 400

in vec4 fragment_position;
in vec4 fragment_color;
out vec4 result_fragment_color;

void main()
{
    result_fragment_color = fragment_color;
    // result_fragment_color = vec4(fragment_position.rgb * 2.0, 1.0);
}

)GLSL";

    uint32 vertex_shader_id = compile_shader(vertex_shader, GL_VERTEX_SHADER);
    uint32 fragment_shader_id = compile_shader(fragment_shader, GL_FRAGMENT_SHADER);

    GLuint shader_program_id = glCreateProgram();
    glAttachShader(shader_program_id, fragment_shader_id);
    glAttachShader(shader_program_id, vertex_shader_id);
    glLinkProgram(shader_program_id);

    {
        glValidateProgram(shader_program_id);
        GLint program_valid;
        glGetProgramiv(shader_program_id, GL_VALIDATE_STATUS, &program_valid);
        if (program_valid == GL_FALSE)
        {
            return 1;
        }
    }

    Running = true;
    while (Running)
    {
        Win32_ProcessPendingMessages();

        if (Wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if (ViewportNeedsResize) {
            glViewport(0, 0, CurrentClientWidth, CurrentClientHeight);
            ViewportNeedsResize = false;
        }

        glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program_id);

        glBindVertexArray(vertex_array_id);

#if 0
        glDrawArrays(GL_TRIANGLES, 0, 3);
#else
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
        glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indices), GL_UNSIGNED_INT, NULL);
#endif

        SwapBuffers(DeviceContext);
    }

    return 0;
}
