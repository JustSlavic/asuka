#include <stdio.h>

#include <defines.hpp>
#include <math/vector3.hpp>
#include <math/matrix4.hpp>
#include <math/color.hpp>
#include <os/time.hpp>
#include <png.hpp>

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
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_MULTISAMPLE                    0x809D
#define GL_MULTISAMPLE_ARB                0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLES_ARB                    0x80A9
#define GL_SAMPLES_EXT                    0x80A9
#define GL_CLAMP_TO_EDGE                  0x812F
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

typedef void glGenFramebuffersType(GLsizei n, GLuint *ids);
typedef void glBindFramebufferType(GLenum target, GLuint framebuffer);
typedef void glFramebufferTexture2DType(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void glBlitFramebufferType(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void glClearBufferivType(GLenum buffer, GLint drawbuffer, GLint const *value);
typedef void glGenBuffersType(GLsizei n, GLuint *buffers);
typedef void glBindBufferType(GLenum target, GLuint buffer);
typedef void glBufferDataType(GLenum target, intptr size, const void *data, GLenum usage);
typedef void glGenVertexArraysType(GLsizei n, GLuint *arrays);
typedef void glBindVertexArrayType(GLuint array);
typedef void glVertexAttribPointerType(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void glEnableVertexAttribArrayType(GLuint index);
typedef GLuint glCreateShaderType(GLenum shaderType);
typedef void glShaderSourceType(GLuint shader, GLsizei count, char const **string, GLint const *length);
typedef void glCompileShaderType(GLuint shader);
typedef GLuint glCreateProgramType(void);
typedef void glAttachShaderType(GLuint program, GLuint shader);
typedef void glDetachShaderType(GLuint program, GLuint shader);
typedef void glLinkProgramType( GLuint program);
typedef void glUseProgramType(GLuint program);
typedef void glGetShaderivType(GLuint shader, GLenum pname, GLint *params);
typedef void glGetShaderInfoLogType(GLuint shader, GLsizei maxLength, GLsizei *length, char *infoLog);
typedef void glDeleteShaderType(GLuint shader);
typedef void glValidateProgramType(GLuint program);
typedef void glGetProgramivType(GLuint program, GLenum pname, GLint *params);
typedef GLint glGetUniformLocationType(GLuint program, char const *uniform_name);
typedef void glUniformMatrix4fvType(int32 location, isize count, bool transpose, float32 const *value);
typedef void glTexImage2DMultisampleType(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef const char *WINAPI wglGetExtensionsStringARBType(HDC hdc);
typedef BOOL WINAPI wglChoosePixelFormatARBType(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC WINAPI wglCreateContextAttribsARBType(HDC hdc, HGLRC hShareContext, int const *attribList);
typedef BOOL wglSwapIntervalEXTType(int interval);
typedef int wglGetSwapIntervalEXTType(void);

GLOBAL glGenFramebuffersType *glGenFramebuffers;
GLOBAL glBindFramebufferType *glBindFramebuffer;
GLOBAL glFramebufferTexture2DType *glFramebufferTexture2D;
GLOBAL glBlitFramebufferType *glBlitFramebuffer;
GLOBAL glClearBufferivType *glClearBufferiv;
GLOBAL glGenBuffersType *glGenBuffers;
GLOBAL glBindBufferType *glBindBuffer;
GLOBAL glBufferDataType *glBufferData;
GLOBAL glGenVertexArraysType *glGenVertexArrays;
GLOBAL glBindVertexArrayType *glBindVertexArray;
GLOBAL glVertexAttribPointerType *glVertexAttribPointer;
GLOBAL glEnableVertexAttribArrayType *glEnableVertexAttribArray;
GLOBAL glCreateShaderType *glCreateShader;
GLOBAL glShaderSourceType *glShaderSource;
GLOBAL glCompileShaderType *glCompileShader;
GLOBAL glCreateProgramType *glCreateProgram;
GLOBAL glAttachShaderType *glAttachShader;
GLOBAL glDetachShaderType *glDetachShader;
GLOBAL glLinkProgramType *glLinkProgram;
GLOBAL glUseProgramType *glUseProgram;
GLOBAL glGetShaderivType *glGetShaderiv;
GLOBAL glGetShaderInfoLogType *glGetShaderInfoLog;
GLOBAL glDeleteShaderType *glDeleteShader;
GLOBAL glValidateProgramType *glValidateProgram;
GLOBAL glGetProgramivType *glGetProgramiv;
GLOBAL glGetUniformLocationType *glGetUniformLocation;
GLOBAL glUniformMatrix4fvType *glUniformMatrix4fv;
GLOBAL glTexImage2DMultisampleType *glTexImage2DMultisample;
GLOBAL wglGetExtensionsStringARBType *wglGetExtensionsStringARB;
GLOBAL wglChoosePixelFormatARBType *wglChoosePixelFormatARB;
GLOBAL wglCreateContextAttribsARBType *wglCreateContextAttribsARB;
GLOBAL wglSwapIntervalEXTType *wglSwapIntervalEXT;
GLOBAL wglGetSwapIntervalEXTType *wglGetSwapIntervalEXT;


void InitializeOpenGLFunctions()
{
    glGenFramebuffers = (glGenFramebuffersType *) wglGetProcAddress("glGenFramebuffers");
    glBindFramebuffer = (glBindFramebufferType *) wglGetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D = (glFramebufferTexture2DType *) wglGetProcAddress("glFramebufferTexture2D");
    glBlitFramebuffer = (glBlitFramebufferType *) wglGetProcAddress("glBlitFramebuffer");
    glClearBufferiv = (glClearBufferivType *) wglGetProcAddress("glClearBufferiv");
    glGenBuffers = (glGenBuffersType *) wglGetProcAddress("glGenBuffers");
    glBindBuffer = (glBindBufferType *) wglGetProcAddress("glBindBuffer");
    glBufferData = (glBufferDataType *) wglGetProcAddress("glBufferData");
    glGenVertexArrays = (glGenVertexArraysType *) wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (glBindVertexArrayType *) wglGetProcAddress("glBindVertexArray");
    glVertexAttribPointer = (glVertexAttribPointerType *) wglGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (glEnableVertexAttribArrayType *) wglGetProcAddress("glEnableVertexAttribArray");
    glCreateShader = (glCreateShaderType *) wglGetProcAddress("glCreateShader");
    glShaderSource = (glShaderSourceType *) wglGetProcAddress("glShaderSource");
    glCompileShader = (glCompileShaderType *) wglGetProcAddress("glCompileShader");
    glCreateProgram = (glCreateProgramType *) wglGetProcAddress("glCreateProgram");
    glAttachShader = (glAttachShaderType *) wglGetProcAddress("glAttachShader");
    glDetachShader = (glDetachShaderType *) wglGetProcAddress("glDetachShader");
    glLinkProgram = (glLinkProgramType *) wglGetProcAddress("glLinkProgram");
    glUseProgram = (glUseProgramType *) wglGetProcAddress("glUseProgram");
    glGetShaderiv = (glGetShaderivType *) wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (glGetShaderInfoLogType *) wglGetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (glDeleteShaderType *) wglGetProcAddress("glDeleteShader");
    glValidateProgram = (glValidateProgramType *) wglGetProcAddress("glValidateProgram");
    glGetProgramiv = (glGetProgramivType *) wglGetProcAddress("glGetProgramiv");
    glGetUniformLocation = (glGetUniformLocationType *) wglGetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv = (glUniformMatrix4fvType *) wglGetProcAddress("glUniformMatrix4fv");
    glTexImage2DMultisample = (glTexImage2DMultisampleType *) wglGetProcAddress("glTexImage2DMultisample");
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
    result._34 = -1.0f;
    result._43 = -2.0f * f * n / (f - n);

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
    result._34 = -1.0f;
    result._43 = -2.0f * f * n / (f - n);

    return result;
}

matrix4 make_orthographic_matrix(float32 w, float32 h, float32 n, float32 f)
{
    matrix4 result = {};

    result._11 = 2.0f / w;
    result._22 = 2.0f / h;
    result._33 = -2.0f / (f - n);
    result._43 = -(f + n) / (f - n);
    result._44 = 1.0f;

    return result;
}

matrix4 make_orthographic_matrix(float32 aspect_ratio, float32 n, float32 f)
{
    matrix4 result;

    result._11 = 1.0f;
    result._22 = 1.0f * aspect_ratio;
    result._33 = -2.0f / (f - n);
    result._43 = -(f + n) / (f - n);
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


struct Viewport
{
    uint32 offset_x;
    uint32 offset_y;
    uint32 width;
    uint32 height;
};


Viewport get_viewport_size(uint32 width, uint32 height, float32 desired_aspect_ratio)
{
    Viewport result;

    float32 aspect_ratio = f32(width) / f32(height);
    if (aspect_ratio < desired_aspect_ratio)
    {
        result.width    = width;
        result.height   = uint32(result.width / desired_aspect_ratio);
        result.offset_x = 0;
        result.offset_y = (height - result.height) / 2;
    }
    else if (aspect_ratio > desired_aspect_ratio)
    {
        result.height   = CurrentClientHeight;
        result.width    = uint32(result.height * desired_aspect_ratio);
        result.offset_x = (width - result.width) / 2;
        result.offset_y = 0;
    }
    else
    {
        result.width    = width;
        result.height   = height;
        result.offset_x = 0;
        result.offset_y = 0;
    }

    return result;
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


struct RenderTarget
{
    uint32 id;
    uint32 backbuffer_texture;
    uint32 depth_stencil_texture;

    uint32 width;
    uint32 height;
    int32  num_samples;
};


RenderTarget make_render_target(uint32 width, uint32 height, int32 num_samples = 1)
{
    uint32 textures[2] = {};
    glGenTextures(2, textures);

    uint32 framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);

    if (num_samples > 1)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textures[0]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_RGBA8, width, height, false);
        GL_CHECK_ERRORS();

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textures[1]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_DEPTH24_STENCIL8, width, height, false);
        GL_CHECK_ERRORS();

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textures[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, textures[1], 0);
        GL_CHECK_ERRORS();
    }
    else if (num_samples == 1)
    {
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
        GL_CHECK_ERRORS();

        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        GL_CHECK_ERRORS();

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textures[1], 0);
        GL_CHECK_ERRORS();
    }
    else
    {
        INVALID_CODE_PATH();
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    GL_CHECK_ERRORS();

    RenderTarget result = {};
    result.id = framebuffer;
    result.backbuffer_texture = textures[0];
    result.depth_stencil_texture = textures[1];
    result.width = width;
    result.height = height;
    result.num_samples = num_samples;

    return result;
}


void bind_render_target(RenderTarget *rt)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt->id);
}


void blit_render_target(RenderTarget *rt)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, rt->id);

    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, rt->width, rt->height, 0, 0, rt->width, rt->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    GL_CHECK_ERRORS();
}


