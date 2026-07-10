void ProcessChaosEffectsMain(double fDeltaTime, bool inMenu, bool blockedByOtherMeans) {
	PerformanceBenchmarker _perf("ProcessChaosEffectsMain");

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
		if (bDisableChaosHUD && !effect.pEffect->bIgnoreHUDState) continue;
		effect.Draw(y, inMenu || blockedByOtherMeans);
		y += 1 - (inMenu ? 0 : effect.Popup.GetOffscreenPercentage());
	}
	ChaosVoting::DrawUI(ChaosVoting::IsEnabled());

	while (RunningEffectsCleanup()) {}
}

template<ChaosEffect::eChaosHook hook>
void ProcessChaosEffects() {
	PerformanceBenchmarker _perf(std::format("ProcessChaosEffects<{}>", (int)hook).c_str());

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
	PerformanceBenchmarker _perf(std::format("ProcessChaosEffects_SetDir<{}>", (int)hook).c_str());

	DLLDirSetter _setdir;
	ProcessChaosEffects<hook>();
}

void MoneyChecker() {
	static ChaosEffect TempEffect("DUMMY", true);
	if (!TempEffect.sName) TempEffect.sName = "(DUMMY) MONEY CHANGE";

	static int cash = 0;
	if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND) {
		cash = FEDatabase->CurrentUserProfiles[0]->TheCareerSettings.CurrentCash;
	}
	else if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING) {
		int currentCash = FEDatabase->CurrentUserProfiles[0]->TheCareerSettings.CurrentCash;
		if (cash != currentCash) {
			static int cashForEffect = 0;
			if (auto effect = GetEffectRunning(&TempEffect)) {
				effect->fTimer = TempEffect.fTimerLength;
				effect->sNameToDisplay = std::format("Money changed (${} -> ${})", cashForEffect, currentCash);
			}
			else {
				cashForEffect = cash;
				if (auto running = AddRunningEffect(&TempEffect)) {
					running->sNameToDisplay = std::format("Money changed (${} -> ${})", cashForEffect, currentCash);
				}
			}
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

void BountyChecker() {
	static ChaosEffect TempEffect("DUMMY", true);
	if (!TempEffect.sName) TempEffect.sName = "(DUMMY) BOUNTY CHANGE";

	static int cash = 0;
	if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND) {
		cash = FEPlayerCarDB::GetTotalBounty(GetPlayerCarDB());
	}
	else if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING) {
		int currentCash = FEPlayerCarDB::GetTotalBounty(GetPlayerCarDB());
		if (cash != currentCash) {
			static int cashForEffect = 0;
			if (auto effect = GetEffectRunning(&TempEffect)) {
				effect->fTimer = TempEffect.fTimerLength;
				effect->sNameToDisplay = std::format("Bounty changed (${} -> ${})", cashForEffect, currentCash);
			}
			else {
				cashForEffect = cash;
				if (auto running = AddRunningEffect(&TempEffect)) {
					running->sNameToDisplay = std::format("Bounty changed ({} -> {})", cashForEffect, currentCash);
				}
			}
			cash = currentCash;
		}
	}
}

