#pragma once

/* ImGUI Headers */

#include "imgui/imgui.hpp"
#include "imgui/imconfig.hpp"
#include "imgui/imguiraw.hpp"
#include "imgui/imgui_internal.hpp"
#include "imgui/imstb_rectpack.hpp"
#include "imgui/imstb_textedit.hpp"
#include "imgui/imstb_truetype.hpp"
#include "backend/directX/imgui_impl_dx12.hpp"
#include "backend/win32/imgui_impl_win32.hpp"

/* DirectX Headers & Macros */

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <tchar.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER

#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

/* Windows Headers & Macros */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <stdio.h>
#include <iostream>

/* Type Definitions */

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};

struct RenderData {
	static const int							 NUM_FRAMES_IN_FLIGHT = 3;
	FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
	UINT                         g_frameIndex = 0;

	static const int                   NUM_BACK_BUFFERS = 3;
	ID3D12Device* g_pd3dDevice = nullptr;
	ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
	ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
	ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
	ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
	ID3D12Fence* g_fence = nullptr;
	HANDLE                       g_fenceEvent = nullptr;
	UINT64                       g_fenceLastSignaledValue = 0;
	IDXGISwapChain3* g_pSwapChain = nullptr;
	HANDLE                       g_hSwapChainWaitableObject = nullptr;
	ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
	D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};
	WNDCLASSEXW wc;
	HWND hwnd;
};

#define DX12_ENABLE_DEBUG_LAYER

#undef DX12_ENABLE_DEBUG_LAYER // remove if desired

#include "menu/menu.hpp"


namespace App {
	namespace Render {
		bool Start();
		void AppLoop();
		bool Cleanup();

		static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		FrameContext* WaitForNextFrameResources();
		void WaitForLastSubmittedFrame();
		void CleanupRenderTarget();
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		bool CreateDeviceD3D(HWND hWnd);

		inline RenderData* Data = new RenderData();

	}
}