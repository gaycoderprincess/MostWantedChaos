#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <mutex>
#include <random>
#include <numbers>
#include <toml++/toml.hpp>
#include "assimp/Importer.hpp"
#include "assimp/Exporter.hpp"
#include "assimp/Logger.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "nya_dx9_hookbase.h"
#include "nya_commonhooklib.h"
#include "nya_commonmath.h"
#include "nfsmw.h"

#include "include/chloemenulib.h"

std::vector<void(*)()> aMainLoopFunctions;
std::vector<void(*)()> aMainLoopFunctionsOnce;
std::vector<void(*)()> aDrawingLoopFunctions;
std::vector<void(*)()> aDrawingLoopFunctionsOnce;
std::vector<void(*)()> aDrawingGameUILoopFunctions;
std::vector<void(*)()> aDrawingGameUILoopFunctionsOnce;
std::vector<void(*)()> aDrawing3DLoopFunctions;
std::vector<void(*)()> aDrawing3DLoopFunctionsOnce;
void MainLoop() {
	for (auto& func : aMainLoopFunctions) {
		func();
	}

	for (auto& func : aMainLoopFunctionsOnce) {
		func();
	}
	aMainLoopFunctionsOnce.clear();
}

bool bTimerEnabled = true;
float fEffectCycleTimer = 30;
float fEffectCycleTimerSpeedMult = 1;
double fTimeSinceLastEffect = 0;
bool DisableChaosHUD = false;

#include "d3dhook.h"
#include "util.h"
#include "components/achievements.h"
#include "components/customcamera.h"
#include "components/render3d.h"
#include "components/render3d_objects.h"
#include "components/customcar.h"
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
#include "chaoseffect.h"

void OnWinRace() {
	DLLDirSetter _setdir;

	Achievements::AwardAchievement(GetAchievement("WIN_RACE"));
	if (IsEffectRunning(&E_LeakTank)) {
		Achievements::AwardAchievement(GetAchievement("WIN_RACE_LEAKTANK"));
	}
	auto plyModel = GetLocalPlayerVehicle()->GetVehicleName();
	if (!strcmp(plyModel, "cs_clio_trafpizza")) {
		Achievements::AwardAchievement(GetAchievement("WIN_RACE_TRAFPIZZA"));
	}
}

void MoneyChecker() {
	static ChaosEffect TempEffect("DUMMY", true);
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
			if (currentCash >= 2000000) {
				Achievements::AwardAchievement(GetAchievement("MILLIONAIRE"));
			}
			if (currentCash <= -2000000) {
				Achievements::AwardAchievement(GetAchievement("ANTI_MILLIONAIRE"));
			}
			cash = currentCash;
		}
	}
}

void CameraHook(CameraMover* pMover) {
	DLLDirSetter _setdir;

	static CNyaTimer gTimer;
	gTimer.Process();

	if (IsChaosBlocked() && TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_IN_FRONTEND) return;

	auto inMenu = TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND;
	if (!inMenu && CustomCamera::bRunCustom) {
		CustomCamera::SetTargetCar(GetLocalPlayerVehicle(), nullptr);
		CustomCamera::ProcessCam(pMover->pCamera, gTimer.fDeltaTime);
	}

	for (auto& effect : aRunningEffects) {
		if (inMenu && !effect.pEffect->RunInMenus()) continue;
		effect.OnTickCamera(pMover->pCamera, gTimer.fDeltaTime);
	}
}

void ProcessChaosEffects(double fDeltaTime, bool inMenu, bool blockedByOtherMeans) {
	// run effects first, then draw the chaos HUD over top
	for (auto& effect : aRunningEffects) {
		if (inMenu && !effect.pEffect->RunInMenus()) continue;
		if (blockedByOtherMeans && !effect.pEffect->RunWhenBlocked()) continue;
		effect.OnTick(fDeltaTime, inMenu || blockedByOtherMeans);
	}

	float y = 0;
	for (auto& effect : aRunningEffects) {
		if (inMenu && !effect.pEffect->RunInMenus()) continue;
		if (blockedByOtherMeans && !effect.pEffect->RunWhenBlocked()) continue;
		if (DisableChaosHUD && !effect.pEffect->IgnoreHUDState()) continue;
		effect.Draw(y, inMenu || blockedByOtherMeans);
		y += 1 - (inMenu ? 0 : effect.GetOffscreenPercentage());
	}

	while (RunningEffectsCleanup()) {}
}