ChaosEffect* GetSmartRNGEffect(bool redo = false) {
	if (!nSmartRNG) return nullptr;

	// this might be too much?
	//if (!redo) {
	//	if (nSmartRNG == 2 && PercentageChanceCheck(25)) {
	//		if (CanEffectBeSmartlyPicked(&E_RefillActiveTimers)) return &E_RefillActiveTimers;
	//	}
	//	if (nSmartRNG == 1 && PercentageChanceCheck(25)) {
	//		if (CanEffectBeSmartlyPicked(&E_RefillActiveTimers)) {
	//			nSmartRNG++; // this won't work if smart rng is 1, since it'll be set to 0 before refill can trigger
	//			return &E_RefillActiveTimers;
	//		}
	//	}
	//}

	bool isInFreeroamPursuit = IsInCareerMode() && !IsInNormalRace() && GetLocalPlayerInterface<IVehicleAI>()->GetPursuit();

	// global effects
	if (PercentageChanceCheck(10) || redo) {
		std::vector<ChaosEffect*> effects;
		if (CanEffectBeSmartlyPicked(&E_RestartRaceOn99)) { effects.push_back(&E_RestartRaceOn99); }
		if (CanEffectBeSmartlyPicked(&E_GetBusted)) { effects.push_back(&E_GetBusted); }
		if (CanEffectBeSmartlyPicked(&E_173)) { effects.push_back(&E_173); }
		if (CanEffectBeSmartlyPicked(&E_173Somewhere)) { effects.push_back(&E_173Somewhere); }
		if (!isInFreeroamPursuit && CanEffectBeSmartlyPicked(&E_Safehouse)) { effects.push_back(&E_Safehouse); }
		if (CanEffectBeSmartlyPicked(&E_QTE)) { effects.push_back(&E_QTE); }
		if (CanEffectBeSmartlyPicked(&E_Blind)) { effects.push_back(&E_Blind); }
		if (CanEffectBeSmartlyPicked(&E_PlayerCarSpike1)) { effects.push_back(&E_PlayerCarSpike1); }
		if (CanEffectBeSmartlyPicked(&E_10Effects)) { effects.push_back(&E_10Effects); }
		if (CanEffectBeSmartlyPicked(&E_SpawnCarTruck)) { effects.push_back(&E_SpawnCarTruck); }
		if (CanEffectBeSmartlyPicked(&E_GetBustedInstant)) { effects.push_back(&E_GetBustedInstant); }
		//if (CanEffectBeSmartlyPicked(&E_CrashChance)) { effects.push_back(&E_CrashChance); } // the chance is too low for this one

		if (GetLocalPlayerVehicle()->GetSpeed() < TOMPS(60)) {
			if (CanEffectBeSmartlyPicked(&E_MinSpeed)) { effects.push_back(&E_MinSpeed); }
		}

		if (IsInCareerMode()) {
			if (CanEffectBeSmartlyPicked(&E_Millionaire2)) { effects.push_back(&E_Millionaire2); }
			if (CanEffectBeSmartlyPicked(&E_OverwriteCareerCar)) { effects.push_back(&E_OverwriteCareerCar); }
			if (FEDatabase->CurrentUserProfiles[0]->TheCareerSettings.CurrentBin >= 13) {
				if (CanEffectBeSmartlyPicked(&E_SubtractBounty)) { effects.push_back(&E_SubtractBounty); }
			}
		}

		if (!effects.empty()) {
			return effects[rand() % effects.size()];
		}
	}

	if (IsInNormalRace()) {
		if (CanEffectBeSmartlyPicked(&E_Add3Laps)) return &E_Add3Laps;

		auto completion = GRaceStatus::fObj->GetRacerInfo(GetLocalPlayerSimable())->mPctRaceComplete;

		// above 90% completion -> restart race
		if (completion >= 90) {
			if (CanEffectBeSmartlyPicked(&E_RestartRace)) return &E_RestartRace;
			if (CanEffectBeSmartlyPicked(&E_Safehouse)) return &E_Safehouse;
			// don't disable saving during razor's chapter - that might genuinely be useless
			if (!IsInCareerMode() || FEDatabase->CurrentUserProfiles[0]->TheCareerSettings.CurrentBin > BIN_RAZOR) {
				if (CanEffectBeSmartlyPicked(&E_DisableSave)) return &E_DisableSave;
			}
			if (CanEffectBeSmartlyPicked(&E_GetBustedInstant)) return &E_GetBustedInstant;
		}

		std::vector<ChaosEffect*> effects;
		if (CanEffectBeSmartlyPicked(&E_PlayerCarSpin)) { effects.push_back(&E_PlayerCarSpin); }
		if (CanEffectBeSmartlyPicked(&E_SpinCar2)) { effects.push_back(&E_SpinCar2); }
		if (CanEffectBeSmartlyPicked(&E_PlayerHalfGas)) { effects.push_back(&E_PlayerHalfGas); }
		//if (CanEffectBeSmartlyPicked(&E_PlayerHalfSteering)) { effects.push_back(&E_PlayerHalfSteering); }
		if (CanEffectBeSmartlyPicked(&E_NoInput)) { effects.push_back(&E_NoInput); }
		if (CanEffectBeSmartlyPicked(&E_SetCarRandomAI)) { effects.push_back(&E_SetCarRandomAI); }
		if (CanEffectBeSmartlyPicked(&E_PlayerLag)) { effects.push_back(&E_PlayerLag); }
		if (CanEffectBeSmartlyPicked(&E_Skyfall)) { effects.push_back(&E_Skyfall); }
		if (CanEffectBeSmartlyPicked(&E_LockPlayer)) { effects.push_back(&E_LockPlayer); }
		if (CanEffectBeSmartlyPicked(&E_LockPlayerOrient)) { effects.push_back(&E_LockPlayerOrient); }
		if (CanEffectBeSmartlyPicked(&E_LaunchCarSide)) { effects.push_back(&E_LaunchCarSide); }
		if (CanEffectBeSmartlyPicked(&E_LaunchCarBwd)) { effects.push_back(&E_LaunchCarBwd); }
		if (CanEffectBeSmartlyPicked(&E_CruiseControl)) { effects.push_back(&E_CruiseControl); }
		if (CanEffectBeSmartlyPicked(&E_AddLap)) { effects.push_back(&E_AddLap); }
		if (CanEffectBeSmartlyPicked(&E_PlayerCarTuneAero)) { effects.push_back(&E_PlayerCarTuneAero); }
		if (CanEffectBeSmartlyPicked(&E_PlayerRandomInput)) { effects.push_back(&E_PlayerRandomInput); }
		if (CanEffectBeSmartlyPicked(&E_PlayerResetTransform)) { effects.push_back(&E_PlayerResetTransform); }

		// the mustang and clio will be a lot slower than any other car by this point
		if (IsInCareerMode() && completion < 50 && FEDatabase->CurrentUserProfiles[0]->TheCareerSettings.CurrentBin < 5) {
			if (CanEffectBeSmartlyPicked(&E_SetCarRazor)) { effects.push_back(&E_SetCarRazor); }
			if (CanEffectBeSmartlyPicked(&E_SetCarTRAFPIZZA)) { effects.push_back(&E_SetCarTRAFPIZZA); }
		}

		// tollbooths
		if (GetActiveVehicles(DRIVER_RACER).empty()) {
			if (CanEffectBeSmartlyPicked(&E_CarBouncy)) { effects.push_back(&E_CarBouncy); }
			if (CanEffectBeSmartlyPicked(&E_CarFloaty)) { effects.push_back(&E_CarFloaty); }
			if (CanEffectBeSmartlyPicked(&E_CarFlippy)) { effects.push_back(&E_CarFlippy); }
			if (CanEffectBeSmartlyPicked(&E_CarTumbly)) { effects.push_back(&E_CarTumbly); }
		}

		if (completion < 80 && CanEffectBeSmartlyPicked(&E_RubberbandOpponents)) { effects.push_back(&E_RubberbandOpponents); }
		if (completion < 80 && CanEffectBeSmartlyPicked(&E_Neon)) { effects.push_back(&E_Neon); } // the neon slows down your throttle
		if (completion < 10 && CanEffectBeSmartlyPicked(&E_LeakTank)) { effects.push_back(&E_LeakTank); }
		if (completion < 10 && CanEffectBeSmartlyPicked(&E_Fragile)) { effects.push_back(&E_Fragile); }
		if (completion < 10 && CanEffectBeSmartlyPicked(&E_NoPauseMenu)) { effects.push_back(&E_NoPauseMenu); }

		if (!effects.empty()) {
			return effects[rand() % effects.size()];
		}
	}
	else if (auto pursuit = GetLocalPlayerInterface<IVehicleAI>()->GetPursuit()) {
		auto status = pursuit->GetPursuitStatus();
		if (status == PS_COOL_DOWN) {
			if (GetLocalPlayerInterface<IPerpetrator>()->GetHeat() >= 2.0 && CanEffectBeSmartlyPicked(&E_NoHidingSpots)) { return &E_NoHidingSpots; }

			std::vector<ChaosEffect*> effects;
			if (CanEffectBeSmartlyPicked(&E_RuthlessCopSpawns)) { effects.push_back(&E_RuthlessCopSpawns); }
			if (CanEffectBeSmartlyPicked(&E_TeleportAllCars)) { effects.push_back(&E_TeleportAllCars); }
			if (CanEffectBeSmartlyPicked(&E_Piggyback)) { effects.push_back(&E_Piggyback); }

			if (!effects.empty()) {
				return effects[rand() % effects.size()];
			}
		}
		else if (status == PS_INITIAL_CHASE || status == PS_BACKUP_REQUESTED) {
			std::vector<ChaosEffect*> effects;
			if (CanEffectBeSmartlyPicked(&E_PlayerCarReset)) { effects.push_back(&E_PlayerCarReset); }
			if (CanEffectBeSmartlyPicked(&E_InvincibleCops)) { effects.push_back(&E_InvincibleCops); }
			if (CanEffectBeSmartlyPicked(&E_SetCopMassInf)) { effects.push_back(&E_SetCopMassInf); }
			if (CanEffectBeSmartlyPicked(&E_SetCarMass0)) { effects.push_back(&E_SetCarMass0); }
			if (CanEffectBeSmartlyPicked(&E_BlowEngine)) { effects.push_back(&E_BlowEngine); }
			if (CanEffectBeSmartlyPicked(&E_PlayerCarSpikeAll)) { effects.push_back(&E_PlayerCarSpikeAll); }
			if (CanEffectBeSmartlyPicked(&E_PlayerCarSpin)) { effects.push_back(&E_PlayerCarSpin); }
			if (CanEffectBeSmartlyPicked(&E_NoInput)) { effects.push_back(&E_NoInput); }
			if (CanEffectBeSmartlyPicked(&E_LeakTank)) { effects.push_back(&E_LeakTank); }
			if (CanEffectBeSmartlyPicked(&E_Piggyback)) { effects.push_back(&E_Piggyback); }
			if (CanEffectBeSmartlyPicked(&E_LockPlayerOrient)) { effects.push_back(&E_LockPlayerOrient); }
			if (CanEffectBeSmartlyPicked(&E_CruiseControl)) { effects.push_back(&E_CruiseControl); }
			if (CanEffectBeSmartlyPicked(&E_CarBouncy)) { effects.push_back(&E_CarBouncy); }
			if (CanEffectBeSmartlyPicked(&E_PlayerCarTuneAero)) { effects.push_back(&E_PlayerCarTuneAero); }
			if (CanEffectBeSmartlyPicked(&E_Fragile)) { effects.push_back(&E_Fragile); }
			if (CanEffectBeSmartlyPicked(&E_PlayerRandomInput)) { effects.push_back(&E_PlayerRandomInput); }

			if (!effects.empty()) {
				return effects[rand() % effects.size()];
			}
		}
	}

	if (!redo) {
		return GetSmartRNGEffect(true);
	}
	nSmartRNG = 0;
	return nullptr;
}

