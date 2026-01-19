#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <mutex>
#include <random>
#include <thread>
#include <toml++/toml.hpp>

#include "nya_dx9_hookbase.h"
#include "nya_commonhooklib.h"
#include "nya_commonmath.h"
#include "nfsmw.h"

#include "include/chloemenulib.h"
#include "include/cwoeemodel.h"

std::vector<void(*)()> aMainLoopFunctions;
std::vector<void(*)()> aMainLoopFunctionsOnce;
std::vector<void(*)()> aDrawingLoopFunctions;
std::vector<void(*)()> aDrawingLoopFunctionsOnce;
std::vector<void(*)()> aDrawing3DLoopFunctions;

#include "util.h"
#include "d3dhook.h"
#include "chaosvars.h"
#include "chaospopup.h"
#include "components/achievements.h"
#include "components/customcamera.h"
#include "components/render3d.h"
#include "components/render3d_objects.h"
#include "components/customcar.h"
#include "components/hints.h"
namespace FlatOutHUD {
	#include "components/fo1hud/common.h"
	#include "components/fo1hud/ingame.h"
}
#include "hooks/fixes.h"
#include "hooks/noreset.h"
#include "hooks/gamespeed.h"
#include "hooks/geartype.h"
#include "hooks/vehicleconstruct.h"
#include "hooks/text.h"
#include "hooks/shaders.h"
#include "hooks/carrender.h"
#include "hooks/eventwin.h"
#include "hooks/worldrender.h"
#include "hooks/input.h"
#include "hooks/busted.h"
#include "chaoseffect.h"
#include "chaosmod.h"
#include "chaossave.h"

void OnWinRace() {
	DLLDirSetter _setdir;

	Achievements::AwardAchievement(GetAchievement("WIN_RACE"));
	if (GetEffectRunning(&E_LeakTank)) {
		Achievements::AwardAchievement(GetAchievement("WIN_RACE_LEAKTANK"));
	}
	std::string plyModel = GetLocalPlayerVehicle()->GetVehicleName();
	if (plyModel == "cs_clio_trafpizza" && IsInCareerMode()) {
		Achievements::AwardAchievement(GetAchievement("WIN_RACE_TRAFPIZZA"));
	}
	if (plyModel.starts_with("semi")) {
		Achievements::AwardAchievement(GetAchievement("WIN_RACE_SEMI"));
	}
}

void PlayerInputLoop() {
	ProcessChaosEffects_SetDir<ChaosEffect::HOOK_INPUT>();
}

void MainLoop() {
	DLLDirSetter _setdir;

	for (auto& func : aMainLoopFunctions) {
		func();
	}

	for (auto& func : aMainLoopFunctionsOnce) {
		func();
	}
	aMainLoopFunctionsOnce.clear();
}

void CameraHook(CameraMover* pMover) {
	DLLDirSetter _setdir;

	if (CustomCamera::bRunCustom) {
		static CNyaTimer gTimer;
		gTimer.Process();
		CustomCamera::SetTargetCar(GetLocalPlayerVehicle(), nullptr);
		CustomCamera::ProcessCam(pMover->pCamera, gTimer.fDeltaTime);
	}

	pMoverCamera = pMover->pCamera;
	ProcessChaosEffects<ChaosEffect::HOOK_CAMERA>();
}

void Render3DLoop() {
	DLLDirSetter _setdir;

	static IDirect3DStateBlock9* state = nullptr;
	if (g_pd3dDevice->CreateStateBlock(D3DSBT_ALL, &state) != D3D_OK) {
		return;
	}

	if (state->Capture() < 0) {
		state->Release();
		return;
	}

	for (auto& func : aDrawing3DLoopFunctions) {
		func();
	}

	state->Apply();
	state->Release();

	ProcessChaosEffects<ChaosEffect::HOOK_POST3D>();
}

void ExecuteRenderData_WithHooks() {
	ProcessChaosEffects<ChaosEffect::HOOK_PRE3D>();
	ExecuteRenderData();
	ProcessChaosEffects<ChaosEffect::HOOK_POST3D>();
}

/*void MouseWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static bool bOnce = true;
	if (bOnce) {
		RAWINPUTDEVICE device;
		device.usUsagePage = 1;
		device.usUsage = 2;
		device.dwFlags = 256;
		device.hwndTarget = hWnd;
		RegisterRawInputDevices(&device, 1, sizeof(RAWINPUTDEVICE));
		bOnce = false;
	}

	if (msg == WM_INPUT) {
		RAWINPUT raw;
		UINT size = sizeof(raw);
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));
		if (raw.header.dwType != RIM_TYPEMOUSE) return;
		CustomCamera::fMouse[0] += raw.data.mouse.lLastX;
		CustomCamera::fMouse[1] += raw.data.mouse.lLastY;
	}
	WndProcHook(hWnd, msg, wParam, lParam);
}*/

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x3C4040) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using v1.3 (.exe size of 6029312 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			srand(time(0));

			GetCurrentDirectoryW(MAX_PATH, gDLLDir);

			for (auto& func : ChloeHook::aHooks) {
				func();
			}

			ChloeMenuLib::RegisterMenu("Cwoee Chaos", &ChaosModMenu);
			std::sort(ChaosEffect::aEffects.begin(),ChaosEffect::aEffects.end(),[] (ChaosEffect* a, ChaosEffect* b) { return (std::string)a->GetFriendlyName() < (std::string)b->GetFriendlyName(); });
			aRunningEffects.reserve(64);
			WriteLog(std::format("Initialized {} effects", ChaosEffect::aEffects.size()));

			aMainLoopFunctions.push_back(ProcessChaosEffects<ChaosEffect::HOOK_GAMETICK>);
			aDrawing3DLoopFunctions.push_back(ProcessChaosEffects<ChaosEffect::HOOK_3D>);

			NyaHooks::PlaceD3DHooks(true);
			NyaHooks::D3DEndSceneHook::aFunctions.push_back(D3DHookMain);
			NyaHooks::D3DResetHook::aFunctions.push_back(OnD3DReset);
			NyaHooks::PreHUDDrawHook::aFunctions.push_back(D3DHookPreHUD);
			NyaHooks::WndProcHook::Init();
			//NyaHooks::WndProcHook::aFunctions.push_back(MouseWndProc);
			NyaHooks::WndProcHook::aFunctions.push_back(WndProcHook);
			NyaHooks::WorldServiceHook::Init();
			NyaHooks::WorldServiceHook::aFunctions.push_back(MainLoop);
			NyaHooks::InputBlockerHook::Init();
			NyaHooks::CameraMoverHook::Init();
			NyaHooks::CameraMoverHook::aFunctions.push_back(CameraHook);
			NyaHooks::LateInitHook::Init();
			NyaHooks::RenderWorldHook::Init();
			NyaHooks::RenderWorldHook::aPreFunctions.push_back(ProcessChaosEffects_SetDir<ChaosEffect::HOOK_PRE3D>);
			NyaHooks::RenderWorldHook::aPostFunctions.push_back(Render3DLoop);
			NyaHooks::RenderPropsHook::Init();
			NyaHooks::RenderPropsHook::aPreFunctions.push_back(ProcessChaosEffects_SetDir<ChaosEffect::HOOK_PREPROPS>);
			NyaHooks::RenderPropsHook::aPostFunctions.push_back(ProcessChaosEffects_SetDir<ChaosEffect::HOOK_POSTPROPS>);

			//SkipFE = true;
		} break;
		default:
			break;
	}
	return TRUE;
}