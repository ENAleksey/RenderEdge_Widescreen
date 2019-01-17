#include <windows.h>
#include <cmath> 
#include "War3Versions.h"
#include "detours.h"

#pragma comment( lib, "detours.lib" )
#pragma comment(lib, "Version.lib")

using uint32 = unsigned int;

HWND g_hWnd;
uintptr_t address_GameBase = 0;
uintptr_t address_CreateMatrixPerspectiveFov = 0;
uintptr_t address_BuildHPBars = 0;
float g_fWideScreenMul = 1.0f;


void __fastcall CreateMatrixPerspectiveFov_proxy(uint32 outMatrix, uint32 unused, float fovY, float aspectRatio, float nearZ, float farZ)
{
	RECT r;
	if (GetWindowRect(g_hWnd, &r))
	{
		float width = float(r.right - r.left);
		float rHeight = 1.0f / (r.bottom - r.top);
		g_fWideScreenMul = width * rHeight * 0.75f; // (width / height) / (4.0f / 3.0f)
	}

	float yScale = 1.0f / tan(fovY * 0.5f / sqrt(aspectRatio * aspectRatio + 1.0f));
	float xScale = yScale / (aspectRatio * g_fWideScreenMul);

	*(float*)(outMatrix) = xScale;
	*(float*)(outMatrix + 16) = 0.0f;
	*(float*)(outMatrix + 32) = 0.0f;
	*(float*)(outMatrix + 48) = 0.0f;

	*(float*)(outMatrix + 4) = 0.0f;
	*(float*)(outMatrix + 20) = yScale;
	*(float*)(outMatrix + 36) = 0.0f;
	*(float*)(outMatrix + 52) = 0.0f;

	*(float*)(outMatrix + 8) = 0.0f;
	*(float*)(outMatrix + 24) = 0.0f;
	*(float*)(outMatrix + 40) = (nearZ + farZ) / (farZ - nearZ);
	*(float*)(outMatrix + 56) = (-2.0f * nearZ * farZ) / (farZ - nearZ);

	*(float*)(outMatrix + 12) = 0.0f;
	*(float*)(outMatrix + 28) = 0.0f;
	*(float*)(outMatrix + 44) = 1.0f;
	*(float*)(outMatrix + 60) = 0.0f;
}


void __fastcall BuildHPBars_proxy(uint32 a1, uint32 unused, uint32 a2, uint32 a3)
{
	((void(__fastcall *)(uint32, uint32, uint32, uint32))address_BuildHPBars)(a1, unused, a2, a3);

	uint32 pHPBarFrame = *((uint32*)a1 + 3);
	if (pHPBarFrame)
		*((float*)pHPBarFrame + 22) /= g_fWideScreenMul;
}


bool inline_install(uintptr_t* pointer_ptr, uintptr_t detour)
{
	LONG status;
	if ((status = DetourTransactionBegin()) == NO_ERROR)
	{
		if ((status = DetourUpdateThread(GetCurrentThread())) == NO_ERROR)
		{
			if ((status = DetourAttach((PVOID*)pointer_ptr, (PVOID)detour)) == NO_ERROR)
			{
				if ((status = DetourTransactionCommit()) == NO_ERROR)
				{
					return true;
				}
			}
		}
		DetourTransactionAbort();
	}
	return false;
}

Version GetGameVersion()
{
	DWORD dwHandle;
	DWORD sz = GetFileVersionInfoSizeW(L"Game.dll", &dwHandle);
	if (sz == 0)
	{
		return Version::unknown;
	}

	char* buf = new char[sz];
	if (!GetFileVersionInfoW(L"Game.dll", dwHandle, sz, &buf[0]))
	{
		delete buf;
		return Version::unknown;
	}

	VS_FIXEDFILEINFO* pvi;
	sz = sizeof(VS_FIXEDFILEINFO);
	if (!VerQueryValueW(&buf[0], L"\\", (LPVOID*)&pvi, (uint32*)&sz))
	{
		delete buf;
		return Version::unknown;
	}
	delete buf;

	uint32 buildVersion = pvi->dwFileVersionLS & 0xFFFF;
	return static_cast<Version>(buildVersion);
}


BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID /*pReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);

		address_GameBase = (uintptr_t)GetModuleHandleW(L"Game.dll");
		g_hWnd = GetActiveWindow();

		Version gameVersion = GetGameVersion();
		switch (gameVersion)
		{
		case Version::v128f:
			address_CreateMatrixPerspectiveFov = 0x1554F0;
			address_BuildHPBars = 0x3C6700;
			break;
		case Version::v128e:
			address_CreateMatrixPerspectiveFov = 0x1553B0;
			address_BuildHPBars = 0x3C6560;
			break;
		case Version::v128c:
			address_CreateMatrixPerspectiveFov = 0x12B730;
			address_BuildHPBars = 0x39C670;
			break;
		case Version::v128a:
			address_CreateMatrixPerspectiveFov = 0x129150;
			address_BuildHPBars = 0x399F20;
			break;
		case Version::v127b:
			address_CreateMatrixPerspectiveFov = 0x126E30;
			address_BuildHPBars = 0x3925F0;
			break;
		case Version::v127a:
			address_CreateMatrixPerspectiveFov = 0x0D31D0;
			address_BuildHPBars = 0x374E60;
			break;
		case Version::v126a:
			address_CreateMatrixPerspectiveFov = 0x7B66F0;
			address_BuildHPBars = 0x379A30;
			break;
		case Version::v124e:
			address_CreateMatrixPerspectiveFov = 0x7B6E90;
			address_BuildHPBars = 0x37A570;
			break;
		case Version::v123a:
			address_CreateMatrixPerspectiveFov = 0x7A7BA0;
			address_BuildHPBars = 0x37A3F0;
			break;
		default:
			MessageBoxW(0, L"Error! Your WarCraft version is not supported.", L"RenderEdge_Widescreen.mix", 0);
			break;
		}

		if (address_CreateMatrixPerspectiveFov)
		{
			address_CreateMatrixPerspectiveFov += address_GameBase;
			inline_install(&address_CreateMatrixPerspectiveFov, (uintptr_t)CreateMatrixPerspectiveFov_proxy);
		}

		if (address_BuildHPBars)
		{
			address_BuildHPBars += address_GameBase;
			inline_install(&address_BuildHPBars, (uintptr_t)BuildHPBars_proxy);
		}
	}

	return TRUE;
}