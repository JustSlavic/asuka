#include <stdio.h>

#include <defines.hpp>
#include <math/color.hpp>

#include <windows.h>

// Direct 3D 11
#include <d3d11.h>
#include <d3dcompiler.h>
// #include <d3dx11.h>

#pragma comment(lib, "d3d11.lib")
// #pragma comment(lib, "d3dx11.lib")
// #pragma comment(lib, "d3dx10.lib")



struct Vertex
{
    f32 x, y, z;
    Color32 color;
};


GLOBAL Vertex Vertices[3] =
{
    {  0.0f,   0.5f, 0.0f, Color32{ 1.0f, 0.0f, 0.0f, 1.0f } },
    {  0.45f, -0.5f, 0.0f, Color32{ 0.0f, 1.0f, 0.0f, 1.0f } },
    { -0.45f, -0.5f, 0.0f, Color32{ 0.0f, 0.0f, 1.0f, 1.0f } },
};

GLOBAL bool Running;
GLOBAL IDXGISwapChain *D3D11_SwapChain;
GLOBAL ID3D11Device *D3D11_Device;
GLOBAL ID3D11DeviceContext *D3D11_DeviceContext;
GLOBAL ID3D11RenderTargetView *D3D11_BackBuffer;


INTERNAL_FUNCTION
void Win32_InitDirect3D11(HWND Window, i32 Width, i32 Height) {
    DXGI_SWAP_CHAIN_DESC SwapChainDescription;
    ZeroMemory(&SwapChainDescription, sizeof(SwapChainDescription));
    SwapChainDescription.BufferCount = 1;
    SwapChainDescription.BufferDesc.Width = Width;
    SwapChainDescription.BufferDesc.Height = Height;
    SwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    // SwapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
    // SwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.OutputWindow = Window;
    SwapChainDescription.SampleDesc.Count = 1;
    // SwapChainDescription.SampleDesc.Quality = 0;
    SwapChainDescription.Windowed = TRUE;
    // SwapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Allow full-screen switching


    D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
    if(FAILED(D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE, // D3D_DRIVER_TYPE_REFERENCE,
        NULL,
        NULL, // 0,
        NULL, // &FeatureLevels,
        NULL, // 1,
        D3D11_SDK_VERSION,
        &SwapChainDescription,
        &D3D11_SwapChain,
        &D3D11_Device,
        NULL, // &FeatureLevel,
        &D3D11_DeviceContext)))
    {
        return;
    }

    // Render Target

    ID3D11Texture2D *pBackBuffer;
    D3D11_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    D3D11_Device->CreateRenderTargetView(pBackBuffer, NULL, &D3D11_BackBuffer);
    pBackBuffer->Release();

    D3D11_DeviceContext->OMSetRenderTargets(1, &D3D11_BackBuffer, NULL);

    // Viewport

    D3D11_VIEWPORT Viewport {};

    Viewport.TopLeftX = 0;
    Viewport.TopLeftY = 0;
    Viewport.Width = (f32) Width;
    Viewport.Height = (f32) Height;

    D3D11_DeviceContext->RSSetViewports(1, &Viewport);
}


INTERNAL_FUNCTION
void Win32_CleanDirect3D11() {
    D3D11_SwapChain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

    D3D11_SwapChain->Release();
    D3D11_BackBuffer->Release();
    D3D11_Device->Release();
    D3D11_DeviceContext->Release();
}


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
    // RECT ClientRectangle { 0, 0, ClientWidth, ClientHeight };
    // if (!AdjustWindowRect(&ClientRectangle, WS_OVERLAPPEDWINDOW, false)) {
    //     // @error: handle not correct window size ?
    //     return 1;
    // }

    HWND Window = CreateWindowExA(
        0,
        WindowClass.lpszClassName,
        "D3D11 Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        // WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE,
        CW_USEDEFAULT, // int X,
        CW_USEDEFAULT, // int Y,
        ClientWidth, // CW_USEDEFAULT, // int nWidth,
        ClientHeight, // CW_USEDEFAULT, // int nHeight,
        0, 0, Instance, 0);

    if (!Window) {
        // Handle error
        return 1;
    }

    Win32_InitDirect3D11(Window, ClientWidth, ClientHeight);

    ID3D11VertexShader *VertexShader;    // the vertex shader
    ID3D11PixelShader  *PixelShader;     // the pixel shader

    ID3DBlob *VS = NULL;
    ID3DBlob *PS = NULL;
    ID3DBlob *ShaderErrors = NULL;

    if (FAILED(D3DCompileFromFile(
        L"shaders.shader", // pFileName
        NULL,             // pDefines
        NULL,             // pInclude
        "VShader",        // pEntrypoint
        "vs_4_0",         // pTarget
        0,                // Flags1
        0,                // Flags2
        &VS,              // ppCode
        &ShaderErrors     // ppErrorMsgs
    )))
    {
        return 1;
    }

    if (FAILED(D3DCompileFromFile(
        L"shaders.shader", // pFileName
        NULL,             // pDefines
        NULL,             // pInclude
        "PShader",        // pEntrypoint
        "ps_4_0",         // pTarget
        0,                // Flags1
        0,                // Flags2
        &PS,              // ppCode
        &ShaderErrors     // ppErrorMsgs
    )))
    {
        return 1;
    }

    D3D11_Device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &VertexShader);
    D3D11_Device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &PixelShader);

    D3D11_DeviceContext->VSSetShader(VertexShader, 0, 0);
    D3D11_DeviceContext->PSSetShader(PixelShader, 0, 0);

    ID3D11Buffer *VertexBuffer = NULL;

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

    while (Running)
    {
        Win32_ProcessPendingMessages();

        // Clear the back buffer to a deep blue
        Color32 BackgroundColor = { 0.0f, 0.2f, 0.4f, 1.0f };
        D3D11_DeviceContext->ClearRenderTargetView(D3D11_BackBuffer, BackgroundColor.array_);

        // Do 3D rendering on the back buffer here
        u32 Stride = sizeof(Vertex);
        u32 Offset = 0;
        D3D11_DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
        D3D11_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D11_DeviceContext->Draw(3, 0);

        // Switch the back buffer and the front buffer
        D3D11_SwapChain->Present(0, 0);
    }

    VertexShader->Release();
    PixelShader->Release();

    Win32_CleanDirect3D11();

    return 0;
}
