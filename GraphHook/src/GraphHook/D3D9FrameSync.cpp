#include "GraphHook/Common.h"
#include "GraphHook/D3D9FrameSync.h"

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <Base/ScopeGuard.h>

namespace gh
{
	D3D9FrameSync::D3D9FrameSync() :
		Singleton(this)
	{
		m_memory = bip::managed_shared_memory(bip::create_only, "D3D9FSSharedMemory", 65536);
		m_data = m_memory.construct<D3D9FSSharedData>("D3D9FSSharedData")();
		m_data->presentBeginReady = false;
		m_data->presentEndReady = false;

		WNDCLASSEX wcex = {};
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = &DefWindowProc;
		wcex.hInstance = GetModuleHandle(nullptr);
		wcex.lpszClassName = _T("D3D9FSWndClass");
		if (RegisterClassEx(&wcex) == 0)
			THROW_WINDOWS_ERROR(GetLastError());
		ON_SCOPE_EXIT([&]()
		{
			UnregisterClass(wcex.lpszClassName, wcex.hInstance);
		});

		HWND window = CreateWindowEx(0, wcex.lpszClassName, _T("D3D9FSWnd"), WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, wcex.hInstance, nullptr);
		if (window == nullptr)
			THROW_WINDOWS_ERROR(GetLastError());
		ON_SCOPE_EXIT([&]()
		{
			DestroyWindow(window);
		});

		HMODULE d3d9Dll = GetModuleHandle(_T("d3d9.dll"));
		if (d3d9Dll == nullptr)
			THROW_WINDOWS_ERROR(GetLastError());
		Direct3DCreate9_t direct3DCreate9 = reinterpret_cast<Direct3DCreate9_t>(
			GetProcAddress(d3d9Dll, "Direct3DCreate9"));
		if (direct3DCreate9 == nullptr)
			THROW_WINDOWS_ERROR(GetLastError());

		CComPtr<IDirect3D9> d3d9;
		d3d9.p = direct3DCreate9(D3D_SDK_VERSION);
		if (d3d9 == nullptr)
			THROW_BASE_EXCEPTION(Exception() << err_str("Direct3DCreate9() failed."));

		D3DDISPLAYMODE mode = {};
		HRESULT hr = d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
		if (FAILED(hr))
			THROW_DIRECTX_HRESULT(hr);

		D3DPRESENT_PARAMETERS pp = {};
		pp.Windowed = TRUE;
		pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		pp.BackBufferFormat = mode.Format;

		CComPtr<IDirect3DDevice9> device;
		hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
			&pp, &device);
		if (FAILED(hr))
			THROW_DIRECTX_HRESULT(hr);

		uint_t* vTable = (uint_t*)(*((uint_t*)device.p));
		m_clearFunc = HookFunc<Clear_t>(reinterpret_cast<LPVOID>(vTable[43]), &D3D9FrameSync::ClearHook);
		//m_beginSceneFunc = HookFunc<BeginScene_t>(reinterpret_cast<LPVOID>(vTable[41]), &D3D9FrameSync::BeginSceneHook);
		//m_endSceneFunc = HookFunc<EndScene_t>(reinterpret_cast<LPVOID>(vTable[42]), &D3D9FrameSync::EndSceneHook);
		m_presentFunc = HookFunc<Present_t>(reinterpret_cast<LPVOID>(vTable[17]), &D3D9FrameSync::PresentHook);
	}

	D3D9FrameSync::~D3D9FrameSync()
	{
		m_memory.destroy<D3D9FSSharedData>("D3D9FSSharedData");
	}

	HRESULT D3D9FrameSync::ClearHook(IDirect3DDevice9* device, DWORD count, CONST D3DRECT* rects, DWORD flags,
		D3DCOLOR color, float z, DWORD stencil)
	{
		D3D9FrameSync& d3d9FrameSync = D3D9FrameSync::GetInstance();
		return d3d9FrameSync.clearHook(device, count, rects, flags, color, z, stencil);
	}

	HRESULT D3D9FrameSync::BeginSceneHook(IDirect3DDevice9* device)
	{
		D3D9FrameSync& d3d9FrameSync = D3D9FrameSync::GetInstance();
		return d3d9FrameSync.beginSceneHook(device);
	}

	HRESULT D3D9FrameSync::EndSceneHook(IDirect3DDevice9* device)
	{
		D3D9FrameSync& d3d9FrameSync = D3D9FrameSync::GetInstance();
		return d3d9FrameSync.endSceneHook(device);
	}

	HRESULT D3D9FrameSync::PresentHook(IDirect3DDevice9* device, CONST RECT* sourceRect, CONST RECT* destRect,
		HWND destWindowOverride, CONST RGNDATA* dirtyRegion)
	{
		D3D9FrameSync& d3d9FrameSync = D3D9FrameSync::GetInstance();
		return d3d9FrameSync.presentHook(device, sourceRect, destRect, destWindowOverride, dirtyRegion);
	}

	HRESULT D3D9FrameSync::clearHook(IDirect3DDevice9* device, DWORD count, CONST D3DRECT* rects, DWORD flags,
		D3DCOLOR color, float z, DWORD stencil)
	{
		m_t0 = std::chrono::steady_clock::now();
		//std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		//time_t e1 = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - m_t0).count();
		//BOOST_LOG_TRIVIAL(error) << e1;
		return m_clearFunc(device, count, rects, flags, color, z, stencil);
	}

	HRESULT D3D9FrameSync::beginSceneHook(IDirect3DDevice9* device)
	{
		return m_beginSceneFunc(device);
	}

	HRESULT D3D9FrameSync::endSceneHook(IDirect3DDevice9* device)
	{
		return m_endSceneFunc(device);
	}

	HRESULT D3D9FrameSync::presentHook(IDirect3DDevice9* device, CONST RECT* sourceRect, CONST RECT* destRect,
		HWND destWindowOverride, CONST RGNDATA* dirtyRegion)
	{
		{
			bip::scoped_lock<bip::interprocess_mutex> lock(m_data->presentBeginMutex);
			m_data->presentBeginReady = true;
			m_data->presentBeginCond.notify_one();
		}
		//m_data->m_t0 = std::chrono::steady_clock::now();
		//std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
		HRESULT hr = m_presentFunc(device, sourceRect, destRect, destWindowOverride, dirtyRegion);
		//m_t0 = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		time_t e1 = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - m_t0).count();
		BOOST_LOG_TRIVIAL(error) << e1;
		{
			bip::scoped_lock<bip::interprocess_mutex> lock(m_data->presentEndMutex);
			m_data->presentEndReady = true;
			m_data->presentEndCond.notify_one();
		}
		return hr;
	}
}