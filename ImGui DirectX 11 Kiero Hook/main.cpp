#include "includes.h"
#include <TlHelp32.h>
#include <iostream>
#include "Utils.h"
#include "globals.h"
#include "Offsets.h"
#include "Memory.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("ImGui Window");
	ImGui::End();

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}


void InitCheat()
{
	const wchar_t* targetProcessName = L"RustClient.exe";
	DWORD processID = 0;
	// Get the process ID
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32W processEntry;
		processEntry.dwSize = sizeof(PROCESSENTRY32W);

		if (Process32FirstW(snapshot, &processEntry)) {
			do {
				if (_wcsicmp(processEntry.szExeFile, targetProcessName) == 0) {
					processID = processEntry.th32ProcessID;
					break;
				}
			} while (Process32NextW(snapshot, &processEntry));
		}
		CloseHandle(snapshot);
	}
	if (processID != 0) {
		Globals::BaseAddr = mem::get_module_base("GameAssembly.dll");

		std::wcout << L"Module Base Address:  0x" << std::hex << Globals::BaseAddr << std::endl;
		std::wcout << L"BaseNetworkableOffset: 0x" << std::hex << Offsets::BaseNetworkableOffset << std::endl;
		std::wcout << L"Target Address for BaseNetworkable: 0x" << std::hex << (Globals::BaseAddr + Offsets::BaseNetworkableOffset) << std::endl;

		
		uint64_t* BaseNetworkable_o = (uint64_t*)(Globals::BaseAddr + Offsets::BaseNetworkableOffset);
		std::wcout << L"BaseNetworkable_o Address: 0x" << std::hex << (uintptr_t)BaseNetworkable_o << std::endl;
		uint64_t baseNetworkable = *BaseNetworkable_o;
		std::wcout << L"BaseNetworkable Value: 0x" << std::hex << baseNetworkable << std::endl;
		uint64_t* BaseNetworkableStaticFields_o = (uint64_t*)(BaseNetworkable_o + Offsets::staticBaseNetOffset);
		std::wcout << L"BaseNetworkableStaticFields_o Address: 0x" << std::hex << (uintptr_t)BaseNetworkableStaticFields_o << std::endl;
		uint64_t BaseNetworkableStaticFields = *BaseNetworkableStaticFields_o;
		std::wcout << L"BaseNetworkableStaticFields Value: 0x" << std::hex << BaseNetworkableStaticFields << std::endl;
		uint64_t* WrapperClassPtr = (uint64_t*)(BaseNetworkableStaticFields_o + Offsets::wrapperPtrOffset);
		std::wcout << L"WrapperClassPtr Address: 0x" << std::hex << (uintptr_t)WrapperClassPtr << std::endl;
		uint64_t wrapperPtr = *WrapperClassPtr;
		std::wcout << L"WrapperClassPtr Value: 0x" << std::hex << wrapperPtr << std::endl;
		uint64_t WrapperClass = BaseNetworkable(wrapperPtr);
		std::wcout << L"WrapperClass Value: 0x" << std::hex << WrapperClass << std::endl;
	}
	else {
		std::wcerr << L"Target process not found." << std::endl;
	}
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			InitCheat();
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		AllocConsole();
		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}
