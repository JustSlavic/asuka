#include <stdio.h>

#include <defines.hpp>
#include <math/vector3.hpp>
#include <math/matrix4.hpp>
#include <math/color.hpp>
#include <os/time.hpp>

#include <windows.h>

#include <gl/gl.h>


// ===========================================


#define GL_CHECK_ERRORS(...) { auto err = glGetError(); if (err) { osOutputDebugString("%s\n", gl_error_string(err)); ASSERT(err == 0); } } void(0)

#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_ACCELERATION_ARB              0x2003
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_STENCIL_BITS_ARB              0x2023
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_SAMPLE_BUFFERS_ARB            0x2041
#define WGL_SAMPLES_ARB                   0x2042
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_MULTISAMPLE                    0x809D
#define GL_MULTISAMPLE_ARB                0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLES_ARB                    0x80A9
#define GL_SAMPLES_EXT                    0x80A9
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_FRAMEBUFFER                    0x8D40
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100

// Generate framebuffer object names
#define GL_GEN_FRAMEBUFFERS(name) void name(GLsizei n, GLuint *ids)
typedef GL_GEN_FRAMEBUFFERS(OpenGL_GenFrameffers);
GLOBAL OpenGL_GenFrameffers *glGenFramebuffers;

// Bind a framebuffer to a framebuffer target
#define GL_BIND_FRAMEBUFFER(name) void name(GLenum target, GLuint framebuffer)
typedef GL_BIND_FRAMEBUFFER(OpenGL_BindFramebuffer);
GLOBAL OpenGL_BindFramebuffer *glBindFramebuffer;

// Attach a level of a texture object as a logical buffer of a framebuffer object
#define GL_FRAMEBUFFER_TEXTURE_2D(name) void name(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
typedef GL_FRAMEBUFFER_TEXTURE_2D(OpenGL_FramebufferTexture2D);
GLOBAL OpenGL_FramebufferTexture2D *glFramebufferTexture2D;

#define GL_BLIT_FRAMEBUFFER(name) void name(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
typedef GL_BLIT_FRAMEBUFFER(OpenGL_BlitFramebuffer);
GLOBAL OpenGL_BlitFramebuffer *glBlitFramebuffer;

#define GL_CLEAR_BUFFER_IV(name) void name(GLenum buffer, GLint drawbuffer, GLint const *value)
typedef GL_CLEAR_BUFFER_IV(OpenGL_ClearBufferiv);
GLOBAL OpenGL_ClearBufferiv *glClearBufferiv;

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

// Detaches a shader object from a program object to which it is attached
#define GL_DETACH_SHADER(name) void name(GLuint program, GLuint shader)
typedef GL_DETACH_SHADER(OpenGL_DetachShader);
GLOBAL OpenGL_DetachShader *glDetachShader;

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
#define GL_GET_SHADERIV(name) void name(GLuint shader, GLenum pname, GLint *params)
typedef GL_GET_SHADERIV(OpenGL_GetShaderiv);
GLOBAL OpenGL_GetShaderiv *glGetShaderiv;

// Returns the information log for a shader object
#define GL_GET_SHADER_INFO_LOG(name) void name(GLuint shader, GLsizei maxLength, GLsizei *length, char *infoLog)
typedef GL_GET_SHADER_INFO_LOG(OpenGL_GetShaderInfoLog);
GLOBAL OpenGL_GetShaderInfoLog *glGetShaderInfoLog;

// Deletes a shader object
#define GL_DELETE_SHADER(name) void name(GLuint shader)
typedef GL_DELETE_SHADER(OpenGL_DeleteShader);
GLOBAL OpenGL_DeleteShader *glDeleteShader;

// Validates a program object
#define GL_VALIDATE_PROGRAM(name) void name(GLuint program)
typedef GL_VALIDATE_PROGRAM(OpenGL_ValidateProgram);
GLOBAL OpenGL_ValidateProgram *glValidateProgram;

// Returns a parameter from a program object
#define GL_GET_PROGRAMIV(name) void name(GLuint program, GLenum pname, GLint *params)
typedef GL_GET_PROGRAMIV(OpenGL_GetProgramiv);
GLOBAL OpenGL_GetProgramiv *glGetProgramiv;