void ChaosLoop() {
	DLLDirSetter _setdir;

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

	if (IsChaosBlocked()) {
		bool inMenu = TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND;
		ProcessChaosEffects(gTimer.fDeltaTime, inMenu, !inMenu);
		return;
	}
	ProcessChaosEffects(gTimer.fDeltaTime, false, false);

	static ChaosEffect TempEffect("DUMMY", true);
	if (!TempEffect.sName) {
		static char tmp[256];
		strcpy_s(tmp, 256, std::format("Cwoee Chaos v{} by gaycoderprincess", CWOEECHAOS_VERSION).c_str());
		TempEffect.sName = tmp;
		AddRunningEffect(&TempEffect);
	}

	if (bTimerEnabled) {
		static ChaosEffect TempEffect("DUMMY", true);
		if (!TempEffect.sName) {
			TempEffect.sName = "mod active! awruff :3";
			AddRunningEffect(&TempEffect);
		}

		fTimeSinceLastEffect += gTimer.fDeltaTime * fEffectCycleTimerSpeedMult;
		static auto textureL = LoadTexture("CwoeeChaos/data/textures/effectbar.png");
		static auto textureD = LoadTexture("CwoeeChaos/data/textures/effectbar_dark.png");
		auto texture = bDarkMode ? textureD : textureL;
		NyaDrawing::CNyaRGBA32 rgb = bDarkMode ? NyaDrawing::CNyaRGBA32(133,122,168,255) : NyaDrawing::CNyaRGBA32(243,138,175,255);
		if (!DisableChaosHUD) DrawBottomBar(fTimeSinceLastEffect / fEffectCycleTimer, rgb, texture);

		// add a lil bit of extra time so effects don't overlap each other by a frame or two
		float cycleTimer = fEffectCycleTimer + 0.1;
		if (fTimeSinceLastEffect >= cycleTimer) {
			fTimeSinceLastEffect -= cycleTimer;
			AddRunningEffect(GetRandomEffect());
		}
	}
	else {
		fTimeSinceLastEffect = 0;
	}
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

	static CNyaTimer gTimer;
	gTimer.Process();

	/*static auto models = Render3D::CreateModels("shork.fbx");
	for (auto& model : models) {
		auto mat = UMath::Matrix4::kIdentity;
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			ply->GetMatrix4(&mat);
			UMath::Vector3 dim;
			ply->GetDimension(&dim);
			mat.p = *ply->GetPosition();
		}
		mat.x *= 1;
		mat.y *= 1;
		mat.z *= 1;

		UMath::Matrix4 rotation;
		//rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
		mat = (UMath::Matrix4)(mat * rotation);
		model->RenderAt(WorldToRenderMatrix(mat));
	}*/

	for (auto& func : aDrawing3DLoopFunctions) {
		func();
	}

	for (auto& func : aDrawing3DLoopFunctionsOnce) {
		func();
	}
	aDrawing3DLoopFunctionsOnce.clear();

	auto inMenu = TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND;
	auto isBlocked = IsChaosBlocked();
	for (auto& effect : aRunningEffects) {
		if (inMenu && !effect.pEffect->RunInMenus()) continue;
		if (isBlocked && !effect.pEffect->RunWhenBlocked()) continue;
		effect.OnTick3D(gTimer.fDeltaTime);
	}

	state->Apply();
	state->Release();
}

