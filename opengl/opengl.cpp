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
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
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
typedef void glActiveTextureType(GLenum texture);
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
typedef void glUniform1iType(GLint location, GLint v0);
typedef void glUniform1fType(GLint location, GLfloat v0);
typedef void glUniform2fType(GLint location, GLfloat v0, GLfloat v1);
typedef void glUniform3fType(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void glUniform4fType(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
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
GLOBAL glActiveTextureType *glActiveTexture;
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
GLOBAL glUniform1iType *glUniform1i;
GLOBAL glUniform1fType *glUniform1f;
GLOBAL glUniform2fType *glUniform2f;
GLOBAL glUniform3fType *glUniform3f;
GLOBAL glUniform4fType *glUniform4f;
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
    glActiveTexture = (glActiveTextureType *) wglGetProcAddress("glActiveTexture");
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
    glUniform1i = (glUniform1iType *) wglGetProcAddress("glUniform1i");
    glUniform1f = (glUniform1fType *) wglGetProcAddress("glUniform1f");
    glUniform2f = (glUniform2fType *) wglGetProcAddress("glUniform2f");
    glUniform3f = (glUniform3fType *) wglGetProcAddress("glUniform3f");
    glUniform4f = (glUniform4fType *) wglGetProcAddress("glUniform4f");
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

GLOBAL float32 dup;

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
                    if (VKCode == VK_UP)
                    {
                        if (IsDown) dup = 1.0f;
                        else dup = 0.0f;
                    }
                    if (VKCode == VK_DOWN)
                    {
                        if (IsDown) dup = -1.0f;
                        else dup = 0.0f;
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


void uniform(Shader shader, char const *name, float f)
{
    auto location = glGetUniformLocation(shader.id, name);
    glUniform1f(location, f);
}


void uniform(Shader shader, char const *name, vector2 const& v)
{
    auto location = glGetUniformLocation(shader.id, name);
    glUniform2f(location, v.x, v.y);
}


void uniform(Shader shader, char const *name, vector3 const& v)
{
    auto location = glGetUniformLocation(shader.id, name);
    glUniform3f(location, v.x, v.y, v.z);
}


void uniform(Shader shader, char const *name, vector4 const& v)
{
    auto location = glGetUniformLocation(shader.id, name);
    glUniform4f(location, v.x, v.y, v.z, v.w);
}


void uniform(Shader shader, char const *name, matrix4 const& m)
{
    auto location = glGetUniformLocation(shader.id, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, m.get_data());
}


struct Mesh
{
    array<vector3> vertices;
    array<vector2> texture_uvs;
    array<uint32>  indices;
};


[[nodiscard]]
Mesh load_wavefront_obj(byte_array contents, memory::mallocator *allocator)
{
    auto vertices = allocate_array<vector3>(allocator, 50);
    auto normals = allocate_array<vector3>(allocator, 50);
    auto uvs = allocate_array<vector2>(allocator, 50);

    auto result_vertices = allocate_array<vector3>(allocator, 10);
    auto result_uvs = allocate_array<vector2>(allocator, 10);
    auto result_indices = allocate_array<uint32>(allocator, 20);

    uint32 index = 0;

    while (index < contents.size)
    {
        auto byte = contents[index];
        if (byte == '#')
        {
            // Skip comment
            do
            {
                byte = contents[index++];
            }
            while (byte != '\n');
        }
        else if (byte == 'v')
        {
            index++; // eat 'v'
            byte = contents[index++]; // eat next symbol

            if (byte == 'n')
            {
                int n;
                float x, y, z;
                sscanf((char *) contents.get_data() + index, " %f %f %f%n", &x, &y, &z, &n);
                index += n;

                normals.push(make_vector3(x, y, z));
            }
            else if (byte == 't')
            {
                int n;
                float u, v;
                sscanf((char *) contents.get_data() + index, " %f %f%n", &u, &v, &n);
                index += n;

                uvs.push(make_vector2(u, v));
            }
            else if (byte == ' ')
            {
                int n;
                float x, y, z;
                sscanf((char *) contents.get_data() + index, " %f %f %f%n", &x, &y, &z, &n);
                index += n;

                vertices.push(make_vector3(x, y, z));
            }
            else
            {
                // Skip unrecognized line
                do
                {
                    byte = contents[index++];
                }
                while (byte != '\n');
            }
        }
        else if (byte == 'f')
        {
            index++;

            int n;
            int v1, v2, v3, v4;
            int t1, t2, t3, t4;
            int n1, n2, n3, n4;

            sscanf((char *) contents.data + index, " %d/%d/%d %d/%d/%d %d/%d/%d%n",
                &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3, &n);

            index += n;

            result_vertices.push(vertices[v1 - 1]);
            result_vertices.push(vertices[v2 - 1]);
            result_vertices.push(vertices[v3 - 1]);

            result_uvs.push(uvs[t1 - 1]);
            result_uvs.push(uvs[t2 - 1]);
            result_uvs.push(uvs[t3 - 1]);

            result_indices.push((uint32) result_vertices.size - 3);
            result_indices.push((uint32) result_vertices.size - 2);
            result_indices.push((uint32) result_vertices.size - 1);

            byte = contents[index];
            if (byte == ' ')
            {
                sscanf((char *) contents.data + index, " %d/%d/%d%n",
                    &v4, &t4, &n4, &n);
                index += n;

                result_vertices.push(vertices[v4 - 1]);
                result_uvs.push(uvs[t4 - 1]);

                result_indices.push((uint32) result_vertices.size - 2);
                result_indices.push((uint32) result_vertices.size - 1);
                result_indices.push((uint32) result_vertices.size - 4);
            }
        }
        else
        {
            // Skip unrecognized line
            do
            {
                byte = contents[index++];
            }
            while (byte != '\n');
        }
    }

    deallocate_array(vertices);
    deallocate_array(normals);
    deallocate_array(uvs);

    Mesh result = {};
    result.vertices = result_vertices;
    result.texture_uvs = result_uvs;
    result.indices = result_indices;

    return result;
}


int WINAPI WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CmdLine,
    int CmdShow)
{
    memory::mallocator mallocator;

    auto obj_contents = os::load_entire_file("../data/donut.obj");
    Mesh cube = load_wavefront_obj(obj_contents, &mallocator);

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

    wglSwapIntervalEXT(1); // VSync
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

    u32 cube_vertex_buffer_id = 0;
    {
        glGenBuffers(1, &cube_vertex_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, cube.vertices.size * sizeof(vector3), cube.vertices.data, GL_STATIC_DRAW);
    }
    u32 cube_texture_uv_buffer_id = 0;
    {
        glGenBuffers(1, &cube_texture_uv_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, cube_texture_uv_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, cube.texture_uvs.size * sizeof(vector2), cube.texture_uvs.data, GL_STATIC_DRAW);
    }
    u32 cube_index_buffer_id = 0;
    {
        glGenBuffers(1, &cube_index_buffer_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_index_buffer_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.indices.size * sizeof(uint32), cube.indices.data, GL_STATIC_DRAW);
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
                (void *) 0);       // Offset

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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wisp_bitmap.width, wisp_bitmap.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, wisp_bitmap.pixels);
        GL_CHECK_ERRORS();
    }

    char const *vs_1color = R"GLSL(
#version 400
#define square(x) (x*x)

layout (location = 0) in vec3 vertex_position;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(vertex_position, 1.0);
}
)GLSL";

    char const *fs_1color = R"GLSL(
