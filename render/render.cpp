#include "render.hpp"
#include "../fonts/Segoe-UI.cpp"
#include <dwmapi.h>

using namespace App;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Render::Start() {
    Data->wc = { sizeof(Data->wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"WinPatcher", nullptr };
    ::RegisterClassExW(&Data->wc);

    Data->hwnd = ::CreateWindowW(Data->wc.lpszClassName, L"WinPatcher", WS_OVERLAPPEDWINDOW, 100, 100, 1200, 750, nullptr, nullptr, Data->wc.hInstance, nullptr);

    BOOL USE_DARK_MODE = true;
    BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(
        Data->hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
        &USE_DARK_MODE, sizeof(USE_DARK_MODE)));

    // Initialize Direct3D
    if (!CreateDeviceD3D(Data->hwnd) || !SET_IMMERSIVE_DARK_MODE_SUCCESS)
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(Data->wc.lpszClassName, Data->wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(Data->hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(Data->hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.IniFilename = nullptr;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(Data->hwnd);
    ImGui_ImplDX12_Init(Data->g_pd3dDevice, Data->NUM_FRAMES_IN_FLIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM, Data->g_pd3dSrvDescHeap,
        Data->g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        Data->g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart()
    );

    Menu::RegFont = io.Fonts->AddFontFromMemoryTTF(segoe_ui, sizeof segoe_ui, 25.f);
    Menu::SubTFont = io.Fonts->AddFontFromMemoryTTF(segoe_ui, sizeof segoe_ui, 30.f);
    Menu::TitleFont = io.Fonts->AddFontFromMemoryTTF(segoe_ui, sizeof segoe_ui, 40.f);
    Menu::WindowHandle = Data->hwnd;
    

    /// font-based icons impl
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    Menu::IconsFont = io.Fonts->AddFontFromMemoryTTF(&font_awesome_binary, sizeof font_awesome_binary, 20.f, &icons_config, icon_ranges);

    return Cleanup();
}

void Render::AppLoop() {
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
            Menu::MenuActive = false;
    }

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    Menu::SetupStyle();

    Menu::RenderMenu();

    ImGui::Render();

    FrameContext* frameCtx = WaitForNextFrameResources();
    UINT backBufferIdx = Data->g_pSwapChain->GetCurrentBackBufferIndex();
    frameCtx->CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = Data->g_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    Data->g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
    Data->g_pd3dCommandList->ResourceBarrier(1, &barrier);

    // Render Dear ImGui graphics
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    Data->g_pd3dCommandList->ClearRenderTargetView(Data->g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
    Data->g_pd3dCommandList->OMSetRenderTargets(1, &Data->g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
    Data->g_pd3dCommandList->SetDescriptorHeaps(1, &Data->g_pd3dSrvDescHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), Data->g_pd3dCommandList);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    Data->g_pd3dCommandList->ResourceBarrier(1, &barrier);
    Data->g_pd3dCommandList->Close();

    Data->g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&Data->g_pd3dCommandList);

    Data->g_pSwapChain->Present(1, 0); // Present with vsync
    //g_pSwapChain->Present(0, 0); // Present without vsync

    UINT64 fenceValue = Data->g_fenceLastSignaledValue + 1;
    Data->g_pd3dCommandQueue->Signal(Data->g_fence, fenceValue);
    Data->g_fenceLastSignaledValue = fenceValue;
    frameCtx->FenceValue = fenceValue;
}

bool Render::Cleanup() {
    while (Menu::MenuActive)
        AppLoop();

    WaitForLastSubmittedFrame();

    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(Data->hwnd);
    ::UnregisterClassW(Data->wc.lpszClassName, Data->wc.hInstance);

    return 0;
}

void Render::CreateRenderTarget()
{
    for (UINT i = 0; i < Data->NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        Data->g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        Data->g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, Data->g_mainRenderTargetDescriptor[i]);
        Data->g_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void Render::CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < Data->NUM_BACK_BUFFERS; i++)
        if (Data->g_mainRenderTargetResource[i]) { Data->g_mainRenderTargetResource[i]->Release(); Data->g_mainRenderTargetResource[i] = nullptr; }
}

void Render::WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &Data->g_frameContext[Data->g_frameIndex % Data->NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (Data->g_fence->GetCompletedValue() >= fenceValue)
        return;

    Data->g_fence->SetEventOnCompletion(fenceValue, Data->g_fenceEvent);
    WaitForSingleObject(Data->g_fenceEvent, INFINITE);
}

void Render::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (Data->g_pSwapChain) { Data->g_pSwapChain->SetFullscreenState(false, nullptr); Data->g_pSwapChain->Release(); Data->g_pSwapChain = nullptr; }
    if (Data->g_hSwapChainWaitableObject != nullptr) { CloseHandle(Data->g_hSwapChainWaitableObject); }
    for (UINT i = 0; i < Data->NUM_FRAMES_IN_FLIGHT; i++)
        if (Data->g_frameContext[i].CommandAllocator) { Data->g_frameContext[i].CommandAllocator->Release(); Data->g_frameContext[i].CommandAllocator = nullptr; }
    if (Data->g_pd3dCommandQueue) { Data->g_pd3dCommandQueue->Release(); Data->g_pd3dCommandQueue = nullptr; }
    if (Data->g_pd3dCommandList) { Data->g_pd3dCommandList->Release(); Data->g_pd3dCommandList = nullptr; }
    if (Data->g_pd3dRtvDescHeap) { Data->g_pd3dRtvDescHeap->Release(); Data->g_pd3dRtvDescHeap = nullptr; }
    if (Data->g_pd3dSrvDescHeap) { Data->g_pd3dSrvDescHeap->Release(); Data->g_pd3dSrvDescHeap = nullptr; }
    if (Data->g_fence) { Data->g_fence->Release(); Data->g_fence = nullptr; }
    if (Data->g_fenceEvent) { CloseHandle(Data->g_fenceEvent); Data->g_fenceEvent = nullptr; }
    if (Data->g_pd3dDevice) { Data->g_pd3dDevice->Release(); Data->g_pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
    IDXGIDebug1* pDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

bool Render::CreateDeviceD3D(HWND hWnd) {

    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = Data->NUM_BACK_BUFFERS;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
    ID3D12Debug* pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&Data->g_pd3dDevice)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != nullptr)
    {
        ID3D12InfoQueue* pInfoQueue = nullptr;
        Data->g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        pInfoQueue->Release();
        pdx12Debug->Release();
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = Data->NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (Data->g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&Data->g_pd3dRtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = Data->g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = Data->g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < Data->NUM_BACK_BUFFERS; i++)
        {
            Data->g_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (Data->g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&Data->g_pd3dSrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (Data->g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&Data->g_pd3dCommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < Data->NUM_FRAMES_IN_FLIGHT; i++)
        if (Data->g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Data->g_frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (Data->g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Data->g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&Data->g_pd3dCommandList)) != S_OK ||
        Data->g_pd3dCommandList->Close() != S_OK)
        return false;

    if (Data->g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Data->g_fence)) != S_OK)
        return false;

    Data->g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (Data->g_fenceEvent == nullptr)
        return false;

    {
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain1* swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(Data->g_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&Data->g_pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        Data->g_pSwapChain->SetMaximumFrameLatency(Data->NUM_BACK_BUFFERS);
        Data->g_hSwapChainWaitableObject = Data->g_pSwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

FrameContext* Render::WaitForNextFrameResources()
{
    UINT nextFrameIndex = Data->g_frameIndex + 1;
    Data->g_frameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = { Data->g_hSwapChainWaitableObject, nullptr };
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtx = &Data->g_frameContext[nextFrameIndex % Data->NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0) // means no fence was signaled
    {
        frameCtx->FenceValue = 0;
        Data->g_fence->SetEventOnCompletion(fenceValue, Data->g_fenceEvent);
        waitableObjects[1] = Data->g_fenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

LRESULT WINAPI Render::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (Data->g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            WaitForLastSubmittedFrame();
            CleanupRenderTarget();
            HRESULT result = Data->g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
            assert(SUCCEEDED(result) && "Failed to resize swapchain.");
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