void ChaosLoop() {
	PerformanceBenchmarker _perf("ChaosLoop");

	if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) {
		nNumPlayerCarChangesThisRace = 0;
	}

	static bool bOnce = true;
	if (bOnce) {
		DoChaosLoad();
		g_VisualTreatment = true;
		bOnce = false;
	}

	MoneyChecker();
	BountyChecker();

	for (auto& func : aDrawingLoopFunctions) {
		func();
	}

	for (auto& func : aDrawingLoopFunctionsOnce) {
		func();
	}
	aDrawingLoopFunctionsOnce.clear();

	static CNyaTimer gTimer;
	gTimer.Process();

	if (ChaosVoting::IsEnabled()) {
		ChaosVoting::pAllOfTheAbove = &E_VotingAll; // this is such a hack lol woof meow
		ChaosVoting::Update();
	}
	else if (ChaosVoting::bAutoReconnect && ChaosVoting::sChannelName[0]) {
		ChaosVoting::Reconnect();
	}
	bRNGCycles = !ChaosVoting::IsEnabled();

	if (IsInSplashScreenOrIntros()) {
		return;
	}

	for (auto& effect : aEffectsFromSavegameInstant) {
		AddRunningEffect(effect);
	}
	aEffectsFromSavegameInstant.clear();

	if (IsChaosBlocked()) {
		bool inMenu = TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND;
		ProcessChaosEffectsMain(gTimer.fDeltaTime, inMenu, !inMenu);
		return;
	}
	ProcessChaosEffectsMain(gTimer.fDeltaTime, false, false);

	// just to rub it in a little
	static bool bSMSSent = false;
	if (!bSMSSent && nTimesBusted > 0 && IsInCareerMode() && GetUserProfile()->TheCareerSettings.CurrentBin <= 15 && !IsInAnyRace() && !GetLocalPlayerInterface<IPerpetrator>()->IsBeingPursued()) {
		SendSMS(98, true, true);
		bSMSSent = true;
	}

	static ChaosEffect SplashEffect("DUMMY", true);
	if (!SplashEffect.sName) {
		static char tmp[256];
		strcpy_s(tmp, 256, std::format("Cwoee Chaos v{} by gaycoderprincess", CWOEECHAOS_VERSION).c_str());
		SplashEffect.sName = tmp;
		AddRunningEffect(&SplashEffect);
	}

	for (auto& effect : aEffectsFromSavegameIngame) {
		AddRunningEffect(effect, true);
	}
	aEffectsFromSavegameIngame.clear();

	if (bTimerEnabled) {
		PerformanceBenchmarker _perf("ChaosLoop(bTimerEnabled)");

		static ChaosEffect ActiveEffect("DUMMY", true);
		if (!ActiveEffect.sName) {
			ActiveEffect.sName = "mod active! awruff :3";
			AddRunningEffect(&ActiveEffect);
		}

		fTimeSinceLastEffect += gTimer.fDeltaTime * fEffectCycleTimerSpeedMult;
		static auto textureL = LoadTexture("CwoeeChaos/data/textures/effectbar.png");
		static auto textureD = LoadTexture("CwoeeChaos/data/textures/effectbar_dark.png");
		auto texture = bDarkMode ? textureD : textureL;
		NyaDrawing::CNyaRGBA32 rgb = bDarkMode ? NyaDrawing::CNyaRGBA32(133,122,168,255) : NyaDrawing::CNyaRGBA32(243,138,175,255);
		if (!bDisableChaosHUD) DrawBottomBar(fTimeSinceLastEffect / fEffectCycleTimer, rgb, texture);

		// add a lil bit of extra time so effects don't overlap each other by a frame or two
		float cycleTimer = fEffectCycleTimer + 0.1;
		if (fTimeSinceLastEffect >= cycleTimer) {
			fTimeSinceLastEffect -= cycleTimer;
			if (auto effect = GetSmartRNGEffect()) {
				effect->bTriggeredThisCycleSmart = true;
				nSmartRNG--;
				AddRunningEffect(effect);
			}
			else if (ChaosVoting::IsEnabled()) {
				ChaosVoting::TriggerHighestVotedEffect();
			}
			else {
				AddRunningEffect(GetRandomEffect());
			}
		}

		if (bManualKeybindEnabled && IsKeyJustPressed(VK_OEM_PLUS)) {
			AddRunningEffect(GetRandomEffect(true));
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

	auto tmp = fEffectCycleTimer;
	QuickValueEditor("Cycle Timer", fEffectCycleTimer);
	if (tmp != fEffectCycleTimer) {
		if (fEffectCycleTimer < tmp) fTimeSinceLastEffect = 0;
		if (fEffectCycleTimer < 5) fEffectCycleTimer = 5;
	}

	if (DrawMenuOption("UI")) {
		ChloeMenuLib::BeginMenu();
		if (DrawMenuOption(std::format("Dark Mode - {}", bDarkMode))) {
			bDarkMode = !bDarkMode;
		}
		QuickValueEditor("fEffectX", fEffectX);
		QuickValueEditor("fEffectY", fEffectY);
		QuickValueEditor("fEffectSize", fEffectSize);
		QuickValueEditor("fEffectSpacing", fEffectSpacing);
		QuickValueEditor("fEffectVotingSize", fEffectVotingSize);
		ChloeMenuLib::EndMenu();
	}

	if (DrawMenuOption("Voting")) {
		ChloeMenuLib::BeginMenu();
		QuickValueEditor("Voting Options", ChaosVoting::nNumVoteOptions);
		QuickValueEditor("Two-Character Vote Command", ChaosVoting::bCVotes);
		QuickValueEditor("Proportional Voting", ChaosVoting::bProportionalVotes);
		if (!ChaosVoting::bProportionalVotes) {
			if (DrawMenuOption(std::format("Tiebreaker - {}", ChaosVoting::bTiebreakerRandom ? "Randomly Pick One Effect" : "Activate All Tied Effects"))) {
				ChaosVoting::bTiebreakerRandom = !ChaosVoting::bTiebreakerRandom;
			}
		}
		QuickValueEditor("Random Effect Option", ChaosVoting::bRandomEffectOption);
		QuickValueEditor("Add Effects With Num+", bManualKeybindEnabled);
		if (ChaosVoting::IsEnabled()) {
			QuickValueEditor("Auto-Reconnect", ChaosVoting::bAutoReconnect);
			if (DrawMenuOption("Disconnect From Channel")) {
				ChaosVoting::Disconnect();
			}
		}
		else {
			QuickValueEditor("Twitch Channel Name", ChaosVoting::sChannelName, sizeof(ChaosVoting::sChannelName));
			if (ChaosVoting::sChannelName[0]) {
				if (DrawMenuOption("Connect To Channel")) {
					ChaosVoting::Connect();
				}
			}
		}
		ChloeMenuLib::EndMenu();
	}

	if (DrawMenuOption("Effects")) {
		ChloeMenuLib::BeginMenu();
		QuickValueEditor("Allow multiple Vergils", Effect_Vergil::bAllowMultipleVergils);
		QuickValueEditor("Despawn SCP-173 after a kill", Effect_173::bDespawnPeanuts);
		QuickValueEditor("Disable epileptic effects", bDisableEpilepticEffects);
		ChloeMenuLib::EndMenu();
	}

	if (DrawMenuOption("Stats")) {
		ChloeMenuLib::BeginMenu();
		if (DrawMenuOption("Achievement Stats")) {
			ChloeMenuLib::BeginMenu();
			for (auto& ach : Achievements::gAchievements) {
				if (!ach->bUnlocked && ach->bHidden) continue;
				DrawMenuOption(std::format("{} - {}", ach->sName, ach->bUnlocked ? "Unlocked" : "Locked"));
				DrawMenuOption(ach->sDescription);
				DrawMenuOption(" ", "", true);
			}
			ChloeMenuLib::EndMenu();
		}
		if (DrawMenuOption("Effect Stats")) {
			ChloeMenuLib::BeginMenu();
			auto effects = ChaosEffect::aEffects;
			std::sort(effects.begin(),effects.end(),[] (ChaosEffect* a, ChaosEffect* b) { return a->nTotalTimesActivated > b->nTotalTimesActivated; });
			for (auto& effect : effects) {
				DrawMenuOption(std::format("{} - {} times", effect->sName, effect->nTotalTimesActivated));
			}
			ChloeMenuLib::EndMenu();
		}
		DrawMenuOption(std::format("Times Busted - {}", nTimesBusted));
		ChloeMenuLib::EndMenu();
	}

	if (DrawMenuOption("Debug")) {
		ChloeMenuLib::BeginMenu();
		if (SM64::bAvailable && DrawMenuOption("Mario Debug")) {
			ChloeMenuLib::BeginMenu();

			if (DrawMenuOption("mario enable")) {
				SM64::EnableMario();
				CustomCamera::bRunCustom = true;
			}

			if (DrawMenuOption("mario disable")) {
				SM64::DisableMario();
				CustomCamera::bRunCustom = false;
			}

			if (DrawMenuOption("mario music")) {
				sm64_play_music(0, 0x03, 0);
			}

			QuickValueEditor("marioLightness", SM64::marioLightness);
			QuickValueEditor("marioLightnessMenu", SM64::marioLightnessMenu);

			DrawMenuOption(std::format("position {:.2f} {:.2f} {:.2f}",SM64::marioState.position[0],SM64::marioState.position[1],SM64::marioState.position[2]));
			DrawMenuOption(std::format("velocity {:.2f} {:.2f} {:.2f}",SM64::marioState.velocity[0],SM64::marioState.velocity[1],SM64::marioState.velocity[2]));
			DrawMenuOption(std::format("marioId {}",SM64::marioId));
			DrawMenuOption(std::format("numTrianglesUsed {}",SM64::marioGeometry.numTrianglesUsed));
			DrawMenuOption(std::format("geom 0 {:.2f} {:.2f} {:.2f}",SM64::marioGeometry.position[0],SM64::marioGeometry.position[1],SM64::marioGeometry.position[2]));
			DrawMenuOption(std::format("geom 1 {:.2f} {:.2f} {:.2f}",SM64::marioGeometry.position[3],SM64::marioGeometry.position[4],SM64::marioGeometry.position[5]));
			DrawMenuOption(std::format("geom 2 {:.2f} {:.2f} {:.2f}",SM64::marioGeometry.position[6],SM64::marioGeometry.position[7],SM64::marioGeometry.position[8]));

			auto bak = SM64::marioScalar;
			QuickValueEditor("marioScalar", SM64::marioScalar);
			if (bak != SM64::marioScalar) {
				NyaVec3 v = {0,0,0};
				if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
					v = *ply->GetPosition();
					v.y -= 1;
				}

				SM64::ResetMario(v);
			}

			if (DrawMenuOption("mario teleport")) {
				NyaVec3 v = {0,0,0};
				if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
					v = *ply->GetPosition();
					v.y -= 1;
				}

				SM64::ResetMario(v);
			}

			auto worldFacing = SM64::GetMarioWorldFacing();
			DrawMenuOption(std::format("facing {:.2f} {:.2f} {:.2f}",worldFacing[0],worldFacing[1],worldFacing[2]));

			if (DrawMenuOption("mario teleport fwd")) {
				auto v = SM64::GetMarioWorldPos();
				v += SM64::GetMarioWorldFacing() * 5;
				SM64::ResetMario(v);
			}

			if (DrawMenuOption("mario damage")) {
				sm64_mario_take_damage(SM64::marioId, 1, 0, 0, 0, 0);
			}

			if (DrawMenuOption("add wing cap")) {
				sm64_mario_interact_cap(SM64::marioId, MARIO_WING_CAP, 0, 0);
			}

			if (!SM64::aCollisionTris.empty()) {
				auto col = SM64::aCollisionTris[0];
				DrawMenuOption(std::format("col 0 {:.2f} {:.2f} {:.2f}",col.fPt0[0],col.fPt0[1],col.fPt0[2]));
				DrawMenuOption(std::format("col 1 {:.2f} {:.2f} {:.2f}",col.fPt1[0],col.fPt1[1],col.fPt1[2]));
				DrawMenuOption(std::format("col 2 {:.2f} {:.2f} {:.2f}",col.fPt2[0],col.fPt2[1],col.fPt2[2]));
			}

			ChloeMenuLib::EndMenu();
		}
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
					FEPlayerCarDB::CreateNewCareerCar(GetPlayerCarDB(), car->Handle);
				}
			}
			if (DrawMenuOption("Capture Car And Camera Positions")) {
				auto cam = GetLocalPlayerCamera();
				auto camMat = PrepareCameraMatrix(cam);

				auto file = std::ofstream("camera.dat", std::ios::out | std::ios::binary);
				file.write((char*)&camMat, sizeof(camMat));
				file.write((char*)&cam->CurrentKey.FieldOfView, sizeof(cam->CurrentKey.FieldOfView));
				auto file2 = std::ofstream("carrender.dat", std::ios::out | std::ios::binary);
				file2.write((char*)&CarRender_LastMatrix, sizeof(CarRender_LastMatrix));
			}
			if (DrawMenuOption("SMS Inbox")) {
				ChloeMenuLib::BeginMenu();

				for (int i = 0; i < 150; i++) {
					auto content = FEngHashString(std::format("SMS_MESSAGE_{}", i).c_str());
					auto from = FEngHashString(std::format("SMS_MESSAGE_{}_FROM", i).c_str());
					auto subject = FEngHashString(std::format("SMS_MESSAGE_{}_SUBJECT", i).c_str());
					DrawMenuOption(std::format("Message {}", i));
					DrawMenuOption(std::format("From: {}", GetLocalizedString(from)));
					DrawMenuOption(std::format("Subject: {}", GetLocalizedString(subject)));
					//DrawMenuOption(GetLocalizedString(content));
				}

				ChloeMenuLib::EndMenu();
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
				DrawMenuOption(std::format("Player Car: {}", FEDatabase->CurrentUserProfiles[0]->TheCareerSettings.CurrentCar));
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
				DrawMenuOption(std::format("CurrentBin: {}", FEDatabase->CurrentUserProfiles[0]->TheCareerSettings.CurrentBin));
				DrawMenuOption(std::format("IsFinalEpicChase: {}", FEDatabase->IsFinalEpicChase()));
				if (auto heat = GetMaxHeat()) {
					DrawMenuOption(std::format("Max Heat: {:.2f}", *heat));
				}
				if (auto tune = GetLocalPlayerVehicle()->GetTunings()) {
					DrawMenuOption(std::format("Aerodynamics: {:.2f}", tune->Value[Physics::Tunings::AERODYNAMICS]));
				}
				if (GRaceStatus::fObj && IsInNormalRace()) {
					DrawMenuOption(std::format("Race Completion: {:.2f}", GRaceStatus::fObj->GetRacerInfo(GetLocalPlayerSimable())->mPctRaceComplete));
				}
				DrawMenuOption(std::format("SoundEffectsVol: {}", FEDatabase->CurrentUserProfiles[0]->TheOptionsSettings.TheAudioSettings.SoundEffectsVol));
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
			if (DrawMenuOption("CollView")) {
				ChloeMenuLib::BeginMenu();

				if (DrawMenuOption("Toggle CollView")) {
					CollView::bEnabled = !CollView::bEnabled;
				}

				QuickValueEditor("bWireframeGround", CollView::bWireframeGround);
				QuickValueEditor("bWireframeBarriers", CollView::bWireframeBarriers);
				QuickValueEditor("bWireframeCars", CollView::bWireframeCars);

				QuickValueEditor("nWallColR", CollView::nWallColR);
				QuickValueEditor("nWallColG", CollView::nWallColG);
				QuickValueEditor("nWallColB", CollView::nWallColB);

				QuickValueEditor("nGroundColR", CollView::nGroundColR);
				QuickValueEditor("nGroundColG", CollView::nGroundColG);
				QuickValueEditor("nGroundColB", CollView::nGroundColB);

				QuickValueEditor("nCarColR", CollView::nCarColR);
				QuickValueEditor("nCarColG", CollView::nCarColG);
				QuickValueEditor("nCarColB", CollView::nCarColB);

				ChloeMenuLib::EndMenu();
			}
			if (DrawMenuOption("Trigger Busted")) {
				auto tmp = (NISListenerActivity*)nullptr;
				tmp->MessageBusted(0);
			}
			//QuickValueEditor("SceneryScale.x", SceneryScale.x);
			//QuickValueEditor("SceneryScale.y", SceneryScale.y);
			//QuickValueEditor("SceneryScale.z", SceneryScale.z);
			//QuickValueEditor("SceneryMove.x", SceneryMove.x);
			//QuickValueEditor("SceneryMove.y", SceneryMove.y);
			//QuickValueEditor("SceneryMove.z", SceneryMove.z);
			//QuickValueEditor("DanceGame::fYOffset", Effect_DanceGame::fYOffset);
			QuickValueEditor("Scientist::rX", Effect_Scientist::rX);
			QuickValueEditor("Scientist::rY", Effect_Scientist::rY);
			QuickValueEditor("Scientist::rZ", Effect_Scientist::rZ);
			QuickValueEditor("Scientist::scale", Effect_Scientist::scale);
			QuickValueEditor("Scientist::colScale", Effect_Scientist::colScale);
			QuickValueEditor("Vergil::offX", Effect_Vergil::offX);
			QuickValueEditor("Vergil::offY", Effect_Vergil::offY);
			QuickValueEditor("Vergil::offZ", Effect_Vergil::offZ);
			QuickValueEditor("Vergil::rX", Effect_Vergil::rX);
			QuickValueEditor("Vergil::rY", Effect_Vergil::rY);
			QuickValueEditor("Vergil::rZ", Effect_Vergil::rZ);
			QuickValueEditor("Vergil::scale", Effect_Vergil::scale);
			QuickValueEditor("Vergil::colScale", Effect_Vergil::colScale);
			QuickValueEditor("Vergil::attackFrequency", Effect_Vergil::attackFrequency);
			QuickValueEditor("Vergil::sfxVolume", Effect_Vergil::sfxVolume);
			QuickValueEditor("Vergil::attackVolume", Effect_Vergil::attackVolume);
			QuickValueEditor("Vergil::styleVolume", Effect_Vergil::styleVolume);
			QuickValueEditor("Vergil::sfxRange", Effect_Vergil::sfxRange);
			QuickValueEditor("Vergil::targetRange", Effect_Vergil::targetRange);
			QuickValueEditor("Vergil::attackRange", Effect_Vergil::attackRange);
			QuickValueEditor("Vergil::attackPower", Effect_Vergil::attackPower);
			QuickValueEditor("Vergil::attackPowerAng", Effect_Vergil::attackPowerAng);
			QuickValueEditor("Vergil::attackStyleIncrease", Effect_Vergil::attackStyleIncrease);
			QuickValueEditor("Vergil::styleDecay", Effect_Vergil::styleDecay);
			QuickValueEditor("CarMagnetForce", CarMagnetForce);
			QuickValueEditor("ReVoltFirework::offX", Effect_ReVoltFirework::offX);
			QuickValueEditor("ReVoltFirework::offY", Effect_ReVoltFirework::offY);
			QuickValueEditor("ReVoltFirework::offZ", Effect_ReVoltFirework::offZ);
			QuickValueEditor("ReVoltFirework::rotOffX", Effect_ReVoltFirework::rotOffX);
			QuickValueEditor("ReVoltFirework::rotOffY", Effect_ReVoltFirework::rotOffY);
			QuickValueEditor("ReVoltFirework::rotOffZ", Effect_ReVoltFirework::rotOffZ);
			QuickValueEditor("ReVoltFirework::scale", Effect_ReVoltFirework::scale);
			QuickValueEditor("ReVoltFirework::rotSpeed", Effect_ReVoltFirework::rotSpeed);
			QuickValueEditor("ReVoltFirework::moveSpeed", Effect_ReVoltFirework::moveSpeed);
			QuickValueEditor("ReVoltFirework::inFrontThreshold", Effect_ReVoltFirework::inFrontThreshold);
			QuickValueEditor("ReVoltFirework::crosshairSize", Effect_ReVoltFirework::crosshairSize);
			QuickValueEditor("173::peanutSpeed", Effect_173::peanutSpeed);
			QuickValueEditor("173::lastPeanutDot", Effect_173::lastPeanutDot);
			QuickValueEditor("173::lastPeanutDistance", Effect_173::lastPeanutDistance);
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

		if (DrawMenuOption("Performance Debug")) {
			ChloeMenuLib::BeginMenu();

			for (auto& perf : aPerformanceBenchmarkResults) {
				DrawMenuOption(std::format("{}: {}ms", perf.name, perf.ms));
			}

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
			for (auto& effect: aRunningEffects) {
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
					if (!effect->bCanQuickTrigger) continue;

					fout << effect->GetFriendlyName();
					if (effect->sFriendlyName) fout << std::format(" ({})", effect->sName);
					fout << "\n";
				}
			}
		}

		if (DrawMenuOption("Dump Cheat Codes")) {
			std::ofstream fout("cwoee_cheat_codes.txt", std::ios::out);
			if (fout.is_open()) {
				for (auto& effect : ChaosEffect::aEffects) {
					if (!effect->bCanQuickTrigger) continue;

					fout << effect->GetCheatCode(true);
					if (effect->sFriendlyName) fout << std::format(" ({})", effect->GetCheatCode(false));
					fout << "\n";
				}
			}
		}

		ChloeMenuLib::EndMenu();
	}

	ChloeMenuLib::EndMenu();
}