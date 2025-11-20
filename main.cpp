#include <windows.h>
#include <d3d9.h>
#include <mutex>
#include <toml++/toml.hpp>

#include "nya_dx9_hookbase.h"
#include "nya_commonhooklib.h"
#include "nya_commonmath.h"
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

uint32_t ForcedPlayerVehicle = 0;
uint32_t ForcedOpponentVehicle = 0;
uint32_t ForcedTrafficVehicle = 0;
bool PlayerFullyTuned = false;
bool OpponentsFullyTuned = false;
bool RandomizePlayerTuning = false;
bool RandomizeOpponentTuning = false;
bool OpponentPlayerCar = false;
bool OpponentPlayerCarRandom = false;
uint32_t PlayerCarModel;
FECustomizationRecord PlayerCarCustomizations;

#include "d3dhook.h"
#include "util.h"
#include "hooks/fixes.h"
#include "chaoseffect.h"

ISimable* VehicleConstructHooked(Sim::Param params) {
	auto vehicle = (VehicleParams*)params.mData;
	if (vehicle->carClass == DRIVER_HUMAN) {
		if (ForcedPlayerVehicle) {
			vehicle->carType = ForcedPlayerVehicle;
			vehicle->customization = nullptr;
		}
		if (PlayerFullyTuned) {
			if (!vehicle->matched) vehicle->matched = new Physics::Info::Performance;
			vehicle->matched->Acceleration = 100;
			vehicle->matched->Handling = 100;
			vehicle->matched->TopSpeed = 100;
		}
		if (RandomizePlayerTuning) {
			vehicle->customization = new FECustomizationRecord;
			*vehicle->customization = CreateRandomCustomizations(vehicle->carType);
		}
		PlayerCarModel = vehicle->carType;
		if (vehicle->customization) {
			PlayerCarCustomizations = *vehicle->customization;
		}
		else {
			PlayerCarCustomizations = CreateStockCustomizations(vehicle->carType);
		}
	}
	if (vehicle->carClass == DRIVER_RACER) {
		if (ForcedOpponentVehicle) {
			vehicle->carType = ForcedOpponentVehicle;
			vehicle->customization = nullptr;
		}
		if (OpponentPlayerCar) {
			vehicle->carType = PlayerCarModel;
			vehicle->customization = new FECustomizationRecord;
			*vehicle->customization = PlayerCarCustomizations;
			vehicle->matched = nullptr;
		}
		if (OpponentPlayerCarRandom) {
			if (auto car = GetRandomCareerCar()) {
				vehicle->carType = car->VehicleKey;
				vehicle->customization = new FECustomizationRecord;
				*vehicle->customization = *FEPlayerCarDB::GetCustomizationRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->Customization);
				vehicle->matched = nullptr;
			}
			else {
				vehicle->carType = PlayerCarModel;
				vehicle->customization = new FECustomizationRecord;
				*vehicle->customization = PlayerCarCustomizations;
				vehicle->matched = nullptr;
			}
		}
		if (OpponentsFullyTuned) {
			if (!vehicle->matched) vehicle->matched = new Physics::Info::Performance;
			vehicle->matched->Acceleration = 100;
			vehicle->matched->Handling = 100;
			vehicle->matched->TopSpeed = 100;
		}
		if (RandomizeOpponentTuning) {
			vehicle->customization = new FECustomizationRecord;
			*vehicle->customization = CreateRandomCustomizations(vehicle->carType);
		}
	}
	if (vehicle->carClass == DRIVER_TRAFFIC && ForcedTrafficVehicle) {
		vehicle->carType = ForcedTrafficVehicle;
		vehicle->customization = nullptr;
	}
	return PVehicle::Construct(params);
}

bool bTimerEnabled = true;
float fEffectCycleTimer = 30;
double fTimeSinceLastEffect = 0;
void ChaosLoop() {
	*(void**)0x92C534 = (void*)&VehicleConstructHooked;

	static CNyaTimer gTimer;
	gTimer.Process();

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
	
	QuickValueEditor("CarMagnetForce", CarMagnetForce);

	if (DrawMenuOption("Dump Effect List")) {
		std::ofstream fout("cwoee_effects.txt", std::ios::out);
		if (fout.is_open()) {
			for (auto& effect : ChaosEffect::aEffects) {
				fout << effect->sName;
				fout << "\n";
			}
		}
	}

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
		DrawMenuOption(std::format("{} - {:.2f} {}", effect.GetName(), effect.fTimer, effect.IsActive()));
	}

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

			srand(time(0));

			auto configName = "NFSMWChaos_gcp.toml";
			if (std::filesystem::exists(configName)) {
				static auto config = toml::parse_file(configName);

			}

			for (auto& func : ChloeHook::aHooks) {
				func();
			}

			ChloeMenuLib::RegisterMenu("Chaos Test Menu", &ChaosModMenu);
			std::sort(ChaosEffect::aEffects.begin(),ChaosEffect::aEffects.end(),[] (ChaosEffect* a, ChaosEffect* b) { return (std::string)a->sName < (std::string)b->sName; });

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