void ChaosModMenu() {
	DLLDirSetter _setdir;

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
		QuickValueEditor("fEffectTimerTextSize", fEffectTimerTextSize);
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
					rb->GetRightVector(&fwd);
					DrawMenuOption(std::format("Right: {:.2f} {:.2f} {:.2f}", fwd.x, fwd.y, fwd.z));
					rb->GetForwardVector(&fwd);
					DrawMenuOption(std::format("Forward: {:.2f} {:.2f} {:.2f}", fwd.x, fwd.y, fwd.z));
					rb->GetUpVector(&fwd);
					DrawMenuOption(std::format("Up: {:.2f} {:.2f} {:.2f}", fwd.x, fwd.y, fwd.z));
					rb->GetDimension(&fwd);
					DrawMenuOption(std::format("Dimension: {:.2f} {:.2f} {:.2f}", fwd.x, fwd.y, fwd.z));
				}
				auto cam = GetLocalPlayerCamera();
				auto camMatrix = *(NyaMat4x4*)&cam->CurrentKey.Matrix;
				camMatrix = camMatrix.Invert();
				auto camPos = *(NyaVec3*)&cam->CurrentKey.Position;
				auto camDir = *(NyaVec3*)&cam->CurrentKey.Direction;
				DrawMenuOption(std::format("Camera v0: {:.2f} {:.2f} {:.2f}", camMatrix.x.x, camMatrix.x.y, camMatrix.x.z));
				DrawMenuOption(std::format("Camera v1: {:.2f} {:.2f} {:.2f}", camMatrix.y.x, camMatrix.y.y, camMatrix.y.z));
				DrawMenuOption(std::format("Camera v2: {:.2f} {:.2f} {:.2f}", camMatrix.z.x, camMatrix.z.y, camMatrix.z.z));
				DrawMenuOption(std::format("Camera v3: {:.2f} {:.2f} {:.2f}", camMatrix.p.x, camMatrix.p.y, camMatrix.p.z));
				DrawMenuOption(std::format("Camera Position: {:.2f} {:.2f} {:.2f}", camPos.x, camPos.y, camPos.z));
				DrawMenuOption(std::format("Camera Direction: {:.2f} {:.2f} {:.2f}", camDir.x, camDir.y, camDir.z));
				auto jolly = Camera::JollyRancherResponse.CamMatrix;
				DrawMenuOption(std::format("Jolly Rancher Position: {:.2f} {:.2f} {:.2f}", jolly.p.x, jolly.p.y, jolly.p.z));
				DrawMenuOption(std::format("InGameBreaker: {}", ply->InGameBreaker()));
				DrawMenuOption(std::format("CanRechargeNOS: {}", ply->CanRechargeNOS()));
				DrawMenuOption(std::format("HasNOS: {}", GetLocalPlayerEngine()->HasNOS()));
				DrawMenuOption(std::format("Speed: {:.2f}", GetLocalPlayerVehicle()->GetSpeed()));
				DrawMenuOption(std::format("911 Time: {:.2f}", GetLocalPlayerInterface<IPerpetrator>()->Get911CallTime()));
				DrawMenuOption(std::format("Player Car: {}", FEDatabase->mUserProfile->TheCareerSettings.CurrentCar));
				DrawMenuOption(std::format("Sim Timestep: {:.2f}", Sim::Internal::mSystem->mTimeStep));
				DrawMenuOption(std::format("Sim Speed: {:.2f}", Sim::Internal::mSystem->mSpeed));
				DrawMenuOption(std::format("Sim Target Speed: {:.2f}", Sim::Internal::mSystem->mTargetSpeed));
				auto wheelPos = *GetLocalPlayerInterface<ISuspension>()->GetWheelPos(0);
				DrawMenuOption(std::format("Wheel Position: {:.2f} {:.2f} {:.2f}", wheelPos.x, wheelPos.y, wheelPos.z));
				wheelPos = *GetLocalPlayerInterface<ISuspension>()->GetWheelLocalPos(0);
				DrawMenuOption(std::format("Wheel Local Position: {:.2f} {:.2f} {:.2f}", wheelPos.x, wheelPos.y, wheelPos.z));
				GetLocalPlayerInterface<ISuspension>()->GetWheelCenterPos(&wheelPos, 0);
				DrawMenuOption(std::format("Wheel Center Position: {:.2f} {:.2f} {:.2f}", wheelPos.x, wheelPos.y, wheelPos.z));
				DrawMenuOption(std::format("Wheel Velocity: {:.2f}", GetLocalPlayerInterface<ISuspension>()->GetWheelAngularVelocity(0)));
				DrawMenuOption(std::format("Wheel Steer: {:.2f}", GetLocalPlayerInterface<ISuspension>()->GetWheelSteer(0)));
				auto relativeVelocity = GetRelativeCarOffset(GetLocalPlayerVehicle(), *GetLocalPlayerInterface<IRigidBody>()->GetLinearVelocity());
				DrawMenuOption(std::format("Relative Velocity: {:.2f} {:.2f} {:.2f}", relativeVelocity.x, relativeVelocity.y, relativeVelocity.z));
				relativeVelocity = GetRelativeCarOffset(GetLocalPlayerVehicle(), *GetLocalPlayerInterface<IRigidBody>()->GetAngularVelocity());
				DrawMenuOption(std::format("Relative Turn Velocity: {:.2f} {:.2f} {:.2f}", relativeVelocity.x, relativeVelocity.y, relativeVelocity.z));
				DrawMenuOption(std::format("Suspension Height 1: {:.2f}", GetLocalPlayerInterface<ISuspension>()->GetRideHeight(0)));
				DrawMenuOption(std::format("Suspension Height 2: {:.2f}", GetLocalPlayerInterface<ISuspension>()->GetRideHeight(1)));
				DrawMenuOption(std::format("Suspension Render Motion: {:.2f}", GetLocalPlayerInterface<ISuspension>()->GetRenderMotion()));
				DrawMenuOption(std::format("Racers: {}", VEHICLE_LIST::GetList(VEHICLE_AIRACERS).size()));
				DrawMenuOption(std::format("Cops: {}", VEHICLE_LIST::GetList(VEHICLE_AICOPS).size()));
				DrawMenuOption(std::format("Traffic: {}", VEHICLE_LIST::GetList(VEHICLE_AITRAFFIC).size()));
				if (auto heat = GetMaxHeat()) {
					DrawMenuOption(std::format("Max Heat: {:.2f}", *heat));
				}
				if (auto tune = GetLocalPlayerVehicle()->GetTunings()) {
					DrawMenuOption(std::format("Aerodynamics: {:.2f}", tune->Value[Physics::Tunings::AERODYNAMICS]));
				}
				if (GRaceStatus::fObj && IsInNormalRace()) {
					DrawMenuOption(std::format("Race Completion: {:.2f}", GRaceStatus::fObj->mRacerInfo[0].mPctRaceComplete));
				}
				if (DrawMenuOption("Debug Camera")) {
					CameraAI::SetAction(1, "CDActionDebug");
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
			if (DrawMenuOption("Toggle Custom Camera")) {
				CustomCamera::bRunCustom = !CustomCamera::bRunCustom;
			}
			QuickValueEditor("CarMagnetForce", CarMagnetForce);
			QuickValueEditor("LeakTank::TankDrainRate", Effect_LeakTank::TankDrainRate);
			QuickValueEditor("GroovyCars::GroovySpeed", Effect_GroovyCars::GroovySpeed);
			QuickValueEditor("detachThreshold", CustomCarPart::detachThreshold);
			QuickValueEditor("unlatchThreshold", CustomCarPart::unlatchThreshold);
			QuickValueEditor("latchMoveFactor[0]", CustomCarPart::latchMoveFactor[0]);
			QuickValueEditor("latchMoveFactor[1]", CustomCarPart::latchMoveFactor[1]);
			QuickValueEditor("latchMoveFactor[2]", CustomCarPart::latchMoveFactor[2]);
			QuickValueEditor("latchInitialDetachFactor", CustomCarPart::latchInitialDetachFactor);
			QuickValueEditor("latchBounceFactor", CustomCarPart::latchBounceFactor);
			QuickValueEditor("latchDecayFactor", CustomCarPart::latchDecayFactor);
			auto v = CustomCarPart::lastPassedCol;
			DrawMenuOption(std::format("lastPassedCol: {:.2f} {:.2f} {:.2f}", v.x, v.y, v.z));
			v.Normalize();
			DrawMenuOption(std::format("lastPassedColNorm: {:.2f} {:.2f} {:.2f}", v.x, v.y, v.z));
			QuickValueEditor("Render3DObjects::CollisionStrength", Render3DObjects::CollisionStrength);
			QuickValueEditor("bFO2Minimap", FlatOutHUD::CHUD_Minimap::bFO2Minimap);
			QuickValueEditor("Effect_TikTok::fWordFrequency", Effect_TikTok::fWordFrequency);
			QuickValueEditor("Effect_TikTok::fWordTimerSpeed", Effect_TikTok::fWordTimerSpeed);
			ChloeMenuLib::EndMenu();
		}

		if (DrawMenuOption("Achievement Popup")) {
			Achievements::AwardAchievement(GetAchievement("meow"));
		}

		if (DrawMenuOption("Add Effect")) {
			std::vector<std::string> categories;
			for (auto& effect : ChaosEffect::aEffects) {
				if (std::find(categories.begin(), categories.end(), effect->sListCategory) == categories.end()) {
					categories.push_back(effect->sListCategory);
				}
			}
			std::sort(categories.begin(), categories.end());

			ChloeMenuLib::BeginMenu();
			for (auto& cat : categories) {
				if (DrawMenuOption(cat)) {
					ChloeMenuLib::BeginMenu();
					for (auto& effect : ChaosEffect::aEffects) {
						if (effect->sListCategory != cat) continue;
						if (DrawMenuOption(effect->GetFriendlyName())) {
							AddRunningEffect(effect);
						}
					}
					ChloeMenuLib::EndMenu();
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

			auto configName = "NFSMWChaos_gcp.toml";
			if (std::filesystem::exists(configName)) {
				static auto config = toml::parse_file(configName);
				bDarkMode = config["main"]["dark_mode"].value_or(false);
			}

			for (auto& func : ChloeHook::aHooks) {
				func();
			}

			GetCurrentDirectoryW(MAX_PATH, gDLLDir);

			ChloeMenuLib::RegisterMenu("Cwoee Chaos", &ChaosModMenu);
			std::sort(ChaosEffect::aEffects.begin(),ChaosEffect::aEffects.end(),[] (ChaosEffect* a, ChaosEffect* b) { return (std::string)a->GetFriendlyName() < (std::string)b->GetFriendlyName(); });
			aRunningEffects.reserve(64);
			WriteLog(std::format("Initialized {} effects", ChaosEffect::aEffects.size()));
			DoChaosLoad();

			NyaHooks::PlaceD3DHooks(true);
			NyaHooks::aEndSceneFuncs.push_back(D3DHookMain);
			NyaHooks::aD3DResetFuncs.push_back(OnD3DReset);
			NyaHooks::aPreHUDDrawFuncs.push_back(D3DHookPreHUD);
			NyaHooks::PlaceWndProcHook();
			//NyaHooks::aWndProcFuncs.push_back(MouseWndProc);
			NyaHooks::aWndProcFuncs.push_back(WndProcHook);
			NyaHooks::PlaceWorldServiceHook();
			NyaHooks::aWorldServiceFuncs.push_back(MainLoop);
			NyaHooks::PlaceInputBlockerHook();
			NyaHooks::PlaceCameraMoverHook();
			NyaHooks::aCameraMoverFuncs.push_back(CameraHook);
			NyaHooks::PlaceLateInitHook();
			NyaHooks::PlaceRenderHook();
			NyaHooks::aRenderFuncs.push_back(Render3DLoop);

			// SkipFE
			//*(bool*)0x926064 = 1;
		} break;
		default:
			break;
	}
	return TRUE;
}