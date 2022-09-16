#include <stdio.h>

#include <defines.hpp>
#include <math/color.hpp>
#include <math/vector3.hpp>
#include <math/matrix4.hpp>
#include <os/time.hpp>
#include <png.hpp>
#include <obj.hpp>

#include <windows.h>

// Direct 3D 11
#include <d3d11.h>
#include <d3dcompiler.h>
// #include <d3dx11.h>


#define RELEASE_COM(PTR) do { if ((PTR)) { (PTR)->Release(); } (PTR) = NULL; } while(0)


GLOBAL BOOL Running;
GLOBAL BOOL Wireframe;
GLOBAL BOOL IsPerspectiveProjection = TRUE;
GLOBAL BOOL ProjectionMatrixNeedsChange;
GLOBAL float32 inter_t;

GLOBAL UINT CurrentClientWidth;
GLOBAL UINT CurrentClientHeight;
GLOBAL BOOL ViewportNeedsResize;


char const *get_d3d11_error_string(HRESULT ec)
{
    switch (ec)
    {
        case D3D11_ERROR_FILE_NOT_FOUND: return "The file was not found.";
        case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: return "There are too many unique instances of a particular type of state object.";
        case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS: return "There are too many unique instances of a particular type of view object.";
        case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD: return "The first call to ID3D11DeviceContext::Map after either ID3D11Device::CreateDeferredContext or ID3D11DeviceContext::FinishCommandList per Resource was not D3D11_MAP_WRITE_DISCARD.";
        case E_FAIL: return "Attempted to create a device with the debug layer enabled and the layer is not installed.";
        case E_INVALIDARG: return "An invalid parameter was passed to the returning function.";
        case E_OUTOFMEMORY: return "Direct3D could not allocate sufficient memory to complete the call.";
        case E_NOTIMPL: return "The method call isn't implemented with the passed parameter combination.";
        case S_FALSE: return "Alternate success value, indicating a successful but nonstandard completion (the precise meaning depends on context).";
        case S_OK: return "No error occurred.";
    }

    return NULL;
}

char const *get_dxgi_error_string(HRESULT ec)
{
    switch (ec)
    {
        case DXGI_ERROR_ACCESS_DENIED: return "You tried to use a resource to which you did not have the required access privileges. This error is most typically caused when you write to a shared resource with read-only access.";
        case DXGI_ERROR_ACCESS_LOST: return "The desktop duplication interface is invalid. The desktop duplication interface typically becomes invalid when a different type of image is displayed on the desktop.";
        case DXGI_ERROR_ALREADY_EXISTS: return "The desired element already exists. This is returned by DXGIDeclareAdapterRemovalSupport if it is not the first time that the function is called.";
        case DXGI_ERROR_CANNOT_PROTECT_CONTENT: return "DXGI can't provide content protection on the swap chain. This error is typically caused by an older driver, or when you use a swap chain that is incompatible with content protection.";
        case DXGI_ERROR_DEVICE_HUNG: return "The application's device failed due to badly formed commands sent by the application. This is an design-time issue that should be investigated and fixed.";
        case DXGI_ERROR_DEVICE_REMOVED: return "The video card has been physically removed from the system, or a driver upgrade for the video card has occurred. The application should destroy and recreate the device. For help debugging the problem, call ID3D10Device::GetDeviceRemovedReason.";
        case DXGI_ERROR_DEVICE_RESET: return "The device failed due to a badly formed command. This is a run-time issue; The application should destroy and recreate the device.";
        case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return "The driver encountered a problem and was put into the device removed state.";
        case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: return "An event (for example, a power cycle) interrupted the gathering of presentation statistics.";
        case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "The application attempted to acquire exclusive ownership of an output, but failed because some other application (or device within the application) already acquired ownership.";
        case DXGI_ERROR_INVALID_CALL: return "The application provided invalid parameter data; this must be debugged and fixed before the application is released.";
        case DXGI_ERROR_MORE_DATA: return "The buffer supplied by the application is not big enough to hold the requested data.";
        case DXGI_ERROR_NAME_ALREADY_EXISTS: return "The supplied name of a resource in a call to IDXGIResource1::CreateSharedHandle is already associated with some other resource.";
        case DXGI_ERROR_NONEXCLUSIVE: return "A global counter resource is in use, and the Direct3D device can't currently use the counter resource.";
        case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: return "The resource or request is not currently available, but it might become available later.";
        case DXGI_ERROR_NOT_FOUND: return "When calling IDXGIObject::GetPrivateData, the GUID passed in is not recognized as one previously passed to IDXGIObject::SetPrivateData or IDXGIObject::SetPrivateDataInterface. When calling IDXGIFactory::EnumAdapters or IDXGIAdapter::EnumOutputs, the enumerated ordinal is out of range.";
        case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: return "Reserved";
        case DXGI_ERROR_REMOTE_OUTOFMEMORY: return "Reserved";
        case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE: return "The DXGI output (monitor) to which the swap chain content was restricted is now disconnected or changed.";
        case DXGI_ERROR_SDK_COMPONENT_MISSING: return "The operation depends on an SDK component that is missing or mismatched.";
        case DXGI_ERROR_SESSION_DISCONNECTED: return "The Remote Desktop Services session is currently disconnected.";
        case DXGI_ERROR_UNSUPPORTED: return "The requested functionality is not supported by the device or the driver.";
        case DXGI_ERROR_WAIT_TIMEOUT: return "The time-out interval elapsed before the next desktop frame was available.";
        case DXGI_ERROR_WAS_STILL_DRAWING: return "The GPU was busy at the moment when a call was made to perform an operation, and did not execute or schedule the operation.";
        case S_OK: return "The method succeeded without an error.";
    }

    return NULL;
}


