#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#include <defines.hpp>
#include <math/color.hpp>
#include <os/memory.hpp>
#include <os/time.hpp>

#include <windows.h>

// Direct 3D 12
#include <d3d12.h>
#include <dxgi.h>
// #include <d3dx12.h>


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


INTERNAL INLINE
void Offset(D3D12_CPU_DESCRIPTOR_HANDLE *Handle, i32 OffsetInDescriptors, u32 DescriptorIncrementSize)
{
    Handle->ptr = usize(i64(Handle->ptr) + i64(OffsetInDescriptors) * i64(DescriptorIncrementSize));
}


[[nodiscard]] INTERNAL INLINE
D3D12_CPU_DESCRIPTOR_HANDLE InitOffseted(D3D12_CPU_DESCRIPTOR_HANDLE Base, i32 OffsetInDescriptors, u32 DescriptorIncrementSize)
{
    D3D12_CPU_DESCRIPTOR_HANDLE Result = Base;
    Offset(&Result, OffsetInDescriptors, DescriptorIncrementSize);

    return Result;
}


[[nodiscard]] INTERNAL INLINE
D3D12_RESOURCE_BARRIER Transition(
    ID3D12Resource* Resource,
    D3D12_RESOURCE_STATES StateBefore,
    D3D12_RESOURCE_STATES StateAfter,
    UINT Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
    D3D12_RESOURCE_BARRIER_FLAGS Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
{
    D3D12_RESOURCE_BARRIER Result;
    Result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Result.Flags = Flags;
    Result.Transition.pResource = Resource;
    Result.Transition.Subresource = Subresource;
    Result.Transition.StateBefore = StateBefore;
    Result.Transition.StateAfter = StateAfter;
    return Result;
}


void FlushCommandQueue(ID3D12CommandQueue *CommandQueue, ID3D12Fence *Fence)
{
    PERSIST u64 CurrentFence;
    // Advance the fence value to mark commands up to this fence point.
    CurrentFence++;

    // Add an instruction to the command queue to set a new fence point.  Because we
    // are on the GPU timeline, the new fence point won't be set until the GPU finishes
    // processing all the commands prior to this Signal().
    HRESULT SignalResult = CommandQueue->Signal(Fence, CurrentFence);
    ASSERT(SUCCEEDED(SignalResult));

    // Wait until the GPU has completed commands up to this fence point.
    if(Fence->GetCompletedValue() < CurrentFence)
    {
        HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

        // Fire event when GPU hits current fence.
        HRESULT SetEventResult = Fence->SetEventOnCompletion(CurrentFence, EventHandle);
        ASSERT(SUCCEEDED(SetEventResult));

        // Wait until the GPU hits current fence event is fired.
        WaitForSingleObject(EventHandle, INFINITE);
        CloseHandle(EventHandle);
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

    HWND Window = CreateWindowExA(
        0,
        WindowClass.lpszClassName,
        "D3D11 Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, // X
        CW_USEDEFAULT, // Y
        ClientWidth,
        ClientHeight,
        0, 0, Instance, 0);

    if (!Window) { return 1; }


#if ASUKA_DEBUG
    ID3D12Debug *D3D12_Debug = NULL;
    {
        // Activate Debugging capabilities
        if (SUCCEEDED(D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **) &D3D12_Debug)))
        {
            D3D12_Debug->EnableDebugLayer();
        }
    }

    defer { if (D3D12_Debug) D3D12_Debug->Release(); };
#endif

    // Initializating Direct 3D
    //   - Create ID3D12Device using D3D12CreateDevice function;
    //   - Create ID3D12Fence object and query descriptor sizes;
    //   - Check 4x MSAA quality level support;
    //   - Create command queue, command list allocatorm, and main command list;
    //   - Describe and create swap chain;
    //   - Create the descriptor heaps the application requires;
    //   - Resize the back buffer and create a render target view to the back buffer;
    //   - Create the depth/stencil buffer and its associated depth/stencil view;
    //   - Set the viewport and scissor rectangles.

    ID3D12Device *D3D12_Device = NULL;
    {
        HRESULT DeviceCreationResult = D3D12CreateDevice(
            NULL,                   // pAdapter (@note: NULL means we will use primary display adapter)
            D3D_FEATURE_LEVEL_11_0, // MinimumFeatureLevel
            __uuidof(ID3D12Device), // riid
            (void **) &D3D12_Device // ppDevice
        );

        ASSERT(SUCCEEDED(DeviceCreationResult));
    }

    defer { D3D12_Device->Release(); };

    ID3D12Fence *Fence = NULL;
    {
        HRESULT FenceCreationResult = D3D12_Device->CreateFence(
            0,                       // InitialValue
            D3D12_FENCE_FLAG_NONE,   // Flags
            __uuidof(ID3D12Fence),   // riid
            (void **) &Fence   // ppFence
        );

        ASSERT(SUCCEEDED(FenceCreationResult));
    }

    defer { Fence->Release(); };

    DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    u32 RtvDescriptorSize = D3D12_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    u32 DtvDescriptorSize = D3D12_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    u32 CbvDescriptorSize = D3D12_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    b32 D3D12_4xMSAA_Active = false;
    u32 D3D12_4xMSAA_QualityLevelCount = 0;
    {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSAA_QualityLevels;
        ZeroMemory(&MSAA_QualityLevels, sizeof(MSAA_QualityLevels));

        MSAA_QualityLevels.Format = BackBufferFormat;
        MSAA_QualityLevels.SampleCount = 4;
        MSAA_QualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        MSAA_QualityLevels.NumQualityLevels = 0;

        HRESULT MultisampleCheckingResult = D3D12_Device->CheckFeatureSupport(
            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
            (void *) &MSAA_QualityLevels,
            sizeof(MSAA_QualityLevels)
        );
        ASSERT(SUCCEEDED(MultisampleCheckingResult));

        ASSERT(MSAA_QualityLevels.NumQualityLevels > 0);
        D3D12_4xMSAA_QualityLevelCount = MSAA_QualityLevels.NumQualityLevels;
    }

    ID3D12CommandQueue *CommandQueue = NULL;
    ID3D12CommandAllocator *CommandAllocator = NULL;
    ID3D12GraphicsCommandList *CommandList = NULL;
    {
        D3D12_COMMAND_QUEUE_DESC QueueDescription;
        ZeroMemory(&QueueDescription, sizeof(QueueDescription));

        QueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        QueueDescription.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        QueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        QueueDescription.NodeMask = 0;

        HRESULT CommandQueueCreationResult = D3D12_Device->CreateCommandQueue(&QueueDescription, __uuidof(ID3D12CommandQueue), (void **) &CommandQueue);
        ASSERT(SUCCEEDED(CommandQueueCreationResult));

        HRESULT CommandAllocatorCreationResult = D3D12_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **) &CommandAllocator);
        ASSERT(SUCCEEDED(CommandAllocatorCreationResult));

        HRESULT CommandListCreationResult = D3D12_Device->CreateCommandList(
            0, // NodeMask
            D3D12_COMMAND_LIST_TYPE_DIRECT, // Type
            CommandAllocator, // pCommandAllocator
            NULL, // pInitialState
            __uuidof(ID3D12GraphicsCommandList), // riid
            (void **) &CommandList // ppCommandList
        );
        ASSERT(SUCCEEDED(CommandListCreationResult));

        CommandList->Close();
    }

    defer { CommandQueue->Release(); };
    defer { CommandAllocator->Release(); };
    defer { CommandList->Release(); };

    IDXGIFactory *DXGI_Factory = NULL;
    {
        HRESULT FactoryCreationResult = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **) &DXGI_Factory);
        ASSERT(SUCCEEDED(FactoryCreationResult));
    }

    constexpr u32 SwapChainBufferCount = 2;
    u32 CurrentBackBuffer = 0; // Track which buffer to render to

    IDXGISwapChain *SwapChain = NULL;
    {
        DXGI_SWAP_CHAIN_DESC SwapChainDescription;
        ZeroMemory(&SwapChainDescription, sizeof(SwapChainDescription));

        SwapChainDescription.BufferDesc.Width = ClientWidth;
        SwapChainDescription.BufferDesc.Height = ClientHeight;
        SwapChainDescription.BufferDesc.Format = BackBufferFormat;
        SwapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
        SwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
        SwapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

        SwapChainDescription.SampleDesc.Count = D3D12_4xMSAA_Active ? 4 : 1;
        SwapChainDescription.SampleDesc.Quality = D3D12_4xMSAA_Active ? D3D12_4xMSAA_QualityLevelCount - 1 : 0;

        SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDescription.BufferCount = SwapChainBufferCount;
        SwapChainDescription.OutputWindow = Window;
        SwapChainDescription.Windowed = true;
        SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        SwapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        HRESULT SwapChainCreationResult = DXGI_Factory->CreateSwapChain(CommandQueue, &SwapChainDescription, &SwapChain);
        ASSERT(SUCCEEDED(SwapChainCreationResult));
    }

    defer { SwapChain->Release(); };

    ID3D12DescriptorHeap *RtvDescriptorHeap = NULL;
    ID3D12DescriptorHeap *DsvDescriptorHeap = NULL;
    {
        D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDescription;
        ZeroMemory(&RtvHeapDescription, sizeof(RtvHeapDescription));

        RtvHeapDescription.NumDescriptors = SwapChainBufferCount;
        RtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        RtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        RtvHeapDescription.NodeMask = 0;

        HRESULT RtvHeapCreationResult = D3D12_Device->CreateDescriptorHeap(&RtvHeapDescription, __uuidof(ID3D12DescriptorHeap), (void **) &RtvDescriptorHeap);
        ASSERT(SUCCEEDED(RtvHeapCreationResult));

        D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDescription;
        ZeroMemory(&DsvHeapDescription, sizeof(DsvHeapDescription));

        DsvHeapDescription.NumDescriptors = 1;
        DsvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        DsvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        DsvHeapDescription.NodeMask = 0;

        HRESULT DsvHeapCreationResult = D3D12_Device->CreateDescriptorHeap(&DsvHeapDescription, __uuidof(ID3D12DescriptorHeap), (void **) &DsvDescriptorHeap);
        ASSERT(SUCCEEDED(DsvHeapCreationResult));
    }

    defer { RtvDescriptorHeap->Release(); };
    defer { DsvDescriptorHeap->Release(); };

    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView = InitOffseted(RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), CurrentBackBuffer, RtvDescriptorSize);
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentStencilBufferView = DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    ID3D12Resource *RenderTargetViews[SwapChainBufferCount] = {};
    {
        D3D12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle = RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

        for (i32 i = 0; i < SwapChainBufferCount; i++)
        {
            HRESULT GetSwapChainBufferResult = SwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **) &RenderTargetViews[i]);
            ASSERT(SUCCEEDED(GetSwapChainBufferResult));

            D3D12_Device->CreateRenderTargetView(RenderTargetViews[i], NULL, RtvHeapHandle);
            Offset(&RtvHeapHandle, 1, RtvDescriptorSize);
        }
    }

    defer {
        for (i32 i = 0; i < ARRAY_COUNT(RenderTargetViews); i++) {
            RenderTargetViews[i]->Release();
        }
    };

    ID3D12Resource *DepthStencilBuffer = NULL;
    {
        D3D12_RESOURCE_DESC DepthStencilDescription;
        ZeroMemory(&DepthStencilDescription, sizeof(DepthStencilDescription));

        DepthStencilDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        DepthStencilDescription.Alignment = 0;
        DepthStencilDescription.Width = ClientWidth;
        DepthStencilDescription.Height = ClientHeight;
        DepthStencilDescription.DepthOrArraySize = 1;
        DepthStencilDescription.MipLevels = 1;
        DepthStencilDescription.Format = DepthStencilFormat;
        DepthStencilDescription.SampleDesc.Count = D3D12_4xMSAA_Active ? 4 : 1;
        DepthStencilDescription.SampleDesc.Quality = D3D12_4xMSAA_Active ? (D3D12_4xMSAA_QualityLevelCount - 1) : 0;
        DepthStencilDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        DepthStencilDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


        D3D12_HEAP_PROPERTIES HeapProperties;
        ZeroMemory(&HeapProperties, sizeof(HeapProperties));

        HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProperties.CreationNodeMask = 1;
        HeapProperties.VisibleNodeMask = 1;

        D3D12_CLEAR_VALUE OptimizedClearValue;
        ZeroMemory(&OptimizedClearValue, sizeof(OptimizedClearValue));

        OptimizedClearValue.Format = DepthStencilFormat;
        OptimizedClearValue.DepthStencil.Depth = 1.0f;
        OptimizedClearValue.DepthStencil.Stencil = 0;

        HRESULT DepthStencilBufferCreationResult = D3D12_Device->CreateCommittedResource(
            &HeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &DepthStencilDescription,
            D3D12_RESOURCE_STATE_COMMON,
            &OptimizedClearValue,
            __uuidof(ID3D12Resource),
            (void **) &DepthStencilBuffer);

        ASSERT(SUCCEEDED(DepthStencilBufferCreationResult));

        // Create descriptor to mip level 0 of entire resource using the format of the resource.
        D3D12_Device->CreateDepthStencilView(DepthStencilBuffer, NULL, DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        D3D12_RESOURCE_BARRIER D3D12_ResourceBarrier = Transition(DepthStencilBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

        // Transition the resource from its initial state to be used as a depth buffer.
        CommandList->ResourceBarrier(1, &D3D12_ResourceBarrier);
    }

    FlushCommandQueue(CommandQueue, Fence);

    D3D12_VIEWPORT Viewport;
    D3D12_RECT ScissorRect;
    {
        ZeroMemory(&Viewport, sizeof(Viewport));

        Viewport.TopLeftX = 0;
        Viewport.TopLeftY = 0;
        Viewport.Width = f32(ClientWidth);
        Viewport.Height = f32(ClientHeight);
        Viewport.MinDepth = 0;
        Viewport.MaxDepth = 1;

        CommandList->RSSetViewports(1, &Viewport);

        ZeroMemory(&ScissorRect, sizeof(ScissorRect));

        ScissorRect.left = 0;
        ScissorRect.top = 0;
        ScissorRect.right = ClientWidth;
        ScissorRect.bottom = ClientHeight;

        CommandList->RSSetScissorRects(1, &ScissorRect);
    }

    f64 dts_for_average_fps[100] = {};
    f64 sum_for_average_fps = 0;
    u32 dt_current_index = 0;

    os::timepoint t_start = os::get_wall_clock();
    f64 dt = 0;

    char WindowText[256];

    HRESULT D3D_Result;
    Running = true;
    while (Running)
    {
        Win32_ProcessPendingMessages();

        memory::set(WindowText, 0, sizeof(WindowText));
        sprintf(WindowText, "D3D12: dt=%f (fps: %f)", dt, 100.0 / (sum_for_average_fps)); // 0.01 * [seconds / frame]

        SetWindowText(Window, WindowText);

        D3D_Result = CommandAllocator->Reset();
        ASSERT(SUCCEEDED(D3D_Result));

        D3D_Result = CommandList->Reset(CommandAllocator, NULL);
        ASSERT(SUCCEEDED(D3D_Result));

        D3D12_RESOURCE_BARRIER D3D12_ResourceBarrierToRenderTarget = Transition(RenderTargetViews[CurrentBackBuffer], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        CommandList->ResourceBarrier(1, &D3D12_ResourceBarrierToRenderTarget);

        CommandList->RSSetViewports(1, &Viewport);
        CommandList->RSSetScissorRects(1, &ScissorRect);

        Color32 BackgroundColor = { 0.0f, 0.2f, 0.4f, 1.0f };
        CommandList->ClearRenderTargetView(
            InitOffseted(RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), CurrentBackBuffer, RtvDescriptorSize),
            BackgroundColor.array_, 0, NULL);

        CommandList->ClearDepthStencilView(
            DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        D3D12_RESOURCE_BARRIER D3D12_ResourceBarrierToPresent = Transition(RenderTargetViews[CurrentBackBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        CommandList->ResourceBarrier(1, &D3D12_ResourceBarrierToPresent);

        CommandList->Close();
        CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList **) &CommandList);

        HRESULT PresentResult = SwapChain->Present(0, 0);
        ASSERT(SUCCEEDED(PresentResult));

        CurrentBackBuffer = (CurrentBackBuffer + 1) % SwapChainBufferCount;

        FlushCommandQueue(CommandQueue, Fence);

        Sleep(20);
        
        os::timepoint t_end = os::get_wall_clock();
        dt = os::get_seconds(t_end - t_start);

        sum_for_average_fps -= dts_for_average_fps[dt_current_index];
        sum_for_average_fps += dt;

        dts_for_average_fps[dt_current_index] = dt;
        dt_current_index = (dt_current_index + 1) % ARRAY_COUNT(dts_for_average_fps);

        t_start = t_end;
    }

    return 0;
}
