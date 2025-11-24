#include <windows.h>
#include <d3d9.h>
#include <mutex>
#include <toml++/toml.hpp>

#include "nya_dx9_hookbase.h"
#include "nya_commonhooklib.h"
#include "nya_commonmath.h"
#include "nfsmw.h"

#include "include/chloemenulib.h"

std::vector<void(*)()> aMainLoopFunctions;
std::vector<void(*)()> aMainLoopFunctionsOnce;
std::vector<void(*)()> aDrawingLoopFunctions;
std::vector<void(*)()> aDrawingLoopFunctionsOnce;
void MainLoop() {
	for (auto& func : aMainLoopFunctions) {
		func();
	}

	for (auto& func : aMainLoopFunctionsOnce) {
		func();
	}
	aMainLoopFunctionsOnce.clear();
}

bool DisableChaosHUD = false;

#include "d3dhook.h"
#include "util.h"
#include "achievements.h"
#include "hooks/fixes.h"
#include "hooks/noreset.h"
#include "hooks/gamespeed.h"
#include "hooks/geartype.h"
#include "hooks/vehicleconstruct.h"
#include "chaoseffect.h"

void MoneyChecker() {
	static ChaosEffect TempEffect;
	TempEffect.DebugNeverPick = true;
	if (!TempEffect.sName) TempEffect.sName = "(DUMMY) MONEY CHANGE";

	static int cash = 0;
	if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND) {
		cash = FEDatabase->mUserProfile->TheCareerSettings.CurrentCash;
	}
	else if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING) {
		int currentCash = FEDatabase->mUserProfile->TheCareerSettings.CurrentCash;
		if (cash != currentCash) {
			static std::string name;
			name = std::format("Money changed (${} -> ${})", cash, currentCash);
			TempEffect.sName = name.c_str();
			AddRunningEffect(&TempEffect);
			// todo this is an unreliable hack
			if (currentCash > cash && currentCash < cash + 10000) {
				Achievements::AwardAchievement(GetAchievement("WIN_RACE"));
			}
			if (currentCash >= 2000000) {
				Achievements::AwardAchievement(GetAchievement("MILLIONAIRE"));
			}
			cash = currentCash;
		}
	}
}

bool bTimerEnabled = true;
float fEffectCycleTimer = 30;
double fTimeSinceLastEffect = 0;
void ChaosLoop() {
	MoneyChecker();

	for (auto& func : aDrawingLoopFunctions) {
		func();
	}

	for (auto& func : aDrawingLoopFunctionsOnce) {
		func();
	}
	aDrawingLoopFunctionsOnce.clear();

	static CNyaTimer gTimer;
	gTimer.Process();

	for (auto& effect : ChaosEffect::aEffects) {
		effect->fLastTriggerTime += gTimer.fDeltaTime;
	}

	if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING || IsInLoadingScreen() || IsInNIS() || FEManager::mPauseRequest) {
		for (auto& effect : aRunningEffects) {
			if (!effect.pEffect->RunInMenus()) continue;
			effect.OnTick(gTimer.fDeltaTime, true);
		}
		return;
	}

	// run effects first, then draw the chaos HUD over top
	for (auto& effect : aRunningEffects) {
		effect.OnTick(gTimer.fDeltaTime, false);
	}

	float y = 0;
	for (auto& effect : aRunningEffects) {
		if (DisableChaosHUD && !effect.pEffect->IgnoreHUDState()) continue;
		effect.Draw(y);
		y += 1 - effect.GetOffscreenPercentage();
	}
	
	while (RunningEffectsCleanup()) {}
	
	if (bTimerEnabled) {
		fTimeSinceLastEffect += gTimer.fDeltaTime;
		static auto textureL = LoadTexture("CwoeeChaos/data/textures/effectbar.png");
		static auto textureD = LoadTexture("CwoeeChaos/data/textures/effectbar_dark.png");
		auto texture = bDarkMode ? textureD : textureL;
		NyaDrawing::CNyaRGBA32 rgb = bDarkMode ? NyaDrawing::CNyaRGBA32(133,122,168,255) : NyaDrawing::CNyaRGBA32(243,138,175,255);
		if (!DisableChaosHUD) DrawBottomBar(fTimeSinceLastEffect / fEffectCycleTimer, rgb, texture);
		if (fTimeSinceLastEffect >= fEffectCycleTimer) {
			fTimeSinceLastEffect -= fEffectCycleTimer;
			AddRunningEffect(GetRandomEffect());
		}
	}
	else {
		fTimeSinceLastEffect = 0;
	}
}

