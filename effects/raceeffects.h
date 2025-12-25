#define EFFECT_CATEGORY_TEMP "Race"

void SetRaceNumLaps(int lapCount) {
	auto race = GRaceStatus::fObj;
	if (!race) return;
	if (race->mPlayMode == GRaceStatus::kPlayMode_Roaming) return;
	if (!GRaceParameters::GetIsLoopingRace(race->mRaceParms)) return;
	if (GRaceParameters::GetIsPursuitRace(race->mRaceParms)) return;

	if (auto index = race->mRaceParms->mIndex) {
		index->mNumLaps = lapCount;
	}

	auto pLaps = (uint32_t*)Attrib::Instance::GetAttributePointer(race->mRaceParms->mRaceRecord, Attrib::StringHash32("NumLaps"), 0);
	if (pLaps) *pLaps = lapCount;
	if (auto parent = race->mRaceParms->mRaceRecord->mCollection->mParent) {
		pLaps = (uint32_t*)Attrib::Collection::GetData(parent, Attrib::StringHash32("NumLaps"), 0);
		if (pLaps) *pLaps = lapCount;
	}
}

int GetLocalPlayerCurrentLap() {
	return GRaceStatus::fObj->mRacerInfo[0].mLapsCompleted;
}

/*class Effect_RemoveLapProgress : public ChaosEffect {
public:
	Effect_RemoveLapProgress() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Revert Progress By A Lap";
	}

	void InitFunction() override {
		GRaceStatus::fObj->mRacerInfo[0].mLapsCompleted--;
	}
	bool IsAvailable() override {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps < 2) return false;
		return GRaceStatus::fObj->mRacerInfo[0].mLapsCompleted > 0;
	}
	bool IsConditionallyAvailable() override { return true; }
} E_RemoveLapProgress;*/

class Effect_RemoveLap : public ChaosEffect {
public:
	Effect_RemoveLap() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Remove A Lap";
	}

	void InitFunction() override {
		auto laps = *GetRaceNumLaps();
		laps--;
		if (laps <= 1) {
			laps = 1;
			Achievements::AwardAchievement(GetAchievement("LAPS_1"));
		}
		SetRaceNumLaps(laps);
		aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
	}
	bool IsAvailable() override {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps < 2) return false;
		if (!ChaosVoting::IsEnabled() || ChaosVoting::bSelectingEffectsForVote) {
			return GetLocalPlayerCurrentLap() <= 0;
		}
		return true;
	}
	bool AbortOnConditionFailed() override { return true; }
} E_RemoveLap;

class Effect_AddLap : public ChaosEffect {
public:
	Effect_AddLap() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Add A Lap";
	}

	void InitFunction() override {
		auto laps = *GetRaceNumLaps();
		laps++;
		if (laps >= 10) {
			laps = 10;
			Achievements::AwardAchievement(GetAchievement("LAPS_10"));
		}
		SetRaceNumLaps(laps);
		aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
	}
	bool IsAvailable() override {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps >= 10) return false;
		if (!ChaosVoting::IsEnabled() || ChaosVoting::bSelectingEffectsForVote) {
			return GetLocalPlayerCurrentLap() <= 0;
		}
		return true;
	}
	bool AbortOnConditionFailed() override { return true; }
} E_AddLap;

class Effect_Add3Laps : public ChaosEffect {
public:
	Effect_Add3Laps() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Add 3 Laps";
	}

	void InitFunction() override {
		auto laps = *GetRaceNumLaps();
		laps += 3;
		if (laps >= 10) {
			laps = 10;
			Achievements::AwardAchievement(GetAchievement("LAPS_10"));
		}
		SetRaceNumLaps(laps);
		aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
	}
	bool IsAvailable() override {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps >= 10) return false;
		if (!ChaosVoting::IsEnabled() || ChaosVoting::bSelectingEffectsForVote) {
			return GetLocalPlayerCurrentLap() <= 0;
		}
		return true;
	}
	bool AbortOnConditionFailed() override { return true; }
} E_Add3Laps;

class Effect_RestartRace : public EffectBase_InAnyRaceConditional {
public:
	Effect_RestartRace() : EffectBase_InAnyRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Restart Event";
	}

	void InitFunction() override {
		if (GRaceStatus::fObj->mRacerInfo[0].mPctRaceComplete >= 90) {
			Achievements::AwardAchievement(GetAchievement("RESTART_LATE"));
		}
		aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
	}
	bool AbortOnConditionFailed() override { return true; }
} E_RestartRace;