// Returns the location of a uniform variable
#define GL_GET_UNIFORM_LOCATION(name) GLint name(GLuint program, char const *uniform_name)
typedef GL_GET_UNIFORM_LOCATION(OpenGL_GetUniformLocation);
GLOBAL OpenGL_GetUniformLocation *glGetUniformLocation;

// Specify the value of a uniform variable for the current program object
// #define GL_UNIFORM_1F(name) void (GLint location, GLfloat v0)
// #define GL_UNIFORM_2F(name) void (GLint location, GLfloat v0)
// #define GL_UNIFORM_3F(name) void (GLint location, GLfloat v0)
// #define GL_UNIFORM_4F(name) void (GLint location, GLfloat v0)
// @todo: All other definitions here.
#define GL_UNIFORM_MATRIX4(name) void name(int32 location, isize count, bool transpose, float32 const *value)
typedef GL_UNIFORM_MATRIX4(OpenGL_UniformMatrix4);
GLOBAL OpenGL_UniformMatrix4 *glUniformMatrix4fv;

// Establish the data storage, format, dimensions, and number of samples of a multisample texture's image
#define GL_TEX_IMAGE_2D_MULTISAMPLE(name) void name(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
typedef GL_TEX_IMAGE_2D_MULTISAMPLE(OpenGL_TexImage2DMultisample);
GLOBAL OpenGL_TexImage2DMultisample *glTexImage2DMultisample;

// Get string which you need to parse, to get what extensions are there
#define WGL_GET_EXTENSIONS_STRING_ARB(name) const char *WINAPI name(HDC hdc)
typedef WGL_GET_EXTENSIONS_STRING_ARB(WGL_GetExtensionsStringARB);
GLOBAL WGL_GetExtensionsStringARB *wglGetExtensionsStringARB;

// Extension for choosing pixel formats
#define WGL_CHOOSE_PIXEL_FORMAT_ARB(name) BOOL WINAPI name(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats)
typedef WGL_CHOOSE_PIXEL_FORMAT_ARB(WGL_ChoosePixelFormatARB);
GLOBAL WGL_ChoosePixelFormatARB *wglChoosePixelFormatARB;

// Extension function for creating OpenGL context with attributes
#define WGL_CREATE_CONTEXT_ATTRIBS_ARB(name) HGLRC WINAPI name(HDC hdc, HGLRC hShareContext, int const *attribList)
typedef WGL_CREATE_CONTEXT_ATTRIBS_ARB(WGL_CreateContextAttribsARB);
GLOBAL WGL_CreateContextAttribsARB *wglCreateContextAttribsARB;

// Extension function that sets swap interval
#define WGL_SWAP_INTERVAL_EXT(name) BOOL name(int interval)
typedef WGL_SWAP_INTERVAL_EXT(WGL_SwapInterval);
GLOBAL WGL_SwapInterval *wglSwapIntervalEXT;

// Extension function that gets swap interval
#define WGL_GET_SWAP_INTERVAL_EXT(name) int name(void)
typedef WGL_GET_SWAP_INTERVAL_EXT(WGL_GetSwapInterval);
GLOBAL WGL_GetSwapInterval *wglGetSwapIntervalEXT;