void ChaosModMenu() {
	ChloeMenuLib::BeginMenu();

	if (DrawMenuOption(std::format("Chaos On - {}", bTimerEnabled))) {
		bTimerEnabled = !bTimerEnabled;
	}
	QuickValueEditor("Cycle Timer", fEffectCycleTimer);

	if (DrawMenuOption("UI")) {
		ChloeMenuLib::BeginMenu();
		if (DrawMenuOption(std::format("Dark Mode - {}", bDarkMode))) {
			bDarkMode = !bDarkMode;
		}
		QuickValueEditor("fEffectX", fEffectX);
		QuickValueEditor("fEffectY", fEffectY);
		QuickValueEditor("fEffectSize", fEffectSize);
		QuickValueEditor("fEffectSpacing", fEffectSpacing);
		QuickValueEditor("fEffectTextureXSpacing", fEffectTextureXSpacing);
		QuickValueEditor("fEffectTextureYSpacing", fEffectTextureYSpacing);
		QuickValueEditor("fEffectArcX", fEffectArcX);
		QuickValueEditor("fEffectArcSize", fEffectArcSize);
		QuickValueEditor("fEffectArcThickness", fEffectArcThickness);
		QuickValueEditor("fEffectArcRotation", fEffectArcRotation);
		ChloeMenuLib::EndMenu();
	}

	if (DrawMenuOption("Debug")) {
		ChloeMenuLib::BeginMenu();
		if (DrawMenuOption("Player Debug")) {
			ChloeMenuLib::BeginMenu();
			if (auto ply = GetLocalPlayer()) {
				DrawMenuOption(std::format("IPlayer: {:X}", (uintptr_t)ply));
				DrawMenuOption(std::format("ISimable: {:X}", (uintptr_t)ply->GetSimable()));
				DrawMenuOption(std::format("IHud: {:X}", (uintptr_t)ply->GetHud()));
				DrawMenuOption(std::format("IVehicle: {:X}", (uintptr_t)ply->GetSimable()->mCOMObject->Find<IVehicle>()));
				auto pos = ply->GetPosition();
				DrawMenuOption(std::format("Coords: {:.2f} {:.2f} {:.2f}", pos->x, pos->y, pos->z));
				UMath::Vector3 fwd;
				if (auto rb = GetLocalPlayerInterface<IRigidBody>()) {
					rb->GetForwardVector(&fwd);
					DrawMenuOption(std::format("Forward: {:.2f} {:.2f} {:.2f}", fwd.x, fwd.y, fwd.z));
				}
				DrawMenuOption(std::format("InGameBreaker: {}", ply->InGameBreaker()));
				DrawMenuOption(std::format("CanRechargeNOS: {}", ply->CanRechargeNOS()));
				DrawMenuOption(std::format("HasNOS: {}", GetLocalPlayerEngine()->HasNOS()));
				DrawMenuOption(std::format("Speed: {:.2f}", GetLocalPlayerVehicle()->GetSpeed()));
				DrawMenuOption(std::format("911 Time: {:.2f}", GetLocalPlayerInterface<IPerpetrator>()->Get911CallTime()));
				DrawMenuOption(std::format("Player Car: {}", FEDatabase->mUserProfile->TheCareerSettings.CurrentCar));
				DrawMenuOption(std::format("Sim Timestep: {:.2f}", Sim::Internal::mSystem->mTimeStep));
				DrawMenuOption(std::format("Sim Speed: {:.2f}", Sim::Internal::mSystem->mSpeed));
				DrawMenuOption(std::format("Sim Target Speed: {:.2f}", Sim::Internal::mSystem->mTargetSpeed));
				if (auto heat = GetMaxHeat()) {
					DrawMenuOption(std::format("Max Heat: {:.2f}", *heat));
				}
				if (GRaceStatus::fObj && GRaceStatus::fObj->mPlayMode == GRaceStatus::kPlayMode_Racing) {
					DrawMenuOption(std::format("Race Completion: {:.2f}", GRaceStatus::fObj->mRacerInfo[0].mPctRaceComplete));
				}
				//DrawMenuOption(std::format("Race Context: {}", (int)GRaceStatus::fObj->mRaceContext));
			}
			else {
				DrawMenuOption("Local player not found");
			}
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("Effect Debug")) {
			ChloeMenuLib::BeginMenu();
			QuickValueEditor("CarMagnetForce", CarMagnetForce);
			QuickValueEditor("TankDrainRate", Effect_LeakTank::TankDrainRate);
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("Achievement Popup")) {
			Achievements::AwardAchievement(GetAchievement("meow"));
		}

		if (DrawMenuOption("Add Effect")) {
			ChloeMenuLib::BeginMenu();
			for (auto& effect : ChaosEffect::aEffects) {
				if (effect->DebugNeverPick) continue;
				if (DrawMenuOption(effect->GetFriendlyName())) {
					AddRunningEffect(effect);
				}
			}
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("Hideable Effects")) {
			ChloeMenuLib::BeginMenu();
			for (auto& effect : ChaosEffect::aEffects) {
				if (effect->DebugNeverPick) continue;
				if (!effect->IsConditionallyAvailable()) continue;
				if (effect->AbortOnConditionFailed()) continue;
				if (DrawMenuOption(effect->GetFriendlyName())) {
					AddRunningEffect(effect);
				}
			}
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("15 sec Effects")) {
			ChloeMenuLib::BeginMenu();
			for (auto& effect : ChaosEffect::aEffects) {
				if (effect->DebugNeverPick) continue;
				if (effect->fTimerLength != 15) continue;
				if (!effect->HasTimer()) continue;
				if (DrawMenuOption(effect->GetFriendlyName())) {
					AddRunningEffect(effect);
				}
			}
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("30 sec Effects")) {
			ChloeMenuLib::BeginMenu();
			for (auto& effect : ChaosEffect::aEffects) {
				if (effect->DebugNeverPick) continue;
				if (effect->fTimerLength != 30) continue;
				if (!effect->HasTimer()) continue;
				if (DrawMenuOption(effect->GetFriendlyName())) {
					AddRunningEffect(effect);
				}
			}
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("Rehideable Effects")) {
			ChloeMenuLib::BeginMenu();
			for (auto& effect : ChaosEffect::aEffects) {
				if (effect->DebugNeverPick) continue;
				if (!effect->IsRehideable()) continue;
				if (DrawMenuOption(effect->GetFriendlyName())) {
					AddRunningEffect(effect);
				}
			}
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("Unmarked Variable Timer Effects")) {
			ChloeMenuLib::BeginMenu();
			for (auto& effect : ChaosEffect::aEffects) {
				if (effect->DebugNeverPick) continue;
				if (effect->fTimerLength == 15) continue;
				if (effect->HasTimer()) continue;
				if (DrawMenuOption(effect->GetFriendlyName())) {
					AddRunningEffect(effect);
				}
			}
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("Running Effects")) {
			ChloeMenuLib::BeginMenu();
			for (auto &effect: aRunningEffects) {
				DrawMenuOption(std::format("{} - {:.2f} {}", effect.GetName(), effect.fTimer, effect.IsActive()));
			}
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("Dump Effect List")) {
			std::ofstream fout("cwoee_effects.txt", std::ios::out);
			if (fout.is_open()) {
				for (auto& effect : ChaosEffect::aEffects) {
					if (effect->DebugNeverPick) continue;
					fout << effect->GetFriendlyName();
					if (effect->sFriendlyName) fout << std::format(" ({})", effect->sName);
					fout << "\n";
				}
			}
		}

		ChloeMenuLib::EndMenu();
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

			srand(time(0));

			//auto configName = "NFSMWChaos_gcp.toml";
			//if (std::filesystem::exists(configName)) {
			//	static auto config = toml::parse_file(configName);
			//}

			for (auto& func : ChloeHook::aHooks) {
				func();
			}

			ChloeMenuLib::RegisterMenu("Chaos Test Menu", &ChaosModMenu);
			std::sort(ChaosEffect::aEffects.begin(),ChaosEffect::aEffects.end(),[] (ChaosEffect* a, ChaosEffect* b) { return (std::string)a->GetFriendlyName() < (std::string)b->GetFriendlyName(); });

			NyaHooks::PlaceD3DHooks();
			NyaHooks::aEndSceneFuncs.push_back(D3DHookMain);
			NyaHooks::aD3DResetFuncs.push_back(OnD3DReset);
			NyaHooks::PlaceWndProcHook();
			NyaHooks::aWndProcFuncs.push_back(WndProcHook);
			NyaHooks::PlaceWorldServiceHook();
			NyaHooks::aWorldServiceFuncs.push_back(MainLoop);
			NyaHooks::PlaceInputBlockerHook();
		} break;
		default:
			break;
	}
	return TRUE;
}