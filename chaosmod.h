void ProcessChaosEffectsMain(double fDeltaTime, bool inMenu, bool blockedByOtherMeans) {
	// run effects first, then draw the chaos HUD over top
	for (auto& effect : aRunningEffects) {
		if (inMenu && !effect.pEffect->RunInMenus()) continue;
		if (blockedByOtherMeans && !effect.pEffect->RunWhenBlocked()) continue;
		effect.OnTickMain(fDeltaTime, inMenu || blockedByOtherMeans);
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

template<ChaosEffect::eChaosHook hook>
void ProcessChaosEffects() {
	static CNyaTimer gTimer;
	gTimer.Process();

	auto inMenu = TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND;
	auto isBlocked = IsChaosBlocked();
	for (auto& effect : aRunningEffects) {
		if (inMenu && !effect.pEffect->RunInMenus()) continue;
		if (isBlocked && !effect.pEffect->RunWhenBlocked()) continue;
		effect.OnTick(hook, gTimer.fDeltaTime);
	}
}

template<ChaosEffect::eChaosHook hook>
void ProcessChaosEffects_SetDir() {
	DLLDirSetter _setdir;
	ProcessChaosEffects<hook>();
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
			static int cashForEffect = 0;
			if (auto effect = GetEffectRunning(&TempEffect)) {
				effect->fTimer = TempEffect.fTimerLength;
			}
			else {
				cashForEffect = cash;
				AddRunningEffect(&TempEffect);
			}
			static std::string name;
			name = std::format("Money changed (${} -> ${})", cashForEffect, currentCash);
			TempEffect.sName = name.c_str();
			cash = currentCash;

			if (currentCash >= 2000000) {
				Achievements::AwardAchievement(GetAchievement("MILLIONAIRE"));
			}
			if (currentCash <= -2000000) {
				Achievements::AwardAchievement(GetAchievement("ANTI_MILLIONAIRE"));
			}
		}
	}
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

	if (ChaosVoting::bEnabled) {
		ChaosVoting::pAllOfTheAbove = &E_VotingAll; // this is such a hack lol woof meow
		ChaosVoting::DrawUI();
	}

	if (IsChaosBlocked()) {
		bool inMenu = TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND;
		ProcessChaosEffectsMain(gTimer.fDeltaTime, inMenu, !inMenu);
		return;
	}
	ProcessChaosEffectsMain(gTimer.fDeltaTime, false, false);

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
			if (ChaosVoting::bEnabled) {
				ChaosVoting::TriggerHighestVotedEffect();
			}
			else {
				AddRunningEffect(GetRandomEffect());
			}
		}
	}
	else {
		fTimeSinceLastEffect = 0;
	}
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
		QuickValueEditor("fEffectVotingSize", fEffectVotingSize);
		ChloeMenuLib::EndMenu();
	}

	if (DrawMenuOption("Debug")) {
		ChloeMenuLib::BeginMenu();
		if (DrawMenuOption("Player Debug")) {
			ChloeMenuLib::BeginMenu();
			if (DrawMenuOption("Change C6 to Cross")) {
				if (auto model = GetPVehicleModelPointer(Attrib::StringHash32("cs_c6_copsporthench"))) {
					*model = "COPSPORT";
				}
			}
			if (DrawMenuOption("Change Pizza to Coup")) {
				if (auto model = GetPVehicleModelPointer(Attrib::StringHash32("cs_clio_trafpizza"))) {
					*model = "TRAFFICCOUP";
				}
			}
			if (DrawMenuOption("Add CopCross To Garage")) {
				if (auto car = CreateStockCarRecord("copcross")) {
					FEPlayerCarDB::CreateNewCareerCar(&FEDatabase->mUserProfile->PlayersCarStable, car->Handle);
				}
			}
			if (auto ply = GetLocalPlayer()) {
				DrawMenuOption(std::format("IPlayer: {:X}", (uintptr_t)ply));
				DrawMenuOption(std::format("ISimable: {:X}", (uintptr_t)ply->GetSimable()));
				DrawMenuOption(std::format("IHud: {:X}", (uintptr_t)ply->GetHud()));
				DrawMenuOption(std::format("IVehicle: {:X}", (uintptr_t)ply->GetSimable()->mCOMObject->Find<IVehicle>()));
				DrawMenuOption(std::format("Car Model: {}", GetLocalPlayerVehicle()->GetVehicleName()));
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
				DrawMenuOption(std::format("CurrentBin: {}", FEDatabase->mUserProfile->TheCareerSettings.CurrentBin));
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
			if (DrawMenuOption("Toggle Voting Test")) {
				ChaosVoting::bEnabled = !ChaosVoting::bEnabled;
			}
			//QuickValueEditor("SceneryScale.x", SceneryScale.x);
			//QuickValueEditor("SceneryScale.y", SceneryScale.y);
			//QuickValueEditor("SceneryScale.z", SceneryScale.z);
			//QuickValueEditor("SceneryMove.x", SceneryMove.x);
			//QuickValueEditor("SceneryMove.y", SceneryMove.y);
			//QuickValueEditor("SceneryMove.z", SceneryMove.z);
			QuickValueEditor("CarMagnetForce", CarMagnetForce);
			QuickValueEditor("MinSpeed::fTextY", Effect_MinSpeed::fTextY);
			QuickValueEditor("LeakTank::TankDrainRate", Effect_LeakTank::TankDrainRate);
			QuickValueEditor("LeakTankCash::TankDrainRate", Effect_LeakTankCash::TankDrainRate);
			QuickValueEditor("GroovyCars::GroovySpeed", Effect_GroovyCars::GroovySpeed);
			QuickValueEditor("HeatSteer::DeltaMult", Effect_HeatSteer::DeltaMult);
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

		if (DrawMenuOption("Dump Quick-Triggerable Effect List")) {
			std::ofstream fout("cwoee_effects_quick.txt", std::ios::out);
			if (fout.is_open()) {
				for (auto& effect : ChaosEffect::aEffects) {
					if (!effect->CanQuickTrigger()) continue;

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