void InitializeOpenGLFunctions()
{
    glGenFramebuffers = (OpenGL_GenFrameffers *) wglGetProcAddress("glGenFramebuffers");
    glBindFramebuffer = (OpenGL_BindFramebuffer *) wglGetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D = (OpenGL_FramebufferTexture2D *) wglGetProcAddress("glFramebufferTexture2D");
    glBlitFramebuffer = (OpenGL_BlitFramebuffer *) wglGetProcAddress("glBlitFramebuffer");

    glClearBufferiv = (OpenGL_ClearBufferiv *) wglGetProcAddress("glClearBufferiv");
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
    glDetachShader = (OpenGL_DetachShader *) wglGetProcAddress("glDetachShader");
    glLinkProgram = (OpenGL_LinkProgram *) wglGetProcAddress("glLinkProgram");
    glUseProgram = (OpenGL_UseProgram *) wglGetProcAddress("glUseProgram");
    glGetShaderiv = (OpenGL_GetShaderiv *) wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (OpenGL_GetShaderInfoLog *) wglGetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (OpenGL_DeleteShader *) wglGetProcAddress("glDeleteShader");
    glValidateProgram = (OpenGL_ValidateProgram *) wglGetProcAddress("glValidateProgram");
    glGetProgramiv = (OpenGL_GetProgramiv *) wglGetProcAddress("glGetProgramiv");

    glGetUniformLocation = (OpenGL_GetUniformLocation *) wglGetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv = (OpenGL_UniformMatrix4 *) wglGetProcAddress("glUniformMatrix4fv");

    glTexImage2DMultisample = (OpenGL_TexImage2DMultisample *) wglGetProcAddress("glTexImage2DMultisample");
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

        osOutputDebugString("%s", message);

        glDeleteShader(id);
        delete[] message;

        return 0;
    }

    return id;
}

int32 is_shader_program_valid(uint32 program)
{
    glValidateProgram(program);
    int32 program_valid;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &program_valid);

    return program_valid;
}

char const *gl_error_string(GLenum ec)
{
    switch (ec) {
        case GL_INVALID_ENUM: return "Error: GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
        case GL_INVALID_VALUE: return "Error: GL_INVALID_VALUE: A numeric argument is out of range.";
        case GL_INVALID_OPERATION: return "Error: GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "Error: GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
        case GL_OUT_OF_MEMORY: return "Error: GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
        case GL_STACK_UNDERFLOW: return "Error: GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow.";
        case GL_STACK_OVERFLOW: return "Error: GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow.";
        case GL_NO_ERROR: return NULL;  // No error has been recorded. The value of this symbolic constant is guaranteed to be 0.
    }

    return NULL;
}

matrix4 make_projection_matrix(float32 w, float32 h, float32 n, float32 f)
{
    matrix4 result = {};

    result._11 = 2.0f * n / w;
    result._22 = 2.0f * n / h;
    result._33 = -(f + n) / (f - n);
    result._34 = -2.0f * f * n / (f - n);
    result._43 = -1.0f;

    return result;
}

matrix4 make_projection_matrix_fov(float32 fov, float32 aspect_ratio, float32 n, float32 f)
{
    //     w/2
    //   +-----+
    //   |    /
    //   |   /
    // n |  /
    //   | / angle = fov/2
    //   |/  tg(fov / 2) = (w/2) / n
    //   +   => 2n / w = 1 / tg(fov / 2)

    float32 tf2 = (1.0f / tanf(0.5f * fov));

    matrix4 result = {};

    result._11 = tf2;
    result._22 = tf2 * aspect_ratio;
    result._33 = -(f + n) / (f - n);
    result._34 = -2.0f * f * n / (f - n);
    result._43 = -1.0f;

    return result;
}

matrix4 make_orthographic_matrix(float32 w, float32 h, float32 n, float32 f)
{
    matrix4 result = {};

    result._11 = 2.0f / w;
    result._22 = 2.0f / h;
    result._33 = -2.0f / (f - n);
    result._34 = -(f + n) / (f - n);
    result._44 = 1.0f;

    return result;
}

matrix4 make_orthographic_matrix(float32 aspect_ratio, float32 n, float32 f)
{
    matrix4 result;

    result._11 = 1.0f;
    result._22 = 1.0f * aspect_ratio;
    result._33 = -2.0f / (f - n);
    result._34 = -(f + n) / (f - n);
    result._44 = 1.0f;

    return result;
}

// ===========================================


GLOBAL BOOL Running;
GLOBAL BOOL Wireframe;
GLOBAL BOOL IsPerspectiveProjection = TRUE;
GLOBAL BOOL ProjectionMatrixNeedsChange;
GLOBAL float32 inter_t;

GLOBAL UINT CurrentClientWidth;
GLOBAL UINT CurrentClientHeight;
GLOBAL BOOL ViewportNeedsResize;


