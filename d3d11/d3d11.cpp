#include <stdio.h>

#include <defines.hpp>
#include <math/color.hpp>
#include <math/vector3.hpp>
#include <math/matrix4.hpp>

#include <windows.h>

// Direct 3D 11
#include <d3d11.h>
#include <d3dcompiler.h>
// #include <d3dx11.h>


#define RELEASE_COM(PTR) do { if ((PTR)) { (PTR)->Release(); } (PTR) = NULL; } while(0)


GLOBAL BOOL Running;
GLOBAL BOOL Wireframe;

GLOBAL UINT CurrentClientWidth;
GLOBAL UINT CurrentClientHeight;
GLOBAL BOOL ViewportNeedsResize;

GLOBAL ID3D11Device *D3D11_Device;
GLOBAL ID3D11DeviceContext *D3D11_DeviceContext;
GLOBAL IDXGISwapChain *D3D11_SwapChain;

GLOBAL ID3D11RenderTargetView *D3D11_BackBufferView;
GLOBAL ID3D11DepthStencilView *D3D11_DepthStencilView;

GLOBAL ID3D11RasterizerState* D3D11_RasterizerState;
GLOBAL ID3D11DepthStencilState *D3D11_DepthStencilState;


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


ViewportSize GetViewport(uint32 ClientWidth, uint32 ClientHeight, float32 DesiredAspectRatio)
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


int WINAPI WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CmdLine,
    int CmdShow)
{
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

    SwapChainDescription.SampleDesc.Count = 1;
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
        &D3D11_SwapChain,
        &D3D11_Device,
        NULL, // FeatureLevel
        &D3D11_DeviceContext)))
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "Could not create Direct3D 11 device and swap chain.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    // Setting the Rasterizer state
    {
        D3D11_RASTERIZER_DESC RasterizerDescription = {};
        RasterizerDescription.FillMode = D3D11_FILL_SOLID;
        RasterizerDescription.CullMode = D3D11_CULL_BACK;
        RasterizerDescription.FrontCounterClockwise = TRUE;
        RasterizerDescription.DepthClipEnable = TRUE;
        RasterizerDescription.MultisampleEnable = FALSE;

        HRESULT CreateRasterizerStateResult = D3D11_Device->CreateRasterizerState(&RasterizerDescription, &D3D11_RasterizerState);
        if (SUCCEEDED(CreateRasterizerStateResult))
        {
            D3D11_DeviceContext->RSSetState(D3D11_RasterizerState);
        }
    }

    // Setting the DepthStencil state
    {
        D3D11_DEPTH_STENCIL_DESC DepthStencilDescription = {};
        DepthStencilDescription.DepthEnable    = TRUE;
        DepthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        DepthStencilDescription.DepthFunc      = D3D11_COMPARISON_LESS;

        HRESULT CreateDepthStencilStateResult = D3D11_Device->CreateDepthStencilState(&DepthStencilDescription, &D3D11_DepthStencilState);
        if (SUCCEEDED(CreateDepthStencilStateResult))
        {
            D3D11_DeviceContext->OMSetDepthStencilState(D3D11_DepthStencilState, 0);
        }
    }

    char const Shader[] = R"HLSL(
struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
    VOut output;

    output.position = position;
    output.color = color;

    return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}
)HLSL";

    ID3DBlob *ShaderErrors = NULL;

    ID3DBlob *VS = NULL;
    HRESULT VertexShaderCompilationResult = D3DCompile(
        Shader,         // SrcData
        sizeof(Shader), // SrcDataSize
        NULL,           // SourceName
        NULL,           // Defines
        NULL,           // Include
        "VShader",      // Entrypoint
        "vs_4_0",       // Target,
        0,              // Flags1
        0,              // Flags2
        &VS,            // Code
        &ShaderErrors); // ErrorMsgs

    if (FAILED(VertexShaderCompilationResult))
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "Compilation HLSL error.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    ID3D11VertexShader *VertexShader = NULL;
    D3D11_Device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &VertexShader);

    ID3DBlob *PS = NULL;
    HRESULT PixelShaderCompilationResult = D3DCompile(
        Shader,         // SrcData
        sizeof(Shader), // SrcDataSize
        NULL,           // SourceName
        NULL,           // Defines
        NULL,           // Include
        "PShader",      // Entrypoint
        "ps_4_0",       // Target
        0,              // Flags1
        0,              // Flags2
        &PS,            // Code
        &ShaderErrors); // ErrorMsgs

    if (FAILED(PixelShaderCompilationResult))
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "Compilation HLSL error.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    ID3D11PixelShader  *PixelShader = NULL;
    D3D11_Device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &PixelShader);

    struct Vertex
    {
        vector3 position;
        color32 color;
    };

    Vertex Vertices[] =
    {
        { -1.0f, -1.0f, 0.1f, color32::blue },   // 0 bottom left
        {  1.0f, -1.0f, 0.1f, color32::green },  // 1 bottom right
        {  1.0f,  1.0f, 0.1f, color32::red },    // 2 top right
        { -1.0f,  1.0f, 0.1f, color32::yellow }, // 3 top left
    };

    ID3D11Buffer *VertexBuffer = NULL;
    {
        D3D11_BUFFER_DESC BufferDescription = {};

        BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescription.ByteWidth = sizeof(Vertices);
        BufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_Device->CreateBuffer(&BufferDescription, NULL, &VertexBuffer);

        D3D11_MAPPED_SUBRESOURCE MappedSubresource {};
        D3D11_DeviceContext->Map(VertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
        memcpy(MappedSubresource.pData, Vertices, sizeof(Vertices));
        D3D11_DeviceContext->Unmap(VertexBuffer, NULL);
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

        D3D11_Device->CreateBuffer(&BufferDescription, &IndexData, &IndexBuffer);
    }

    D3D11_INPUT_ELEMENT_DESC InputDescription[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    ID3D11InputLayout *InputLayout = NULL;
    D3D11_Device->CreateInputLayout(InputDescription, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &InputLayout);

    float32 DesiredAspectRatio = 16.0f / 9.0f;

    float32 n = 0.05f;
    float32 f = 100.0f;
    float32 l = -0.1f;
    float32 r = 0.1f;
    float32 t = 0.1f;
    float32 b = -0.1f;

    if (DesiredAspectRatio > 1.0f)
    {
        // Width is bigger than height
        l = -0.1f;
        r =  0.1f;
        t =  0.1f * (1.0f / DesiredAspectRatio);
        b = -0.1f * (1.0f / DesiredAspectRatio);
    }
    else if ((0.0f < DesiredAspectRatio) && (DesiredAspectRatio < 1.0f))
    {
        // Height is bigger than height
        l = -0.1f * (1.0f / DesiredAspectRatio);
        r =  0.1f * (1.0f / DesiredAspectRatio);
        t =  0.1f;
        b = -0.1f;
    }
    else if (DesiredAspectRatio == 1.0f)
    {
        l = -0.1f;
        r =  0.1f;
        t =  0.1f;
        b = -0.1f;
    }
    else
    {
        INVALID_CODE_PATH();
    }

    auto ProjectionMatrix = make_matrix4(
        2*n/(r - l), 0, (r + l)/(r - l), 0,
        0, 2*n/(t - b), (t + b)/(t - b), 0,
        0, 0, -(f + n)/(f - n), -2.0f*f*n/(f - n),
        0, 0, -1, 0);

    Running = TRUE;
    int32 FrameCounter = 0;
    while (Running)
    {
        Win32_ProcessPendingMessages();

        if (ViewportNeedsResize)
        {
            ViewportNeedsResize = FALSE;

            D3D11_DeviceContext->OMSetRenderTargets(0, NULL, NULL);
            RELEASE_COM(D3D11_BackBufferView);
            RELEASE_COM(D3D11_DepthStencilView);

            HRESULT ResizeBuffersResult = D3D11_SwapChain->ResizeBuffers(0, CurrentClientWidth, CurrentClientHeight, DXGI_FORMAT_UNKNOWN, 0);
            if (SUCCEEDED(ResizeBuffersResult))
            {
                ID3D11Texture2D *BackBufferTexture = NULL;
                HRESULT GetBufferResult = D3D11_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&BackBufferTexture);
                if (SUCCEEDED(GetBufferResult))
                {
                    HRESULT CreateRenderTargetViewResult = D3D11_Device->CreateRenderTargetView(BackBufferTexture, NULL, &D3D11_BackBufferView);
                    if (SUCCEEDED(CreateRenderTargetViewResult))
                    {
                        D3D11_TEXTURE2D_DESC DepthStencilTextureDescription = {};
                        BackBufferTexture->GetDesc(&DepthStencilTextureDescription);

                        DepthStencilTextureDescription.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
                        DepthStencilTextureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;

                        ID3D11Texture2D *DepthStencilTexture;
                        HRESULT DepthStencilCreationResult = D3D11_Device->CreateTexture2D(&DepthStencilTextureDescription, NULL, &DepthStencilTexture);
                        if (SUCCEEDED(DepthStencilCreationResult))
                        {
                            HRESULT DepthStencilViewCreationResult = D3D11_Device->CreateDepthStencilView(DepthStencilTexture, NULL, &D3D11_DepthStencilView);
                            if (SUCCEEDED(DepthStencilViewCreationResult))
                            {
                                // Ok.
                                D3D11_DeviceContext->OMSetRenderTargets(1, &D3D11_BackBufferView, D3D11_DepthStencilView);
                            }
                            else
                            {
                                MessageBoxA(0, "Failed to create Depth Stencil View", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
                                RELEASE_COM(D3D11_DepthStencilView);
                            }

                            RELEASE_COM(DepthStencilTexture);
                        }
                        else
                        {
                            MessageBoxA(0, "Failed to create DepthStencil texture.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
                        }
                        // D3D11_DeviceContext->OMSetRenderTargets(1, &D3D11_BackBufferView, D3D11_DepthStencilView);
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
            }
            else
            {
                MessageBoxA(0, "Failed to resize buffers.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
            }

            ViewportSize Viewport = GetViewport(CurrentClientWidth, CurrentClientHeight, DesiredAspectRatio);

            D3D11_VIEWPORT DxViewport;
            DxViewport.Width = float32(Viewport.Width);
            DxViewport.Height = float32(Viewport.Height);
            DxViewport.TopLeftX = float32(Viewport.OffsetX);
            DxViewport.TopLeftY = float32(Viewport.OffsetY);
            DxViewport.MinDepth = 0.0f;
            DxViewport.MaxDepth = 1.0f;
            
            D3D11_DeviceContext->RSSetViewports(1, &DxViewport);

            osOutputDebugString("(%u, %u) - (%u, %u)    Ratio - %4.2f (should be ~1.0)\n",
                Viewport.OffsetX, Viewport.OffsetY,
                Viewport.Width, Viewport.Height,
                float32(CurrentClientWidth - 2.0f*Viewport.OffsetX) / float32(CurrentClientHeight - 2.0f*Viewport.OffsetY) / DesiredAspectRatio);

            // D3D11_DeviceContext->OMSetRenderTargets(1, &D3D11_BackBufferView, D3D11_DepthStencilView);
            // D3D11_DeviceContext->OMSetDepthStencilState(D3D11_DepthStencilState, 0);
        }

        // Clear the back buffer to a deep blue
        color32 BackgroundColor = { 0.0f, 0.2f, 0.4f, 1.0f };
        D3D11_DeviceContext->ClearRenderTargetView(D3D11_BackBufferView, BackgroundColor.e);
        D3D11_DeviceContext->ClearDepthStencilView(D3D11_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Do 3D rendering on the back buffer here
        u32 Stride = sizeof(Vertex);
        u32 Offset = 0;

        D3D11_DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
        D3D11_DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        D3D11_DeviceContext->IASetInputLayout(InputLayout);

        D3D11_DeviceContext->VSSetShader(VertexShader, 0, 0);
        D3D11_DeviceContext->PSSetShader(PixelShader, 0, 0);

        D3D11_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        // D3D11_DeviceContext->Draw(3, 0);
        D3D11_DeviceContext->DrawIndexedInstanced(ARRAYSIZE(Indices), 2, 0, 0, 0);

        // Switch the back buffer and the front buffer
        D3D11_SwapChain->Present(0, 0);
    }

    RELEASE_COM(VertexShader);
    RELEASE_COM(PixelShader);

    RELEASE_COM(D3D11_DepthStencilState);
    RELEASE_COM(D3D11_RasterizerState);
    RELEASE_COM(D3D11_DepthStencilView);
    RELEASE_COM(D3D11_BackBufferView);
    RELEASE_COM(D3D11_SwapChain);
    RELEASE_COM(D3D11_DeviceContext);
    RELEASE_COM(D3D11_Device);

    return 0;
}