class Effect_DisableBarriers : public EffectBase_InRaceConditional {
public:
	Effect_DisableBarriers() : EffectBase_InRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Race Barriers";
		fTimerLength = 120;
	}

	void TickFunctionMain(double delta) override {
		GRaceStatus::DisableBarriers();
	}
	void DeinitFunction() override {
		if (IsInNormalRace()) {
			GRaceStatus::EnableBarriers(GRaceStatus::fObj);
		}
	}
	bool HasTimer() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_DisableBarriers;

class Effect_FalseStarts : public ChaosEffect {
public:
	Effect_FalseStarts() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "False Starts";
		fTimerLength = 120;
	}

	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->IsStaging()) car->SetStaging(false);
		}
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		if (!IsInNormalRace()) return false;
		//if (EffectInstance && !EffectInstance->bFirstFrame && GRaceStatus::fObj->mRacerInfo[0].mPctRaceComplete > 5.0) {
		//	return false;
		//}
		return true;
	}
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_FalseStarts;

class Effect_RestartRaceOn99 : public ChaosEffect {
public:
	bool active = false;

	Effect_RestartRaceOn99() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Exit To Menu At 99% Completion";
	}

	void InitFunction() override {
		active = false;
	}
	void TickFunctionMain(double delta) override {
		if (!active) {
			EffectInstance->fTimer = fTimerLength;
			if ((IsInNormalRace() && GRaceStatus::fObj->mRacerInfo[0].mPctRaceComplete >= 99) || (cFrontendDatabase::IsFinalEpicChase(FEDatabase) && IsPlayerApproachingOldBridge())) {
				aMainLoopFunctionsOnce.push_back([]() { EQuitToFE::Create(GARAGETYPE_MAIN_FE, "MainMenu.fng"); });
				//aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
				active = true;
			}
		}
	}
	bool HideFromPlayer() override {
		return !active;
	}
	bool IsAvailable() override { return IsInNormalRace() || cFrontendDatabase::IsFinalEpicChase(FEDatabase); }
	bool AbortOnConditionFailed() override { return true; }
	bool RunInMenus() override { return active; }
	bool CanQuickTrigger() override { return false; }
} E_RestartRaceOn99;

class Effect_SuddenDeath : public ChaosEffect {
public:
	Effect_SuddenDeath() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Sudden Death";
		sFriendlyName = "Blow Engine On Position Loss";
		fTimerLength = 60;
		MakeIncompatibleWithFilterGroup("player_godmode");
	}

	int ranking = 0;
	bool abort = false;
	double fLeewayTimer = 0;

	void InitFunction() override {
		ranking = GRaceStatus::fObj->mRacerInfo[0].mRanking;
		abort = false;
		fLeewayTimer = 0;
	}
	void TickFunctionMain(double delta) override {
		if (GetLocalPlayerVehicle()->IsStaging()) {
			ranking = GRaceStatus::fObj->mRacerCount;
		}

		tNyaStringData data;
		data.x = 0.5;
		data.y = 0.85;
		data.size = 0.04;
		data.XCenterAlign = true;
		data.outlinea = 255;
		data.outlinedist = 0.025;
		DrawString(data, std::format("Stay in #{}{}!", ranking, ranking == 1 ? "" : " or better"));

		auto currRanking = GRaceStatus::fObj->mRacerInfo[0].mRanking;
		if (currRanking < ranking) {
			ranking = currRanking;
		}
		if (currRanking > ranking) {
			data.y += data.size;
			data.SetColor(200,0,0,255);
			DrawString(data, std::format("Time Remaining: {:.1f}", 1.0 - fLeewayTimer));

			fLeewayTimer += delta;
			if (fLeewayTimer > 1.0) {
				GetLocalPlayerInterface<IDamageable>()->Destroy();
				abort = true;
			}
		}
		else {
			fLeewayTimer = 0;
		}
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		if (!IsInNormalRace() || GetActiveVehicles(DRIVER_RACER).empty()) {
			ranking = 999;
			return false;
		}
		return true;
	}
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool ShouldAbort() override { return abort; }
	bool CanQuickTrigger() override { return false; }
} E_SuddenDeath;