LRESULT CALLBACK MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = {};

    switch (message)
    {
        case WM_SIZE:
        {
            CurrentClientWidth  = LOWORD(lParam);
            CurrentClientHeight = HIWORD(lParam);
            ViewportNeedsResize = true;
            osOutputDebugString("Resize (%d, %d)\n", CurrentClientWidth, CurrentClientHeight);
        }
        break;

        case WM_MOVE:
        break;

        case WM_CLOSE:
        case WM_DESTROY:
        {
            Running = false;
        }
        break;

        case WM_ACTIVATEAPP:
        break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            ASSERT_FAIL("Key handling happens in the main loop.");
        }
        break;

        default:
        {
            result = DefWindowProcA(Window, message, wParam, lParam);
        }
    }

    return result;
}


void Win32_ProcessPendingMessages()
{
    MSG Message;
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        if (Message.message == WM_QUIT) Running = false;
        TranslateMessage(&Message);

        switch (Message.message)
        {
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
                    if (VKCode == 'P')
                    {
                        if (IsDown)
                        {
                            TOGGLE(IsPerspectiveProjection);
                            ProjectionMatrixNeedsChange = TRUE;
                            inter_t = 1.0f;
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


struct ViewportSize
{
    uint32 OffsetX;
    uint32 OffsetY;
    uint32 Width;
    uint32 Height;
};


ViewportSize GetViewportSize(uint32 ClientWidth, uint32 ClientHeight, float32 DesiredAspectRatio)
{
    ViewportSize Viewport;

    float32 AspectRatio = f32(ClientWidth) / f32(ClientHeight);
    if (AspectRatio < DesiredAspectRatio)
    {
        Viewport.Width   = ClientWidth;
        Viewport.Height  = uint32(Viewport.Width / DesiredAspectRatio);
        Viewport.OffsetX = 0;
        Viewport.OffsetY = (ClientHeight - Viewport.Height) / 2;
    }
    else if (AspectRatio > DesiredAspectRatio)
    {
        Viewport.Height  = CurrentClientHeight;
        Viewport.Width   = uint32(Viewport.Height * DesiredAspectRatio);
        Viewport.OffsetX = (ClientWidth - Viewport.Width) / 2;
        Viewport.OffsetY = 0;
    }
    else
    {
        Viewport.Width   = ClientWidth;
        Viewport.Height  = ClientHeight;
        Viewport.OffsetX = 0;
        Viewport.OffsetY = 0;
    }

    return Viewport;
}


struct Camera
{
    vector3 position;
    vector3 forward;
    vector3 up;
    vector3 right;
};


Camera make_camera_at(vector3 position)
{
    Camera result;
    result.position = position;
    result.forward = { 0, 0, -1 };
    result.up = { 0, 1, 0 };
    result.right = { 1, 0, 0 };
    return result;
}


int WINAPI WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CmdLine,
    int CmdShow)
{
    int32 PrimaryMonitorWidth  = GetSystemMetrics(SM_CXSCREEN);
    int32 PrimaryMonitorHeight = GetSystemMetrics(SM_CYSCREEN);

    HBRUSH BlackBrush = CreateSolidBrush(RGB(0, 0, 0));

    WNDCLASSA WindowClass {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "AsukaWindowClass";
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = BlackBrush; // (HBRUSH) COLOR_WINDOW;

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

    HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "OpenGL Window",
        WS_OVERLAPPEDWINDOW, 0, 0, 50, 50, 0, 0, Instance, NULL);

    if (!Window)
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! Could not create a window.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    HDC DeviceContext = GetDC(Window);
    HGLRC TempRenderContext = {};
    HGLRC RenderContext = {};

    {
        PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
        DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
        DesiredPixelFormat.nVersion = 1;
        DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
        DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
        DesiredPixelFormat.cColorBits = 32;
        DesiredPixelFormat.cAlphaBits = 8;
        DesiredPixelFormat.cDepthBits = 24;
        DesiredPixelFormat.cStencilBits = 8;
        DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

        int SuggestedPixelFormatIndex = ChoosePixelFormat(DeviceContext, &DesiredPixelFormat);
        PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
        DescribePixelFormat(DeviceContext, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);

        if (SetPixelFormat(DeviceContext, SuggestedPixelFormatIndex, &SuggestedPixelFormat))
        {
            TempRenderContext = wglCreateContext(DeviceContext);
            if (wglMakeCurrent(DeviceContext, TempRenderContext))
            {
                wglGetExtensionsStringARB = (WGL_GetExtensionsStringARB *) wglGetProcAddress("wglGetExtensionsStringARB");
                wglChoosePixelFormatARB = (WGL_ChoosePixelFormatARB *) wglGetProcAddress("wglChoosePixelFormatARB");
                wglCreateContextAttribsARB = (WGL_CreateContextAttribsARB *) wglGetProcAddress("wglCreateContextAttribsARB");
                // @todo Check if 'WGL_EXT_swap_control' extension is available
                wglSwapIntervalEXT = (WGL_SwapInterval *) wglGetProcAddress("wglSwapIntervalEXT");
                wglGetSwapIntervalEXT = (WGL_GetSwapInterval *) wglGetProcAddress("wglGetSwapIntervalEXT");

                wglMakeCurrent(NULL, NULL);
                wglDeleteContext(TempRenderContext);
                ReleaseDC(Window, DeviceContext);
                DestroyWindow(Window);

                Window = CreateWindowExA(
                    0,                                // ExStyle
                    WindowClass.lpszClassName,        // ClassName
                    "OpenGL Window",                  // WindowName
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Style
                    (PrimaryMonitorWidth - Width(WindowRectangle)) / 2,   // X
                    (PrimaryMonitorHeight - Height(WindowRectangle)) / 2, // Y
                    Width(WindowRectangle),           // Width
                    Height(WindowRectangle),          // Height
                    0,                                // WndParent
                    0,                                // Menu
                    Instance,                         // Instance
                    NULL);                            // Param
                
                DeviceContext = GetDC(Window);

                // char const *WglExtensionString = wglGetExtensionsStringARB(DeviceContext);
                // Check if extension is available in the string
                // OutputDebugStringA(WglExtensionString);

                int WglAttributeList[] =
                {
                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                    WGL_COLOR_BITS_ARB, 32,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 8,
                    // WGL_SAMPLE_BUFFERS_ARB, 1,
                    // WGL_SAMPLES_ARB, 4,
                    0, // End
                };

                i32 PixelFormat;
                u32 NumberFormats;
                wglChoosePixelFormatARB(DeviceContext, WglAttributeList, NULL, 1, &PixelFormat, &NumberFormats);
                SetPixelFormat(DeviceContext, PixelFormat, &DesiredPixelFormat);

                int WglContextAttribList[] =
                {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
                    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    0, // End
                };

                RenderContext = wglCreateContextAttribsARB(DeviceContext, 0, WglContextAttribList);
                wglMakeCurrent(DeviceContext, RenderContext);
            }
            else
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

    {
        osOutputDebugString("%s\n", glGetString(GL_VENDOR));
        osOutputDebugString("%s\n", glGetString(GL_RENDERER));
        osOutputDebugString("%s\n", glGetString(GL_VERSION));

        int32 GL_MajorVersion, GL_MinorVersion;
        glGetIntegerv(GL_MAJOR_VERSION, &GL_MajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &GL_MinorVersion);
        osOutputDebugString("GL_MAJOR: %d\nGL_MINOR: %d\n", GL_MajorVersion, GL_MinorVersion);
    }

    wglSwapIntervalEXT(0);
    glDepthFunc(GL_LESS);

    // glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    // glEnable(GL_SAMPLE_ALPHA_TO_ONE);
    // glEnable(GL_MULTISAMPLE);

    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    uint32 framebuffer = 0;
    {
        uint32 framebuffer_color_texture;
        glGenTextures(1, &framebuffer_color_texture);

#define USE_MSAAx4 1
#if USE_MSAAx4
        int32 num_samples = 4;
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer_color_texture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_RGBA8, CurrentClientWidth, CurrentClientHeight, false);
#else
        glBindTexture(GL_TEXTURE_2D, framebuffer_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CurrentClientWidth, CurrentClientHeight, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
#endif

        GL_CHECK_ERRORS();

        uint32 framebuffer_depthstencil_texture;
        glGenTextures(1, &framebuffer_depthstencil_texture);

#if USE_MSAAx4
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer_depthstencil_texture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_DEPTH24_STENCIL8, CurrentClientWidth, CurrentClientHeight, false);
#else
        glBindTexture(GL_TEXTURE_2D, framebuffer_depthstencil_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, CurrentClientWidth, CurrentClientHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
#endif

        GL_CHECK_ERRORS();

        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
#if USE_MSAAx4
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebuffer_color_texture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, framebuffer_depthstencil_texture, 0);
#else
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_color_texture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebuffer_depthstencil_texture, 0);
#endif

        GL_CHECK_ERRORS();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);

        GL_CHECK_ERRORS();
    }

    int32 NumberOfSamples;
    glGetIntegerv(GL_SAMPLES, &NumberOfSamples);
    osOutputDebugString("Number of samples: %d\n", NumberOfSamples);

    struct Vertex
    {
        vector3 position;
        color32 color;
    };

    Vertex vertices[] =
    {
        { { -1.0f, -1.0f, -1.0f }, color32::blue },   // 0 bottom left
        { {  1.0f, -1.0f, -1.0f }, color32::green },  // 1 bottom right
        { {  1.0f,  1.0f, -1.0f }, color32::red },    // 2 top right
        { { -1.0f,  1.0f, -1.0f }, color32::yellow }, // 3 top left
    };

    u32 vertex_buffer_id = 0;
    {
        glGenBuffers(1, &vertex_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    u32 skybox_vertex_buffer = 0;
    {
        for (uint32 vertex_index = 0; vertex_index < ARRAY_COUNT(vertices); vertex_index++)
        {
            vertices[vertex_index].color = make_color32(0, 0.2, 0.4, 1);
        }
        glGenBuffers(1, &skybox_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, skybox_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    u32 indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0,  // second triangle
    };

    u32 index_buffer_id = 0;
    {
        glGenBuffers(1, &index_buffer_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }

    u32 skybox_vao = 0;
    {
        glGenVertexArrays(1, &skybox_vao);
        glBindVertexArray(skybox_vao);
        glBindBuffer(GL_ARRAY_BUFFER, skybox_vertex_buffer);

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

    // Cube mesh rendering
    vector3 cube_vertices[] =
    {
        {  1.0f,  1.0f, -0.5f },
        {  1.0f, -1.0f, -0.5f },
        {  1.0f,  1.0f,  0.5f },
        {  1.0f, -1.0f,  0.5f },
        { -1.0f,  1.0f, -0.5f },
        { -1.0f, -1.0f, -0.5f },
        { -1.0f,  1.0f,  0.5f },
        { -1.0f, -1.0f,  0.5f },
    };

    u32 cube_vertex_buffer_id = 0;
    {
        glGenBuffers(1, &cube_vertex_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    }

    f32 cube_texture_uv[] =
    {
        0.625f, 0.500f,
        0.875f, 0.500f,
        0.875f, 0.750f,
        0.625f, 0.750f,
        0.375f, 0.750f,
        0.625f, 1.000f,
        0.375f, 1.000f,
        0.375f, 0.000f,
        0.625f, 0.000f,
        0.625f, 0.250f,
        0.375f, 0.250f,
        0.125f, 0.500f,
        0.375f, 0.500f,
        0.125f, 0.750f,
    };

    u32 cube_texture_uv_buffer_id = 0;
    {
        glGenBuffers(1, &cube_texture_uv_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, cube_texture_uv_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texture_uv), cube_texture_uv, GL_STATIC_DRAW);
    }

    u32 cube_indices[] =
    {
        // top face
        1, 5, 7,
        7, 3, 1,
        // back face
        4, 3, 7,
        7, 8, 4,
        // left face
        8, 7, 5,
        5, 6, 8,
        // buttom face
        6, 2, 4,
        4, 8, 6,
        // right face
        2, 1, 3,
        3, 4, 2,
        // front face
        6, 5, 1,
        1, 2, 6,
    };

    // Fix enumeration from 1 in OBJ file format
    for (uint32 i = 0; i < ARRAY_COUNT(cube_indices); i++)
    {
        cube_indices[i] -= 1;
    }

    u32 cube_index_buffer_id = 0;
    {
        glGenBuffers(1, &cube_index_buffer_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_index_buffer_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
    }

    u32 cube_vertex_array_id = 0;
    {
        glGenVertexArrays(1, &cube_vertex_array_id);
        glBindVertexArray(cube_vertex_array_id);

        uint32 attrib_index = 0;
        {
            uint32 count = 3; // Because it's vector3

            glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_buffer_id);
            glEnableVertexAttribArray(attrib_index);
            glVertexAttribPointer(
                attrib_index,      // Index
                count,             // Count
                GL_FLOAT,          // Type
                GL_FALSE,          // Normalized?
                sizeof(vector3),   // Stride
                (void *) 0);  // Offset

            attrib_index += 1;
        }
        {
            uint32 count = 2; // Because it's 2d points

            glBindBuffer(GL_ARRAY_BUFFER, cube_texture_uv_buffer_id);
            glEnableVertexAttribArray(attrib_index);
            glVertexAttribPointer(
                attrib_index,
                count,
                GL_FLOAT,
                GL_FALSE,
                sizeof(vector2),
                (void *) 0);

            attrib_index += 1;
        }
    }

    char const *vertex_shader = R"GLSL(
#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 vertex_color;

// out vec3 fragment_position;
out vec4 fragment_color;

uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    vec4 p = u_projection * u_view * vec4(vertex_position, 1.0);
    // fragment_position = p.xyz / p.w;
    fragment_color = vertex_color;
    gl_Position = p;
}

)GLSL";

    char const *textured_uv_vs = R"GLSL(
#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 uv;

// out vec4 fragment_position;
out vec4 fragment_color;

uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    vec4 p = u_projection * u_view * vec4(vertex_position, 1.0);
    // fragment_position = vec4(vertex_position, 1.0);
    fragment_color = vec4(uv.rgr, 1.0);

    gl_Position = p;
}

)GLSL";

    char const *fragment_shader = R"GLSL(
#version 400

// in vec4 fragment_position;
in vec4 fragment_color;
out vec4 result_fragment_color;

void main()
{
    result_fragment_color = vec4(fragment_color.rgb, 1.0);
    // result_fragment_color = vec4(fragment_position.rgb * 2.0, 1.0);
}

)GLSL";

    auto vertex_shader_id = compile_shader(vertex_shader, GL_VERTEX_SHADER);
    auto fragment_shader_id = compile_shader(fragment_shader, GL_FRAGMENT_SHADER);

    auto shader_program_id = glCreateProgram();
    glAttachShader(shader_program_id, fragment_shader_id);
    glAttachShader(shader_program_id, vertex_shader_id);
    glLinkProgram(shader_program_id);

    glDetachShader(shader_program_id, fragment_shader_id);
    glDetachShader(shader_program_id, vertex_shader_id);
    glDeleteShader(vertex_shader_id);

    if (!is_shader_program_valid(shader_program_id))
    {
        return 1;
    }

    auto tex_vs_id = compile_shader(textured_uv_vs, GL_VERTEX_SHADER);
    auto tex_uv_shader_program = glCreateProgram();
    glAttachShader(tex_uv_shader_program, tex_vs_id);
    glAttachShader(tex_uv_shader_program, fragment_shader_id);
    glLinkProgram(tex_uv_shader_program);

    auto err = glGetError();
    if (err)
    {
        osOutputDebugString("%s\n", gl_error_string(err));
    }


    glDetachShader(tex_uv_shader_program, tex_vs_id);
    glDetachShader(tex_uv_shader_program, fragment_shader_id);

    glDeleteShader(tex_vs_id);
    glDeleteShader(fragment_shader_id);

    if (!is_shader_program_valid(tex_uv_shader_program))
    {
        return 1;
    }

    float32 DesiredAspectRatio = 16.0f / 9.0f;

    float32 n = 0.05f;
    float32 f = 100.0f;

    auto perspective = make_projection_matrix_fov(to_radians(60), DesiredAspectRatio, n, f);
    auto orthographic = make_orthographic_matrix(4, 3, n, f);

    Camera camera = make_camera_at({0, 0, 3});

    Running = true;
    os::timepoint LastClockTimepoint = os::get_wall_clock();
    float32 dtFromLastFrame = 0.0f;

    auto projection = perspective;

    while (Running)
    {
        Win32_ProcessPendingMessages();

        PERSIST f32 circle_t = 0.0f;
        f32 rot_x = 6.0f * cosf(circle_t);
        f32 rot_z = 6.0f * sinf(circle_t);

        camera.position.x = rot_x;
        camera.position.z = rot_z;

        circle_t += 0.5f * dtFromLastFrame;

        auto view = make_look_at_matrix(camera.position, make_vector3(0, 0, 0), camera.up);

        if (ProjectionMatrixNeedsChange)
        {
            // projection = IsPerspectiveProjection ? perspective : orthographic;
            projection = IsPerspectiveProjection ? lerp(perspective, orthographic, inter_t) : lerp(orthographic, perspective, inter_t);
            inter_t -= dtFromLastFrame;
            if (inter_t < 0) ProjectionMatrixNeedsChange = false;
        }

        if (ViewportNeedsResize)
        {
            ViewportSize Viewport = GetViewportSize(CurrentClientWidth, CurrentClientHeight, DesiredAspectRatio);
            glViewport(Viewport.OffsetX, Viewport.OffsetY, Viewport.Width, Viewport.Height);

            ViewportNeedsResize = false;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        if (1)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        }

        glUseProgram(shader_program_id);

        int32 u_view_location = glGetUniformLocation(shader_program_id, "u_view");
        glUniformMatrix4fv(u_view_location, 1, GL_TRUE, matrix4::identity.get_data());

        int32 u_projection_location = glGetUniformLocation(shader_program_id, "u_projection");
        glUniformMatrix4fv(u_projection_location, 1, GL_TRUE, matrix4::identity.get_data());

#if 0
        glBindVertexArray(vertex_array_id);
        glDrawArrays(GL_TRIANGLES, 0, 3);
#else
        // Skybox-ish thing
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindVertexArray(skybox_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
        glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indices), GL_UNSIGNED_INT, NULL);

        glEnable(GL_DEPTH_TEST);
#endif

        if (Wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glUniformMatrix4fv(u_view_location, 1, GL_TRUE, view.get_data());
        glUniformMatrix4fv(u_projection_location, 1, GL_TRUE, projection.get_data());

        glBindVertexArray(vertex_array_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
        glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indices), GL_UNSIGNED_INT, NULL);

        glUseProgram(tex_uv_shader_program);
        u_view_location = glGetUniformLocation(shader_program_id, "u_view");
        glUniformMatrix4fv(u_view_location, 1, GL_TRUE, view.get_data());
        u_projection_location = glGetUniformLocation(shader_program_id, "u_projection");
        glUniformMatrix4fv(u_projection_location, 1, GL_TRUE, projection.get_data());

        glBindVertexArray(cube_vertex_array_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_index_buffer_id);
        glDrawElements(GL_TRIANGLES, ARRAY_COUNT(cube_indices), GL_UNSIGNED_INT, NULL);

        if (1)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

            glDrawBuffer(GL_BACK);
            glBlitFramebuffer(0, 0, CurrentClientWidth, CurrentClientHeight, 0, 0, CurrentClientWidth, CurrentClientHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        SwapBuffers(DeviceContext);

        os::timepoint WorkTimepoint = os::get_wall_clock();
        dtFromLastFrame = get_seconds(WorkTimepoint - LastClockTimepoint);
        LastClockTimepoint = os::get_wall_clock();
    }

    return 0;
}
