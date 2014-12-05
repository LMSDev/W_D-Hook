#include "stdafx.h"
#include "D3D11Hook.h"
#include "Logger.h"
#include <Windows.h> 
#include "../Depends/FW1FontWrapper_1_1/FW1FontWrapper_1_1/FW1FontWrapper.h"
#include "WDOffsets.h"
#include "LuaFunctions.h"
#include "LuaEngine.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "../Depends/FW1FontWrapper_1_1/FW1FontWrapper_1_1/x64/FW1FontWrapper.lib")

typedef HRESULT (__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void (__stdcall *D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void (__stdcall *D3D11ClearRenderTargetViewHook) (ID3D11DeviceContext* pContext, ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[4]);
typedef void (__stdcall *D3D11CreateDeviceAndSwapChain_t) (IDXGIAdapter *pAdapter, 
														 D3D_DRIVER_TYPE DriverType, 
														 HMODULE Software, 
														 UINT Flags, 
														 const D3D_FEATURE_LEVEL *pFeatureLevels, 
														 UINT FeatureLevels,
														 UINT SDKVersion,
														 const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
														 IDXGISwapChain **ppSwapChain,
														 ID3D11Device **ppDevice,
														 D3D_FEATURE_LEVEL *pFeatureLevel, 
														 ID3D11DeviceContext **ppImmediateContext);

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;

DWORD64* pSwapChainVtable = NULL;
DWORD64* pDeviceContextVTable = NULL;

D3D11PresentHook phookD3D11Present = NULL;
D3D11DrawIndexedHook phookD3D11DrawIndexed = NULL;
D3D11ClearRenderTargetViewHook phookD3D11ClearRenderTargetView = NULL;
D3D11CreateDeviceAndSwapChain_t phookD3D11CreateDeviceAndSwapChain  = NULL;

IFW1Factory *pFW1Factory = NULL;
IFW1FontWrapper *pFontWrapper = NULL;

bool firstTime = true;
bool firstTime2 = true;
void* detourBuffer = NULL;
DWORD64 tickStart = 0;

HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (firstTime)
    {
        pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
        pDevice->GetImmediateContext(&pContext);

        FW1CreateFactory(FW1_VERSION, &pFW1Factory);
        pFW1Factory->CreateFontWrapper(pDevice, L"Calibri", &pFontWrapper);

        pFW1Factory->Release();

        firstTime = false;
    }

	DWORD64 diff = GetTickCount64() - tickStart;
	if (diff < 25000)
	{
		pFontWrapper->DrawString(pContext, L"W_D ScriptHook 0.14 by LMS", 16.0f, 16.0f, 16.0f, 0xffffffff, FW1_RESTORESTATE);
	}
	else if (diff > 55000 && diff < 70000)
	{
		pFontWrapper->DrawString(pContext, L"LukeD is the master of injuring himself with minimal effort. What a moron!", 46.0f, 150.0f, 250.0f, 0xffffffff, FW1_RESTORESTATE);
	}

	if (!LuaEngine::IsReloading())
	{
		for (int i = 0; i < LuaEngine::d3d11PresentCallbacks.size(); i++)
		{	
			// Lua callback
			void* l_state = LuaEngine::GetLuaState();
			LuaFunctions::lua_rawgeti(l_state, LUA_REGISTRYINDEX, LuaEngine::d3d11PresentCallbacks[i]);
			int result = LuaFunctions::plua_pcall(l_state, 0, 0, 0);

			if (result != LUA_OK)
			{
				const char* err = LuaFunctions::lua_tostring(l_state, -1);
				Logger::LogMessage("D3D11Present: Execution error: %s\n", err);
			}
		}
	}

    return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}

void __stdcall hookD3D11DrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	// Do original call
	DWORD64 dwContext = (DWORD64)pContext + 0x0009C08;
	D3D11DrawIndexedHook dwCallTarget = (D3D11DrawIndexedHook)*(DWORD64*)(pContext + 0x8D);
	return dwCallTarget((ID3D11DeviceContext*)dwContext, IndexCount, StartIndexLocation, BaseVertexLocation);

	/*ID3D11DepthStencilState *one;
    D3D11_DEPTH_STENCIL_DESC oneDesc;
    UINT ref;

	if (IndexCount > indexCount)
	{
		pContext->OMGetDepthStencilState(&one,&ref);
		one->GetDesc(&oneDesc);
		oneDesc.DepthEnable=false;
		pDevice->CreateDepthStencilState( &oneDesc, &one);
		pContext->OMSetDepthStencilState(one,ref);

		// Do original call
		DWORD64 dwContext = (DWORD64)pContext + 0x0009C08;
		D3D11DrawIndexedHook dwCallTarget = (D3D11DrawIndexedHook)*(DWORD64*)(pContext + 0x8D);
		dwCallTarget((ID3D11DeviceContext*)dwContext, IndexCount, StartIndexLocation, BaseVertexLocation);

		oneDesc.DepthEnable=true;
		pDevice->CreateDepthStencilState( &oneDesc, &one);
		pContext->OMSetDepthStencilState(one,ref);
	}
	else
	{
		// Do original call
		DWORD64 dwContext = (DWORD64)pContext + 0x0009C08;
		D3D11DrawIndexedHook dwCallTarget = (D3D11DrawIndexedHook)*(DWORD64*)(pContext + 0x8D);
		dwCallTarget((ID3D11DeviceContext*)dwContext, IndexCount, StartIndexLocation, BaseVertexLocation);
	}*/

	//DWORD64 dwContext = (DWORD64)pContext + 0x0009C08;
	//D3D11DrawIndexedHook dwCallTarget = (D3D11DrawIndexedHook)*(DWORD64*)(pContext + 0x8D);
	//return dwCallTarget((ID3D11DeviceContext*)dwContext, IndexCount, StartIndexLocation, BaseVertexLocation);

    //return phookD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void __stdcall hookD3D11ClearRenderTargetView(ID3D11DeviceContext* pContext, ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[4])
{
    return phookD3D11ClearRenderTargetView(pContext, pRenderTargetView, ColorRGBA);
}

const void* __cdecl DetourFunc(BYTE* src, const BYTE* dest, const DWORD length)
{
    BYTE* jump = new BYTE[length + 12];
    detourBuffer = jump;

    DWORD dwVirtualProtectBackup;
    VirtualProtect(src, length, PAGE_READWRITE, &dwVirtualProtectBackup);

	// Normal (positive call)
	// 7FB1F89ED00 - E9 2B16D202           - jmp 7FB225C0330		target has: 2D2162B

    memcpy(jump, src, length);

	// Are we trying to hook a hooked function (e.g. by steam's overlay64.dll)?
	if (jump[0] == 0xE9)
	{
		// Get jump target
		DWORD64 target = 0;
		memcpy(&target, jump+1, 4);

		// 64bit align
		//target += 0xFFFFFFFF00000000;
		Logger::LogMessage("Target: 0x%I64X\n", target);

		// Restore full target address
		DWORD64 tempTarget = target + (DWORD64)src + 5;

		// Hack: To see if target is positive or negative, check if jump target holds valid code
		if (IsBadReadPtr((void*)tempTarget, 0x4))
		{
			Logger::LogMessage("Target appears to be negative\n", target);
			DWORD64 tempTarget = target + (DWORD64)src + 5 + 0xFFFFFFFF00000000;
			target = tempTarget;
		}
		else
		{
			target = tempTarget;
		}

		// mov rax, addr
		jump[0] = 0x48;
		jump[1] = 0xB8;
		*(DWORD64*)(jump + 2) = target;

		// jmp rax
		jump[10] = 0xff;
		jump[11] = 0xe0;

		// Just so return statement won't fuck up
		jump += length;
	}
	else
	{
		jump += length;
		jump[0] = 0xE9;
		*(DWORD*)(jump + 1) = (DWORD)(src + length - jump) - 5;
	}

    src[0] = 0xE9;
    *(DWORD*)(src + 1) = (DWORD)(dest - src) - 5;

    VirtualProtect(src, length, dwVirtualProtectBackup, &dwVirtualProtectBackup);

    return jump - length;
}

void D3D11Hook::Initialize()
{
	// Get fullscreen window
	HWND hWnd = FindWindowA(NULL, "DXGIWatchdogThreadWindow");
    IDXGISwapChain* pSwapChain;
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP != 0 ? FALSE : TRUE;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext);
    if (FAILED(result))
    {
		Logger::LogMessage("D3D11Hook::Initialize(): Attempting again\n");

		// Failed, look for windowed mode
		hWnd = FindWindowA(NULL, "Watch_Dogs");
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP != 0 ? FALSE : TRUE;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext);
		if (FAILED(result))
		{
			Logger::LogMessage("D3D11Hook::Initialize(): Failed to create device!\n");
			return;
		}
    }

    pSwapChainVtable = (DWORD64*)pSwapChain;
    pSwapChainVtable = (DWORD64*)pSwapChainVtable[0];

    pDeviceContextVTable = (DWORD64*)pContext;
    pDeviceContextVTable = (DWORD64*)pDeviceContextVTable[0];

	tickStart = GetTickCount64();

	//Logger::LogMessage("D3D11Present: 0x%I64X\n", (BYTE*)pSwapChainVtable[8]);
	//MessageBoxA(0, "", "", 0);
    phookD3D11Present = (D3D11PresentHook)DetourFunc((BYTE*)pSwapChainVtable[8], (BYTE*)hookD3D11Present, 5);
	//Logger::LogMessage("D3D11DrawIndexedHook: 0x%I64X\n", (BYTE*)pDeviceContextVTable[12]);
    phookD3D11DrawIndexed = (D3D11DrawIndexedHook)DetourFunc((BYTE*)pDeviceContextVTable[12], (BYTE*)hookD3D11DrawIndexed, 5);
    //phookD3D11ClearRenderTargetView = (D3D11ClearRenderTargetViewHook)DetourFunc((BYTE*)pDeviceContextVTable[50], (BYTE*)hookD3D11ClearRenderTargetView, 5);

    DWORD dwOld;
    VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

    pDevice->Release();
    pContext->Release();
    pSwapChain->Release();

    return;
}

void D3D11Hook::DrawString(const WCHAR* pszString, float fontSize, float x, float y, UINT32 color)
{
	pFontWrapper->DrawString(pContext, pszString, fontSize, x, y, color, FW1_RESTORESTATE);
}