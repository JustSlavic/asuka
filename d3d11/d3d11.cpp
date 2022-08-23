#include <stdio.h>

#include <defines.hpp>
#include <math/color.hpp>

#include <windows.h>

// Direct 3D 11
#include <d3d11.h>
#include <d3dcompiler.h>
// #include <d3dx11.h>



struct Vertex
{
    f32 x, y, z;
    color32 color;
};

GLOBAL bool Running;
GLOBAL IDXGISwapChain *D3D11_SwapChain;
GLOBAL ID3D11Device *D3D11_Device;
GLOBAL ID3D11DeviceContext *D3D11_DeviceContext;
GLOBAL ID3D11RenderTargetView *D3D11_BackBuffer;


LRESULT CALLBACK MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result {};

    switch (message) {
        case WM_SIZE: {
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
                break;
            default: {
                DispatchMessageA(&Message);
                break;
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


int WINAPI WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CmdLine,
    int CmdShow)
{
    WNDCLASSA WindowClass {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "AsukaWindowClass";
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = NULL; // (HBRUSH) COLOR_WINDOW;

    ATOM ClassAtomResult = RegisterClassA(&WindowClass);
    if (!ClassAtomResult) {
        // Handle error
        return 1;
    }

    i32 ClientWidth = 800;
    i32 ClientHeight = 600;
    RECT WindowRectangle { 0, 0, ClientWidth, ClientHeight };
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
        CW_USEDEFAULT,                    // X,
        CW_USEDEFAULT,                    // Y,
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

    SwapChainDescription.BufferDesc.Width = ClientWidth;
    SwapChainDescription.BufferDesc.Height = ClientHeight;
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
        D3D_DRIVER_TYPE_HARDWARE, // D3D_DRIVER_TYPE_REFERENCE,
        NULL,
        NULL, // 0,
        NULL, // &FeatureLevel,
        NULL, // 1,
        D3D11_SDK_VERSION,
        &SwapChainDescription,
        &D3D11_SwapChain,
        &D3D11_Device,
        NULL, // &FeatureLevel,
        &D3D11_DeviceContext)))
    {
        return 1;
    }

    defer { D3D11_Device->Release(); };
    defer { D3D11_DeviceContext->Release(); };
    defer { D3D11_SwapChain->Release(); };

    // Render Target

    ID3D11Texture2D *pBackBuffer;
    D3D11_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    D3D11_Device->CreateRenderTargetView(pBackBuffer, NULL, &D3D11_BackBuffer);
    pBackBuffer->Release();

    D3D11_DeviceContext->OMSetRenderTargets(1, &D3D11_BackBuffer, NULL);

    defer { D3D11_BackBuffer->Release(); };

    // Viewport

    D3D11_VIEWPORT Viewport {};

    Viewport.TopLeftX = 0;
    Viewport.TopLeftY = 0;
    Viewport.Width = (f32) ClientWidth;
    Viewport.Height = (f32) ClientHeight;

    D3D11_DeviceContext->RSSetViewports(1, &Viewport);


    ID3DBlob *VS = NULL;
    ID3DBlob *PS = NULL;
    ID3DBlob *ShaderErrors = NULL;

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

    ID3D11VertexShader *VertexShader = NULL;
    D3D11_Device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &VertexShader);
    defer { VertexShader->Release(); };

    ID3D11PixelShader  *PixelShader = NULL;
    D3D11_Device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &PixelShader);
    defer { PixelShader->Release(); };

    D3D11_DeviceContext->VSSetShader(VertexShader, 0, 0);
    D3D11_DeviceContext->PSSetShader(PixelShader, 0, 0);

    ID3D11Buffer *VertexBuffer = NULL;

    Vertex Vertices[3] =
    {
        {  0.0f,   0.5f, 0.0f, color32{ 1.0f, 0.0f, 0.0f, 1.0f } },
        {  0.45f, -0.5f, 0.0f, color32{ 0.0f, 1.0f, 0.0f, 1.0f } },
        { -0.45f, -0.5f, 0.0f, color32{ 0.0f, 0.0f, 1.0f, 1.0f } },
    };

    D3D11_BUFFER_DESC BufferDescription {};

    BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
    BufferDescription.ByteWidth = sizeof(Vertex) * ARRAY_COUNT(Vertices);
    BufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_Device->CreateBuffer(&BufferDescription, NULL, &VertexBuffer);

    D3D11_MAPPED_SUBRESOURCE MappedSubresource {};
    D3D11_DeviceContext->Map(VertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
    memcpy(MappedSubresource.pData, Vertices, sizeof(Vertices));
    D3D11_DeviceContext->Unmap(VertexBuffer, NULL);

    D3D11_INPUT_ELEMENT_DESC InputDescription[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    ID3D11InputLayout *InputLayout = NULL;
    D3D11_Device->CreateInputLayout(InputDescription, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &InputLayout);
    D3D11_DeviceContext->IASetInputLayout(InputLayout);

    Running = true;
    int32 FrameCounter = 0;
    while (Running)
    {
        Win32_ProcessPendingMessages();

        // Clear the back buffer to a deep blue
        color32 BackgroundColor = { 0.0f, 0.2f, 0.4f, 1.0f };
        D3D11_DeviceContext->ClearRenderTargetView(D3D11_BackBuffer, BackgroundColor.e);

        // Do 3D rendering on the back buffer here
        u32 Stride = sizeof(Vertex);
        u32 Offset = 0;
        D3D11_DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
        D3D11_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D11_DeviceContext->Draw(3, 0);

        // Switch the back buffer and the front buffer
        D3D11_SwapChain->Present(0, 0);
    }

    return 0;
}