void resize_render_target(RenderTarget *fbo, uint32 width, uint32 height)
{
    if (fbo->num_samples > 1)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->backbuffer_texture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fbo->num_samples, GL_RGBA8, width, height, false);
        GL_CHECK_ERRORS();

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->depth_stencil_texture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fbo->num_samples, GL_DEPTH24_STENCIL8, width, height, false);
        GL_CHECK_ERRORS();
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, fbo->backbuffer_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
        GL_CHECK_ERRORS();

        glBindTexture(GL_TEXTURE_2D, fbo->depth_stencil_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        GL_CHECK_ERRORS();
    }

    fbo->width  = width;
    fbo->height = height;
}


struct Shader
{
    uint32 id;
    uint32 vertex_shader;
    uint32 fragment_shader;
};


Shader link_shader(uint32 vs, uint32 fs)
{
    auto id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);
    glDetachShader(id, vs);
    glDetachShader(id, fs);

    GL_CHECK_ERRORS();

    Shader result = {};
    if (is_shader_program_valid(id))
    {
        result.id = id;
        result.vertex_shader = vs;
        result.fragment_shader = fs;
    }
    else
    {
        // @todo: process error
    }

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
                wglGetExtensionsStringARB = (wglGetExtensionsStringARBType *) wglGetProcAddress("wglGetExtensionsStringARB");
                wglChoosePixelFormatARB = (wglChoosePixelFormatARBType *) wglGetProcAddress("wglChoosePixelFormatARB");
                wglCreateContextAttribsARB = (wglCreateContextAttribsARBType *) wglGetProcAddress("wglCreateContextAttribsARB");
                // @todo Check if 'WGL_EXT_swap_control' extension is available
                wglSwapIntervalEXT = (wglSwapIntervalEXTType *) wglGetProcAddress("wglSwapIntervalEXT");
                wglGetSwapIntervalEXT = (wglGetSwapIntervalEXTType *) wglGetProcAddress("wglGetSwapIntervalEXT");

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
        osOutputDebugString("Vendor:     %s\n", glGetString(GL_VENDOR));
        osOutputDebugString("Renderer:   %s\n", glGetString(GL_RENDERER));
        osOutputDebugString("GL Version: %s\n", glGetString(GL_VERSION));

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

    int32 num_samples = 16;
    RenderTarget framebuffer = make_render_target(CurrentClientWidth, CurrentClientHeight, num_samples);

    osOutputDebugString("Number of samples: %d\n", num_samples);

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

    Bitmap wisp_bitmap = load_png_file("../data/familiar.png");
    uint32 wisp_texture = 0;
    {
        glGenTextures(1, &wisp_texture);
        glBindTexture(GL_TEXTURE_2D, wisp_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wisp_bitmap.width, wisp_bitmap.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, wisp_bitmap.pixels);
        GL_CHECK_ERRORS();
    }

    char const *vs_source = R"GLSL(
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

    char const *cube_vs_source = R"GLSL(
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

    char const *fs_source = R"GLSL(
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

    auto plane_vs = compile_shader(vs_source, GL_VERTEX_SHADER);
    auto cube_vs = compile_shader(cube_vs_source, GL_VERTEX_SHADER);
    auto fs = compile_shader(fs_source, GL_FRAGMENT_SHADER);

    auto plane_shader = link_shader(plane_vs, fs);
    auto cube_shader = link_shader(cube_vs, fs);

    glDeleteShader(plane_vs);
    glDeleteShader(cube_vs);
    glDeleteShader(fs);

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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    Viewport viewport = {};

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
            resize_render_target(&framebuffer, CurrentClientWidth, CurrentClientHeight);

            viewport = get_viewport_size(CurrentClientWidth, CurrentClientHeight, DesiredAspectRatio);
            glViewport(viewport.offset_x, viewport.offset_y, viewport.width, viewport.height);

            ViewportNeedsResize = false;
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.id);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        glUseProgram(plane_shader.id);

        int32 u_view_location = glGetUniformLocation(plane_shader.id, "u_view");
        glUniformMatrix4fv(u_view_location, 1, GL_FALSE, matrix4::identity.get_data());

        int32 u_projection_location = glGetUniformLocation(plane_shader.id, "u_projection");
        glUniformMatrix4fv(u_projection_location, 1, GL_FALSE, matrix4::identity.get_data());

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

        glUniformMatrix4fv(u_view_location, 1, GL_FALSE, view.get_data());
        glUniformMatrix4fv(u_projection_location, 1, GL_FALSE, projection.get_data());

        glBindVertexArray(vertex_array_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
        glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indices), GL_UNSIGNED_INT, NULL);

        glUseProgram(cube_shader.id);

        u_view_location = glGetUniformLocation(cube_shader.id, "u_view");
        glUniformMatrix4fv(u_view_location, 1, GL_FALSE, view.get_data());
        u_projection_location = glGetUniformLocation(cube_shader.id, "u_projection");
        glUniformMatrix4fv(u_projection_location, 1, GL_FALSE, projection.get_data());

        glBindVertexArray(cube_vertex_array_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_index_buffer_id);
        glDrawElements(GL_TRIANGLES, ARRAY_COUNT(cube_indices), GL_UNSIGNED_INT, NULL);

        blit_render_target(&framebuffer);

        SwapBuffers(DeviceContext);

        os::timepoint WorkTimepoint = os::get_wall_clock();
        dtFromLastFrame = get_seconds(WorkTimepoint - LastClockTimepoint);
        LastClockTimepoint = os::get_wall_clock();
    }

    return 0;
}