#version 400

out vec4 result_color;

uniform vec4 u_color;

void main()
{
    result_color = u_color;
}
)GLSL";

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

    char const *cube_vs_source = R"GLSL(
#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 uv;

// out vec4 fragment_position;
out vec2 uv_coords;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    vec4 p = u_projection * u_view * u_model * vec4(vertex_position, 1.0);
    uv_coords = uv;

    gl_Position = p;
}
)GLSL";

    char const *cube_fs_source = R"GLSL(
#version 400

in vec2 uv_coords;
out vec4 result_color;

uniform sampler2D u_texture0;

void main()
{
    vec4 tex_color = texture(u_texture0, uv_coords);
    result_color = vec4(tex_color.rgb * tex_color.a, 1.0);
}
)GLSL";

    auto color_vs = compile_shader(vs_1color, GL_VERTEX_SHADER);
    auto color_fs = compile_shader(fs_1color, GL_FRAGMENT_SHADER);
    auto color_shader = link_shader(color_vs, color_fs);
    glDeleteShader(color_vs);
    glDeleteShader(color_fs);
    GL_CHECK_ERRORS();

    auto plane_vs = compile_shader(vs_source, GL_VERTEX_SHADER);
    auto plane_fs = compile_shader(fs_source, GL_FRAGMENT_SHADER);
    auto plane_shader = link_shader(plane_vs, plane_fs);
    glDeleteShader(plane_vs);
    glDeleteShader(plane_fs);
    GL_CHECK_ERRORS();

    auto cube_vs = compile_shader(cube_vs_source, GL_VERTEX_SHADER);
    auto cube_fs = compile_shader(cube_fs_source, GL_FRAGMENT_SHADER);
    auto cube_shader = link_shader(cube_vs, cube_fs);
    glDeleteShader(cube_vs);
    glDeleteShader(cube_fs);
    GL_CHECK_ERRORS();


    glUseProgram(cube_shader.id);
    int32 u_texture_location = glGetUniformLocation(cube_shader.id, "u_texture0");
    glUniform1i(u_texture_location, 0);
    GL_CHECK_ERRORS();

    float32 DesiredAspectRatio = 16.0f / 9.0f;

    float32 n = 0.05f;
    float32 f = 100.0f;

    auto perspective = make_projection_matrix_fov(to_radians(60), DesiredAspectRatio, n, f);
    auto orthographic = make_orthographic_matrix(4, 3, n, f);

    Camera camera = make_camera_at({0, 3, 3});

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
        f32 rot_x = 3.0f * cosf(circle_t);
        f32 rot_z = 3.0f * sinf(circle_t);

        camera.position.x = rot_x;
        camera.position.y += dtFromLastFrame * dup;
        camera.position.z = rot_z;

        circle_t += 0.5f * dtFromLastFrame;

        auto view = make_look_at_matrix(camera.position, make_vector3(0, 0, 0), make_vector3(0, 1, 0));

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

        // Draw Skybox-ish thing
        {
            glUseProgram(plane_shader.id);
            uniform(plane_shader, "u_view", matrix4::identity);
            uniform(plane_shader, "u_projection", matrix4::identity);

#if 0
            glBindVertexArray(vertex_array_id);
            glDrawArrays(GL_TRIANGLES, 0, 3);
#endif
            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glBindVertexArray(skybox_vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
            // glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indices), GL_UNSIGNED_INT, NULL);

            glEnable(GL_DEPTH_TEST);
        }

        if (Wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // Draw plane
        {
            glUseProgram(plane_shader.id);

            uniform(plane_shader, "u_view", view);
            uniform(plane_shader, "u_projection", projection);

            glBindVertexArray(vertex_array_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
            glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indices), GL_UNSIGNED_INT, NULL);
        }

        // Draw cube
        {
            glUseProgram(cube_shader.id);

            matrix4 model = matrix4::identity;
            scale(model, make_vector3(20));
            // rotate_z(model, to_radians(30.0f * circle_t));
            // rotate_y(model, to_radians(30.0f * circle_t));
            // rotate_x(model, to_radians(30.0f * circle_t));

            uniform(cube_shader, "u_model", model);
            uniform(cube_shader, "u_view", view);
            uniform(cube_shader, "u_projection", projection);

            glActiveTexture(GL_TEXTURE0); // Texture unit (slot)
            glBindTexture(GL_TEXTURE_2D, wisp_texture);

            glBindVertexArray(cube_vertex_array_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_index_buffer_id);
            glDrawElements(GL_TRIANGLES, (int32) cube.indices.size, GL_UNSIGNED_INT, NULL);
        }

        // Draw Axis
        {
            glUseProgram(color_shader.id);

            auto model = matrix4::identity;
            translate(model, make_vector3(0, 0, 1));

            scale(model, make_vector3(0.5, 0.01, 1));
            translate(model, make_vector3(0.5, 0, 0));

            uniform(color_shader, "u_model", model);
            uniform(color_shader, "u_view", view);
            uniform(color_shader, "u_projection", projection);

            uniform(color_shader, "u_color", make_vector4(1, 0, 0, 1));

            glBindVertexArray(vertex_array_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
            glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indices), GL_UNSIGNED_INT, NULL);

            rotate_z(model, to_radians(90));
            uniform(color_shader, "u_model", model);
            uniform(color_shader, "u_color", make_vector4(0, 1, 0, 1));
            glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indices), GL_UNSIGNED_INT, NULL);

            rotate_x(model, to_radians(90));
            rotate_y(model, to_radians(180));
            uniform(color_shader, "u_model", model);
            uniform(color_shader, "u_color", make_vector4(0, 0, 1, 1));
            glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indices), GL_UNSIGNED_INT, NULL);
        }

        blit_render_target(&framebuffer);

        SwapBuffers(DeviceContext);

        os::timepoint WorkTimepoint = os::get_wall_clock();
        dtFromLastFrame = get_seconds(WorkTimepoint - LastClockTimepoint);
        LastClockTimepoint = os::get_wall_clock();
    }

    return 0;
}
