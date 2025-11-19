#include <windows.h>
#include <d3d9.h>
#include <mutex>
#include <toml++/toml.hpp>

#include "nya_dx9_hookbase.h"
#include "nya_commonhooklib.h"
#include "nfsmw.h"

#include "include/chloemenulib.h"

#include "d3dhook.h"
#include "util.h"
#include "chaoseffect.h"

// todo isconditionallyavailable/isavailable check

void ChaosLoop() {
	static CNyaTimer gTimer;
	gTimer.Process();

	if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING || FEManager::mPauseRequest) {
		for (auto& effect : aRunningEffects) {
			if (!effect.pEffect->RunInMenus()) continue;
			effect.OnTick(gTimer.fDeltaTime);
		}
		return;
	}

	// run effects first, then draw the chaos HUD over top
	for (auto& effect : aRunningEffects) {
		effect.OnTick(gTimer.fDeltaTime);
	}

	float y = 0;
	for (auto& effect : aRunningEffects) {
		effect.Draw(y);
		y += 1 - effect.GetOffscreenPercentage();
	}

	while (RunningEffectsCleanup()) {}
}

void ChaosModMenu() {
	ChloeMenuLib::BeginMenu();

	QuickValueEditor("fEffectX", fEffectX);
	QuickValueEditor("fEffectY", fEffectY);
	QuickValueEditor("fEffectSize", fEffectSize);
	QuickValueEditor("fEffectSpacing", fEffectSpacing);
	if (DrawMenuOption("Add Effect")) {
		ChloeMenuLib::BeginMenu();
		for (auto& effect : ChaosEffect::aEffects) {
			if (DrawMenuOption(effect->sName)) {
				AddRunningEffect(effect);
			}
		}
		ChloeMenuLib::EndMenu();
	}
	for (auto& effect : aRunningEffects) {
		DrawMenuOption(std::format("{} - {:.2f} {}", effect.pEffect->sName, effect.fTimer, effect.IsActive()));
	}

	if (auto ply = GetLocalPlayer()) {
		DrawMenuOption(std::format("IPlayer: {:X}", (uintptr_t)ply));
		DrawMenuOption(std::format("ISimable: {:X}", (uintptr_t)ply->GetSimable()));
		DrawMenuOption(std::format("IHud: {:X}", (uintptr_t)ply->GetHud()));
		DrawMenuOption(std::format("IVehicle: {:X}", (uintptr_t)ply->GetSimable()->mCOMObject->Find<IVehicle>()));
		auto pos = ply->GetPosition();
		DrawMenuOption(std::format("Coords: {:.2f} {:.2f} {:.2f}", pos->x, pos->y, pos->z));
		DrawMenuOption(std::format("InGameBreaker: {}", ply->InGameBreaker()));
		DrawMenuOption(std::format("CanRechargeNOS: {}", ply->CanRechargeNOS()));
		DrawMenuOption(std::format("HasNOS: {}", GetLocalPlayerEngine()->HasNOS()));
		DrawMenuOption(std::format("Speed: {:.2f}", GetLocalPlayerVehicle()->GetSpeed()));
		if (DrawMenuOption("Set Standard HUD")) {
			ply->SetHud(PHT_STANDARD);
		}
		if (DrawMenuOption("Set Drag HUD")) {
			ply->SetHud(PHT_DRAG);
		}
	}
	else {
		DrawMenuOption("Local player not found");
	}

	ChloeMenuLib::EndMenu();
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x3C4040) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using v1.3 (.exe size of 6029312 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			auto configName = "NFSMWChaos_gcp.toml";
			if (std::filesystem::exists(configName)) {
				static auto config = toml::parse_file(configName);

			}

			ChloeMenuLib::RegisterMenu("test", &ChaosModMenu);

			NyaHooks::PlaceD3DHooks();
			NyaHooks::aEndSceneFuncs.push_back(D3DHookMain);
			NyaHooks::aD3DResetFuncs.push_back(OnD3DReset);
			NyaHooks::PlaceWndProcHook();
			NyaHooks::aWndProcFuncs.push_back(WndProcHook);
			NyaHooks::PlaceInputBlockerHook();
		} break;
		default:
			break;
	}
	return TRUE;
}