LRESULT CALLBACK MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = {};

    switch (message)
    {
        case WM_SIZE:
        {
            CurrentClientWidth  = LOWORD(lParam);
            CurrentClientHeight = HIWORD(lParam);
            ViewportNeedsResize = TRUE;
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


matrix4 MakeProjectionMatrix(float32 w, float32 h, float32 n, float32 f)
{
    matrix4 Result = {};

    Result._11 = 2.0f * n / w;
    Result._22 = 2.0f * n / h;
    Result._33 = -f / (f - n);
    Result._34 = -1.0f;
    Result._43 = -f*n / (f - n);

    return Result;
}


matrix4 MakeProjectionMatrixFov(float32 fov, float32 aspect_ratio, float32 n, float32 f)
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
    result._33 = -f / (f - n);
    result._34 = -1.0f;
    result._43 = -f*n / (f - n);

    return result;
}


matrix4 MakeOrthographicMatrix(float32 w, float32 h, float32 n, float32 f)
{
    matrix4 result = {};

    result._11 = 2.0f / w;
    result._22 = 2.0f / h;
    result._33 = -1.0f / (f - n);
    result._43 = -n / (f - n);
    result._44 = 1.0f;

    return result;
}

matrix4 MakeOrthographicMatrix(float32 aspect_ratio, float32 n, float32 f)
{
    matrix4 result;

    result._11 = 1.0f;
    result._22 = 1.0f * aspect_ratio;
    result._33 = -1.0f / (f - n);
    result._43 = -n / (f - n);
    result._44 = 1.0f;

    return result;
}


struct Dx11
{
    ID3D11Device *Device;
    ID3D11DeviceContext *DeviceContext;
    IDXGISwapChain *SwapChain;
};


struct Shader
{
    ID3D11VertexShader *VertexShader;
    ID3D11PixelShader *PixelShader;
    ID3DBlob *VertexShaderBytecode;
    ID3DBlob *PixelShaderBytecode;
};


Shader compile_shader(Dx11& Dx, string_view source_code)
{
    ID3DBlob *ShaderErrors = NULL;

    ID3DBlob *VSByteCode = NULL;
    HRESULT VertexShaderCompilationResult = D3DCompile(
        source_code.data, // SrcData
        source_code.size, // SrcDataSize
        NULL,             // SourceName
        NULL,             // Defines
        NULL,             // Include
        "VShader",        // Entrypoint
        "vs_4_0",         // Target,
        0,                // Flags1
        0,                // Flags2
        &VSByteCode,      // Code
        &ShaderErrors);   // ErrorMsgs

    if (FAILED(VertexShaderCompilationResult))
    {
        char ErrorBuffer[1024] = {};
        sprintf(ErrorBuffer, "Vertex Shader compilation error:\n\n%.*s",
            (int) ShaderErrors->GetBufferSize(), (char *) ShaderErrors->GetBufferPointer());

        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, ErrorBuffer, "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        RELEASE_COM(ShaderErrors);
        return {};
    }

    ID3D11VertexShader *VertexShader = NULL;
    Dx.Device->CreateVertexShader(VSByteCode->GetBufferPointer(), VSByteCode->GetBufferSize(), NULL, &VertexShader);

    ID3DBlob *PSBytecode = NULL;
    HRESULT PixelShaderCompilationResult = D3DCompile(
        source_code.data, // SrcData
        source_code.size, // SrcDataSize
        NULL,             // SourceName
        NULL,             // Defines
        NULL,             // Include
        "PShader",        // Entrypoint
        "ps_4_0",         // Target
        0,                // Flags1
        0,                // Flags2
        &PSBytecode,      // Code
        &ShaderErrors);   // ErrorMsgs

    if (FAILED(PixelShaderCompilationResult))
    {
        char ErrorBuffer[1024] = {};
        sprintf(ErrorBuffer, "Pixel Shader compilation error:\n\n%.*s",
            (int) ShaderErrors->GetBufferSize(), (char *) ShaderErrors->GetBufferPointer());

        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, ErrorBuffer, "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);

        RELEASE_COM(VSByteCode);
        RELEASE_COM(VertexShader);
        RELEASE_COM(ShaderErrors);
        return {};
    }

    ID3D11PixelShader *PixelShader = NULL;
    Dx.Device->CreatePixelShader(PSBytecode->GetBufferPointer(), PSBytecode->GetBufferSize(), NULL, &PixelShader);

    Shader Result;
    Result.VertexShader = VertexShader;
    Result.PixelShader = PixelShader;
    Result.VertexShaderBytecode = VSByteCode;
    Result.PixelShaderBytecode = PSBytecode;

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
    result.forward = { 0, 0, 1 };
    result.up = { 0, 1, 0 };
    result.right = { 1, 0, 0 };
    return result;
}


struct RenderTarget
{
    ID3D11RenderTargetView *backbuffer;
    ID3D11DepthStencilView *depth_stencil;

    uint32 width;
    uint32 height;
    int32  num_samples;
};


RenderTarget MakeRenderTarget(Dx11 *Driver, uint32 width, uint32 height, int32 num_samples)
{
    RenderTarget result = {};

    ID3D11Texture2D *BackBufferTexture = NULL;
    HRESULT GetBufferResult = Driver->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&BackBufferTexture);
    if (SUCCEEDED(GetBufferResult))
    {
        ID3D11RenderTargetView *BackBufferView = NULL;
        HRESULT CreateRenderTargetViewResult = Driver->Device->CreateRenderTargetView(BackBufferTexture, NULL, &BackBufferView);
        if (SUCCEEDED(CreateRenderTargetViewResult))
        {
            D3D11_TEXTURE2D_DESC DepthStencilTextureDescription = {};
            BackBufferTexture->GetDesc(&DepthStencilTextureDescription);

            DepthStencilTextureDescription.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
            DepthStencilTextureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;

            ID3D11Texture2D *DepthStencilTexture;
            HRESULT DepthStencilCreationResult = Driver->Device->CreateTexture2D(&DepthStencilTextureDescription, NULL, &DepthStencilTexture);
            if (SUCCEEDED(DepthStencilCreationResult))
            {
                ID3D11DepthStencilView *DepthStencilView = NULL;
                HRESULT DepthStencilViewCreationResult = Driver->Device->CreateDepthStencilView(DepthStencilTexture, NULL, &DepthStencilView);
                if (SUCCEEDED(DepthStencilViewCreationResult))
                {
                    // Ok.
                    result.backbuffer = BackBufferView;
                    result.depth_stencil = DepthStencilView;
                    result.width = width;
                    result.height = height;
                    result.num_samples = num_samples;
                }
                else
                {
                    MessageBoxA(0, "Failed to create Depth Stencil View", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
                    RELEASE_COM(BackBufferView);
                }

                RELEASE_COM(DepthStencilTexture);
            }
            else
            {
                MessageBoxA(0, "Failed to create DepthStencil texture.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
                RELEASE_COM(BackBufferView);
            }
        }
        else
        {
            MessageBoxA(0, "Failed to create RenderTargetView.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        }

        RELEASE_COM(BackBufferTexture);
    }
    else
    {
        MessageBoxA(0, "Failed to get BackBuffer texture.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
    }

    return result;
}


void BindRenderTarget(Dx11 *Driver, RenderTarget *rt)
{
    Driver->DeviceContext->OMSetRenderTargets(1, &rt->backbuffer, rt->depth_stencil);
}


void ResizeRenderTarget(Dx11 *Driver, RenderTarget *rt, uint32 width, uint32 height)
{
    Driver->DeviceContext->OMSetRenderTargets(0, NULL, NULL);
    RELEASE_COM(rt->backbuffer);
    RELEASE_COM(rt->depth_stencil);

    HRESULT ResizeBuffersResult = Driver->SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(ResizeBuffersResult))
    {
        *rt = MakeRenderTarget(Driver, width, height, 4);
    }
    else
    {
        MessageBoxA(0, "Failed to resize buffers.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
    }
}


int WINAPI WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CmdLine,
    int CmdShow)
{
    memory::mallocator mallocator;

    auto wisp_bitmap = load_png_file("../data/familiar.png");

    auto obj_contents = os::load_entire_file("../data/donut.obj");
    auto cube = load_wavefront_obj(obj_contents, &mallocator);

    Dx11 Dx = {};

    ID3D11RasterizerState* D3D11_RasterizerState = NULL;
    ID3D11RasterizerState* D3D11_WireframeRasterizerState = NULL;

    ID3D11DepthStencilState *D3D11_DepthStencilState = NULL;
    ID3D11DepthStencilState *D3D11_DisabledDepthStencilState = NULL;

    int32 PrimaryMonitorWidth  = GetSystemMetrics(SM_CXSCREEN);
    int32 PrimaryMonitorHeight = GetSystemMetrics(SM_CYSCREEN);

    WNDCLASSA WindowClass {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
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
        "D3D11 Window",                   // WindowName
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

    DXGI_SWAP_CHAIN_DESC SwapChainDescription;
    ZeroMemory(&SwapChainDescription, sizeof(SwapChainDescription));

    SwapChainDescription.BufferDesc.Width = CurrentClientWidth;
    SwapChainDescription.BufferDesc.Height = CurrentClientHeight;
    SwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;

    SwapChainDescription.SampleDesc.Count = 4;
    SwapChainDescription.SampleDesc.Quality = 0;

    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.BufferCount = 2;
    SwapChainDescription.OutputWindow = Window;
    SwapChainDescription.Windowed = TRUE;
    // SwapChainDescription.SwapEffect =  //?
    // SwapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Allow full-screen switching

    D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
    if(FAILED(D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE, // DriverType
        NULL, // Software
        NULL, // Flags
        NULL, // FeatureLevels
        NULL, // FeatureLevelCount,
        D3D11_SDK_VERSION, // SDKVersion
        &SwapChainDescription,
        &Dx.SwapChain,
        &Dx.Device,
        NULL, // FeatureLevel
        &Dx.DeviceContext)))
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "Could not create Direct3D 11 device and swap chain.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    // Setting the Rasterizer state
    {
        D3D11_RASTERIZER_DESC RasterizerDescription = {};
        RasterizerDescription.FillMode = D3D11_FILL_SOLID;
        RasterizerDescription.CullMode = D3D11_CULL_NONE;
        RasterizerDescription.FrontCounterClockwise = TRUE;
        RasterizerDescription.DepthClipEnable = TRUE;
        RasterizerDescription.MultisampleEnable = FALSE;

        HRESULT CreateRasterizerStateResult = Dx.Device->CreateRasterizerState(&RasterizerDescription, &D3D11_RasterizerState);
        if (SUCCEEDED(CreateRasterizerStateResult))
        {
            Dx.DeviceContext->RSSetState(D3D11_RasterizerState);
        }


        RasterizerDescription.FillMode = D3D11_FILL_WIREFRAME;

        HRESULT CreateWireframeRasterizerStateResult = Dx.Device->CreateRasterizerState(&RasterizerDescription, &D3D11_WireframeRasterizerState);
        if (SUCCEEDED(CreateWireframeRasterizerStateResult))
        {
            // Ok.
        }
    }

    // Setting the DepthStencil state
    {
        D3D11_DEPTH_STENCIL_DESC DepthStencilDescription = {};

        HRESULT CreateDepthStencilStateResult = Dx.Device->CreateDepthStencilState(&DepthStencilDescription, &D3D11_DisabledDepthStencilState);
        if (SUCCEEDED(CreateDepthStencilStateResult))
        {
            // Ok.
        }

        DepthStencilDescription.DepthEnable    = TRUE;
        DepthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        DepthStencilDescription.DepthFunc      = D3D11_COMPARISON_LESS;

        CreateDepthStencilStateResult = Dx.Device->CreateDepthStencilState(&DepthStencilDescription, &D3D11_DepthStencilState);
        if (SUCCEEDED(CreateDepthStencilStateResult))
        {
            Dx.DeviceContext->OMSetDepthStencilState(D3D11_DepthStencilState, 0);
        }
    }

    char const ShaderCode1[] = R"HLSL(
cbuffer VsConstantBuffer : register(b0)
{
    float4x4 model_matrix;
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

struct VS_Input
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_Output VShader(VS_Input input)
{
    float4 p = mul(projection_matrix, mul(view_matrix, float4(input.position, 1.0)));

    VS_Output result;
    result.position = p;
    result.color = input.color;

    return result;
}

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}
)HLSL";

    Shader PlaneShader = compile_shader(Dx, ShaderCode1);

    struct Vertex
    {
        vector3 position;
        color32 color;
    };

    Vertex Vertices[] =
    {
        { -1.0f, -1.0f, -1.0f, color32::blue },   // 0 bottom left
        {  1.0f, -1.0f, -1.0f, color32::green },  // 1 bottom right
        {  1.0f,  1.0f, -1.0f, color32::red },    // 2 top right
        { -1.0f,  1.0f, -1.0f, color32::yellow }, // 3 top left
    };

    ID3D11Buffer *VertexBuffer = NULL;
    {
        D3D11_BUFFER_DESC BufferDescription = {};

        BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescription.ByteWidth = sizeof(Vertices);
        BufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        Dx.Device->CreateBuffer(&BufferDescription, NULL, &VertexBuffer);

        D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
        Dx.DeviceContext->Map(VertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
        memcpy(MappedSubresource.pData, Vertices, sizeof(Vertices));
        Dx.DeviceContext->Unmap(VertexBuffer, NULL);
    }

    ID3D11Buffer *SkyboxBuffer = NULL;
    {
        for (uint32 vertex_index = 0; vertex_index < ARRAY_COUNT(Vertices); vertex_index++)
        {
            Vertices[vertex_index].position.z = 0.0f;
            Vertices[vertex_index].color = make_color32(0, 0.2, 0.4, 1);
        }

        D3D11_BUFFER_DESC BufferDescription = {};
        BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescription.ByteWidth = sizeof(Vertices);
        BufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        Dx.Device->CreateBuffer(&BufferDescription, NULL, &SkyboxBuffer);

        D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
        Dx.DeviceContext->Map(SkyboxBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
        memcpy(MappedSubresource.pData, Vertices, sizeof(Vertices));
        Dx.DeviceContext->Unmap(SkyboxBuffer, NULL);
    }

    u32 Indices[] =
    {
        0, 1, 2,
        2, 3, 0,
    };

    ID3D11Buffer *IndexBuffer = NULL;
    {
        D3D11_BUFFER_DESC BufferDescription = {};

        BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescription.ByteWidth = sizeof(Indices);
        BufferDescription.Usage     = D3D11_USAGE_IMMUTABLE;
        BufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA IndexData = { Indices };

        HRESULT CreateBufferResult = Dx.Device->CreateBuffer(&BufferDescription, &IndexData, &IndexBuffer);
        if (SUCCEEDED(CreateBufferResult))
        {
            // Ok.
        }
        else
        {
            // Not ok.
        }
    }

    ID3D11InputLayout *InputLayout = NULL;
    {
        D3D11_INPUT_ELEMENT_DESC InputDescription[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        Dx.Device->CreateInputLayout(InputDescription, 2, PlaneShader.VertexShaderBytecode->GetBufferPointer(), PlaneShader.VertexShaderBytecode->GetBufferSize(), &InputLayout);
    }

    // ================= cube =================

    char const CubeShaderCode1[] = R"HLSL(
cbuffer VsConstantBuffer : register(b0)
{
    float4x4 model_matrix;
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

struct VS_Input
{
    float3 position : POSITION;
    float2 uv : TEXTURE_UV;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float2 uv : TEXTURE_UV;
};

Texture2D    my_texture : register(t0);
SamplerState my_sampler : register(s0);

VS_Output VShader(float3 position : POSITION, float2 uv : TEXTURE_UV)
{
    float4 p = mul(projection_matrix, mul(view_matrix, mul(model_matrix, float4(position, 1.0))));

    VS_Output result;
    result.position = p;
    result.uv = uv;

    return result;
}

float4 PShader(float4 position : SV_POSITION, float2 uv : TEXTURE_UV) : SV_TARGET
{
    return my_texture.Sample(my_sampler, uv);
}
)HLSL";

    Shader CubeShader = compile_shader(Dx, CubeShaderCode1);

    ID3D11Buffer *CubeVertexBuffer = NULL;
    {
        D3D11_BUFFER_DESC BufferDescription = {};

        BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescription.ByteWidth = uint32(cube.vertices.size * sizeof(vector3));
        BufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT CreateBufferResult = Dx.Device->CreateBuffer(&BufferDescription, NULL, &CubeVertexBuffer);
        if (SUCCEEDED(CreateBufferResult))
        {
            D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
            Dx.DeviceContext->Map(CubeVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
            memcpy(MappedSubresource.pData, cube.vertices.data, uint32(cube.vertices.size * sizeof(vector3)));
            Dx.DeviceContext->Unmap(CubeVertexBuffer, NULL);
        }
    }

    ID3D11Buffer *CubeTextureUVBuffer = NULL;
    {
        D3D11_BUFFER_DESC BufferDescription = {};

        BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescription.ByteWidth = uint32(cube.texture_uvs.size * sizeof(vector2));
        BufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT CreateBufferResult = Dx.Device->CreateBuffer(&BufferDescription, NULL, &CubeTextureUVBuffer);
        if (SUCCEEDED(CreateBufferResult))
        {
            D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
            Dx.DeviceContext->Map(CubeTextureUVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
            memcpy(MappedSubresource.pData, cube.texture_uvs.data, uint32(cube.texture_uvs.size * sizeof(vector2)));
            Dx.DeviceContext->Unmap(CubeTextureUVBuffer, NULL);
        }
    }

    ID3D11Buffer *CubeIndexBuffer = NULL;
    {
        D3D11_BUFFER_DESC BufferDescription = {};

        BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescription.ByteWidth = uint32(cube.indices.size * sizeof(uint32));
        BufferDescription.Usage     = D3D11_USAGE_IMMUTABLE;
        BufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA IndexData = { cube.indices.data };

        HRESULT CreateBufferResult = Dx.Device->CreateBuffer(&BufferDescription, &IndexData, &CubeIndexBuffer);
    }

    ID3D11InputLayout *CubeInputLayout = NULL;
    {
        D3D11_INPUT_ELEMENT_DESC InputDescription[] =
        {
            { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXTURE_UV", 0, DXGI_FORMAT_R32G32_FLOAT,       1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        Dx.Device->CreateInputLayout(InputDescription, 2, CubeShader.VertexShaderBytecode->GetBufferPointer(), CubeShader.VertexShaderBytecode->GetBufferSize(), &CubeInputLayout);
    }

    // ================= cube =================

    // ================= texture =================

    ID3D11Texture2D *WispTexture = NULL;
    {
        D3D11_TEXTURE2D_DESC TextureDescription;
        TextureDescription.Width = wisp_bitmap.width;
        TextureDescription.Height = wisp_bitmap.height;
        TextureDescription.MipLevels = 1;
        TextureDescription.ArraySize = 1;
        TextureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        TextureDescription.SampleDesc.Count = 1;
        TextureDescription.SampleDesc.Quality = 0;
        TextureDescription.Usage = D3D11_USAGE_DYNAMIC;
        TextureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        TextureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        TextureDescription.MiscFlags = 0;

        Dx.Device->CreateTexture2D(&TextureDescription, NULL, &WispTexture);

        D3D11_MAPPED_SUBRESOURCE MappedSubresource;
        Dx.DeviceContext->Map(WispTexture, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
        memory::copy(MappedSubresource.pData, wisp_bitmap.pixels, wisp_bitmap.size);
        Dx.DeviceContext->Unmap(WispTexture, NULL);
    }

    ID3D11ShaderResourceView* WispTextureView;
    Dx.Device->CreateShaderResourceView(WispTexture, NULL, &WispTextureView);

    D3D11_SAMPLER_DESC SamplerDescription = {};
    SamplerDescription.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamplerDescription.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDescription.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDescription.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;

    ID3D11SamplerState* SamplerState;
    Dx.Device->CreateSamplerState(&SamplerDescription, &SamplerState);

    // ================= texture =================

    struct VertexShaderContants
    {
        matrix4 ModelMatrix;
        matrix4 ViewMatrix;
        matrix4 ProjectionMatrix;
    };

    VertexShaderContants VSConstants =
    {
        // transposed(make_look_at_matrix(camera.position, make_vector3(0, 0, 0), camera.up)),
        // transposed(ProjectionMatrix)
    };

    // Set VertexShader ConstantBuffer
    ID3D11Buffer *VsConstantBuffer = NULL;
    {
        // Fill in a buffer description.
        D3D11_BUFFER_DESC BufferDescription = {};
        BufferDescription.ByteWidth = sizeof(VertexShaderContants);
        BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA ConstantData = { &VSConstants };

        HRESULT CreateBufferResult = Dx.Device->CreateBuffer(&BufferDescription, &ConstantData, &VsConstantBuffer);
        if (SUCCEEDED(CreateBufferResult))
        {
            // Ok.
            Dx.DeviceContext->VSSetConstantBuffers(0, 1, &VsConstantBuffer);
        }
        else
        {
            // Not ok.
        }
    }

    float32 DesiredAspectRatio = 16.0f / 9.0f;

    float32 n = 0.05f;
    float32 f = 100.0f;

    auto PerspectiveProjection = MakeProjectionMatrixFov(to_radians(60), DesiredAspectRatio, n, f);
    auto OrthographicProjection = MakeOrthographicMatrix(4, 3, n, f);
    auto ProjectionMatrix = PerspectiveProjection;

    Camera camera = make_camera_at({0, 0, 3});

    RenderTarget MainRenderTarget = {};

    Running = TRUE;
    os::timepoint LastClockTimepoint = os::get_wall_clock();
    float32 dtFromLastFrame = 0.0f;

    while (Running)
    {
        Win32_ProcessPendingMessages();

        PERSIST f32 circle_t = 0.0f;
        f32 rot_x = 6.0f * cosf(circle_t);
        f32 rot_z = 6.0f * sinf(circle_t);

        camera.position.x = rot_x;
        camera.position.z = rot_z;

        circle_t += 0.5f * dtFromLastFrame;

        auto ViewMatrix = make_look_at_matrix(camera.position, make_vector3(0, 0, 0), camera.up);

        if (ProjectionMatrixNeedsChange)
        {
            ProjectionMatrix = IsPerspectiveProjection ? lerp(PerspectiveProjection, OrthographicProjection, inter_t) : lerp(OrthographicProjection, PerspectiveProjection, inter_t);
            inter_t -= dtFromLastFrame;
            if (inter_t < 0) ProjectionMatrixNeedsChange = false;
        }

        if (ViewportNeedsResize)
        {
            ViewportNeedsResize = FALSE;

            ResizeRenderTarget(&Dx, &MainRenderTarget, CurrentClientWidth, CurrentClientHeight);
            BindRenderTarget(&Dx, &MainRenderTarget);

            ViewportSize Viewport = GetViewportSize(CurrentClientWidth, CurrentClientHeight, DesiredAspectRatio);

            D3D11_VIEWPORT DxViewport;
            DxViewport.Width = float32(Viewport.Width);
            DxViewport.Height = float32(Viewport.Height);
            DxViewport.TopLeftX = float32(Viewport.OffsetX);
            DxViewport.TopLeftY = float32(Viewport.OffsetY);
            DxViewport.MinDepth = 0.0f;
            DxViewport.MaxDepth = 1.0f;

            Dx.DeviceContext->RSSetViewports(1, &DxViewport);

            osOutputDebugString("(%u, %u) - (%u, %u)    Ratio - %4.2f (should be ~1.0)\n",
                Viewport.OffsetX, Viewport.OffsetY,
                Viewport.Width, Viewport.Height,
                float32(CurrentClientWidth - 2.0f*Viewport.OffsetX) / float32(CurrentClientHeight - 2.0f*Viewport.OffsetY) / DesiredAspectRatio);
        }

        // Clear the back buffer to a black color
        auto BackgroundColor = color32::black;
        Dx.DeviceContext->ClearRenderTargetView(MainRenderTarget.backbuffer, color32::black.e);
        Dx.DeviceContext->ClearDepthStencilView(MainRenderTarget.depth_stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Do 3D rendering on the back buffer here
        u32 Stride = sizeof(Vertex);
        u32 Offset = 0;

        {
            Dx.DeviceContext->OMSetDepthStencilState(D3D11_DisabledDepthStencilState, 0);

            Dx.DeviceContext->IASetVertexBuffers(
                0, // Start slot
                1, // Num buffers
                &SkyboxBuffer, // Vertex Buffers
                &Stride, // Strides
                &Offset); // Offsets
            Dx.DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
            Dx.DeviceContext->IASetInputLayout(InputLayout);

            {
                D3D11_MAPPED_SUBRESOURCE MappedSubresource;

                Dx.DeviceContext->Map(VsConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
                auto *Constants = (VertexShaderContants *) MappedSubresource.pData;
                Constants->ModelMatrix = matrix4::identity;
                Constants->ViewMatrix = matrix4::identity;
                Constants->ProjectionMatrix = matrix4::identity;
                Dx.DeviceContext->Unmap(VsConstantBuffer, NULL);
            }

            Dx.DeviceContext->VSSetShader(PlaneShader.VertexShader, 0, 0);
            Dx.DeviceContext->PSSetShader(PlaneShader.PixelShader, 0, 0);
            Dx.DeviceContext->RSSetState(D3D11_RasterizerState);

            Dx.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            Dx.DeviceContext->DrawIndexedInstanced(ARRAY_COUNT(Indices), 1, 0, 0, 0);
        }

        if (Wireframe)
        {
            Dx.DeviceContext->RSSetState(D3D11_WireframeRasterizerState);
        }
        else
        {
            Dx.DeviceContext->RSSetState(D3D11_RasterizerState);
        }

        {
            Dx.DeviceContext->OMSetDepthStencilState(D3D11_DepthStencilState, 0);

            Dx.DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
            Dx.DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
            Dx.DeviceContext->IASetInputLayout(InputLayout);

            {
                D3D11_MAPPED_SUBRESOURCE MappedSubresource;

                Dx.DeviceContext->Map(VsConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
                auto *Constants = (VertexShaderContants *) MappedSubresource.pData;
                Constants->ModelMatrix = matrix4::identity;
                Constants->ViewMatrix = ViewMatrix;
                Constants->ProjectionMatrix = ProjectionMatrix;
                Dx.DeviceContext->Unmap(VsConstantBuffer, NULL);
            }

            Dx.DeviceContext->VSSetShader(PlaneShader.VertexShader, 0, 0);
            Dx.DeviceContext->PSSetShader(PlaneShader.PixelShader, 0, 0);

            Dx.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            // Dx.DeviceContext->Draw(3, 0);
            Dx.DeviceContext->DrawIndexedInstanced(ARRAY_COUNT(Indices), 1, 0, 0, 0);
        }

        {
            Dx.DeviceContext->IASetInputLayout(CubeInputLayout);

            ID3D11Buffer *Buffers[] = { CubeVertexBuffer, CubeTextureUVBuffer };
            u32 Strides[] = { sizeof(vector3), sizeof(vector2) };
            u32 Offsets[] = { 0, 0 };
            Dx.DeviceContext->IASetVertexBuffers(0, 2, Buffers, Strides, Offsets);
            Dx.DeviceContext->IASetIndexBuffer(CubeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

            auto model = matrix4::identity;
            scale(model, make_vector3(20));

            {
                D3D11_MAPPED_SUBRESOURCE MappedSubresource;

                Dx.DeviceContext->Map(VsConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
                auto *Constants = (VertexShaderContants *) MappedSubresource.pData;
                Constants->ModelMatrix = model;
                Constants->ViewMatrix = ViewMatrix;
                Constants->ProjectionMatrix = ProjectionMatrix;
                Dx.DeviceContext->Unmap(VsConstantBuffer, NULL);
            }

            Dx.DeviceContext->VSSetShader(CubeShader.VertexShader, 0, 0);
            Dx.DeviceContext->PSSetShader(CubeShader.PixelShader, 0, 0);
            Dx.DeviceContext->PSSetShaderResources(0, 1, &WispTextureView);
            Dx.DeviceContext->PSSetSamplers(0, 1, &SamplerState);

            Dx.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            Dx.DeviceContext->DrawIndexedInstanced(uint32(cube.indices.size), 1, 0, 0, 0);
        }

        // Switch the back buffer and the front buffer
        Dx.SwapChain->Present(0, 0);

        os::timepoint WorkTimepoint = os::get_wall_clock();
        dtFromLastFrame = get_seconds(WorkTimepoint - LastClockTimepoint);
        LastClockTimepoint = os::get_wall_clock();
    }

    RELEASE_COM(PlaneShader.VertexShader);
    RELEASE_COM(PlaneShader.PixelShader);
    RELEASE_COM(PlaneShader.VertexShaderBytecode);
    RELEASE_COM(PlaneShader.PixelShaderBytecode);

    RELEASE_COM(CubeShader.VertexShader);
    RELEASE_COM(CubeShader.PixelShader);
    RELEASE_COM(CubeShader.VertexShaderBytecode);
    RELEASE_COM(CubeShader.PixelShaderBytecode);

    RELEASE_COM(D3D11_DisabledDepthStencilState);
    RELEASE_COM(D3D11_DepthStencilState);

    RELEASE_COM(D3D11_WireframeRasterizerState);
    RELEASE_COM(D3D11_RasterizerState);

    RELEASE_COM(MainRenderTarget.backbuffer);
    RELEASE_COM(MainRenderTarget.depth_stencil);
    RELEASE_COM(Dx.SwapChain);
    RELEASE_COM(Dx.DeviceContext);
    RELEASE_COM(Dx.Device);

    return 0;
}
