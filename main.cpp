#include <windows.h>
#include <d3d9.h>
#include <mutex>
#include <toml++/toml.hpp>

#include "nya_dx9_hookbase.h"
#include "nya_commonhooklib.h"
#include "nfsmw.h"

#include "include/chloemenulib.h"

// copies the attrib lap count to GRaceIndexData, fixes lap glitch from the in-game menu and makes ingame lap count data persist in general
void FixIndexLapCount() {
	auto race = GRaceStatus::fObj;
	if (!race) return;
	//if (race->mPlayMode == GRaceStatus::kPlayMode_Roaming) return;
	if (!race->mRaceParms) return;
	if (!GRaceParameters::GetIsLoopingRace(race->mRaceParms)) return;
	if (auto index = race->mRaceParms->mIndex) {
		index->mNumLaps = *(uint32_t*)Attrib::Instance::GetAttributePointer(race->mRaceParms->mRaceRecord, Attrib::StringHash32("NumLaps"), 0);
	}
}

std::vector<void(*)()> aMainLoopFunctions;
void MainLoop() {
	for (auto& func : aMainLoopFunctions) {
		func();
	}
	aMainLoopFunctions.clear();

	FixIndexLapCount();
}

#include "d3dhook.h"
#include "util.h"
#include "chaoseffect.h"

// todo isconditionallyavailable/isavailable check
ChaosEffect* GetRandomEffect() {
	std::vector<ChaosEffect*> availableEffects;
	for (auto& effect : ChaosEffect::aEffects) {
		if (effect->bTriggeredThisCycle) continue;
		//if (effect->fLastTriggerTime) // todo
		availableEffects.push_back(effect);
	}
	if (availableEffects.empty()) {
		for (auto& effect : ChaosEffect::aEffects) {
			effect->bTriggeredThisCycle = false;
		}
		return GetRandomEffect();
	}
	return availableEffects[rand()%availableEffects.size()];
}

bool bTimerEnabled = true;
float fEffectCycleTimer = 30;
double fTimeSinceLastEffect = 0;
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
	
	if (bTimerEnabled) {
		fTimeSinceLastEffect += gTimer.fDeltaTime;
		DrawRectangle(0, 1, 0, 0.05, {0, 0, 0, 255});
		DrawRectangle(0, fTimeSinceLastEffect / fEffectCycleTimer, 0, 0.05, {255, 255, 255, 255});
		if (fTimeSinceLastEffect >= fEffectCycleTimer) {
			fTimeSinceLastEffect -= fEffectCycleTimer;
			AddRunningEffect(GetRandomEffect());
		}
	}
}

void ChaosModMenu() {
	ChloeMenuLib::BeginMenu();

	if (DrawMenuOption(std::format("Chaos On - {}", bTimerEnabled))) {
		bTimerEnabled = !bTimerEnabled;
	}
	QuickValueEditor("Cycle Timer", fEffectCycleTimer);
	
	//QuickValueEditor("fEffectX", fEffectX);
	//QuickValueEditor("fEffectY", fEffectY);
	//QuickValueEditor("fEffectSize", fEffectSize);
	//QuickValueEditor("fEffectSpacing", fEffectSpacing);
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
		DrawMenuOption(std::format("911 Time: {:.2f}", GetLocalPlayerInterface<IPerpetrator>()->Get911CallTime()));
		if (GRaceStatus::fObj && GRaceStatus::fObj->mRaceParms) {
			DrawMenuOption(std::format("Race Hash: {:X}", Attrib::Instance::GetCollection(GRaceStatus::fObj->mRaceParms->mRaceRecord)));
			auto numLaps = (uint32_t*)Attrib::Collection::GetData(GRaceStatus::fObj->mRaceParms->mRaceRecord->mCollection, Attrib::StringHash32("NumLaps"), 0);
			DrawMenuOption(std::format("Num Laps: {}", numLaps ? *numLaps : -1));
			if (auto parent = GRaceStatus::fObj->mRaceParms->mRaceRecord->mCollection->mParent) {
				DrawMenuOption(std::format("Parent Hash: {:X}", parent->mKey));
				numLaps = (uint32_t*)Attrib::Collection::GetData(parent, Attrib::StringHash32("NumLaps"), 0);
				DrawMenuOption(std::format("Parent Num Laps: {}", numLaps ? *numLaps : -1));
			}
			if (auto index = GRaceStatus::fObj->mRaceParms->mIndex) {
				DrawMenuOption(std::format("Index Num Laps: {}", index->mNumLaps));
			}
			DrawMenuOption(std::format("Event ID: {}", GRaceParameters::GetEventID(GRaceStatus::fObj->mRaceParms)));
		}
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

// fixes a crash when totaling your car after quitting from a race
// in this case GRacerInfo is valid, mRaceParms is not
void __thiscall TotalVehicleFixed(GRacerInfo* pThis) {
	if (!GRaceStatus::fObj->mRaceParms) return;
	GRacerInfo::TotalVehicle(pThis);
}
void __thiscall BlowEngineFixed(GRacerInfo* pThis) {
	if (!GRaceStatus::fObj->mRaceParms) return;
	GRacerInfo::BlowEngine(pThis);
}

// fixes a crash when hitting objects using a player-driven AI car
// the result of this function is not null checked at 4F0E70
EAX_CarState* GetClosestPlayerCarFixed(bVector3* a1) {
	if (auto car = GetClosestPlayerCar(a1)) return car;
	static EAX_CarState temp;
	memset(&temp,0,sizeof(temp));
	return &temp;
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

			NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x63C093, &TotalVehicleFixed);
			NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x63839A, &BlowEngineFixed);
			NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x4F0E70, &GetClosestPlayerCarFixed);

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