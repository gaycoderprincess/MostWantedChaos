#define EFFECT_CATEGORY_TEMP "Race"

uint8_t* GetRaceNumLaps() {
	auto race = GRaceStatus::fObj;
	if (!race) return nullptr;
	if (race->mPlayMode == GRaceStatus::kPlayMode_Roaming) return nullptr;
	if (!GRaceParameters::GetIsLoopingRace(race->mRaceParms)) return nullptr;
	if (auto index = race->mRaceParms->mIndex) {
		return &index->mNumLaps;
	}
	return (uint8_t*)Attrib::Instance::GetAttributePointer(race->mRaceParms->mRaceRecord, Attrib::StringHash32("NumLaps"), 0);
}

void SetRaceNumLaps(int lapCount) {
	auto race = GRaceStatus::fObj;
	if (!race) return;
	if (race->mPlayMode == GRaceStatus::kPlayMode_Roaming) return;
	if (!GRaceParameters::GetIsLoopingRace(race->mRaceParms)) return;

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
		return GetLocalPlayerCurrentLap() <= 0;
	}
	bool IsConditionallyAvailable() override { return true; }
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
		return GetLocalPlayerCurrentLap() <= 0;
	}
	bool IsConditionallyAvailable() override { return true; }
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
		return GetLocalPlayerCurrentLap() <= 0;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_Add3Laps;

class Effect_RestartRace : public ChaosEffect {
public:
	Effect_RestartRace() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Restart Race";
	}

	void InitFunction() override {
		if (GRaceStatus::fObj->mRacerInfo[0].mPctRaceComplete >= 90) {
			Achievements::AwardAchievement(GetAchievement("RESTART_LATE"));
		}
		aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
	}
	bool IsAvailable() override {
		return GRaceStatus::fObj->mPlayMode == GRaceStatus::kPlayMode_Racing;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_RestartRace;

class Effect_DisableBarriers : public ChaosEffect {
public:
	Effect_DisableBarriers() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Race Barriers";
	}

	void TickFunction(double delta) override {
		GRaceStatus::DisableBarriers();
	}
	void DeinitFunction() override {
		if (GRaceStatus::fObj->mPlayMode == GRaceStatus::kPlayMode_Racing) {
			GRaceStatus::EnableBarriers(GRaceStatus::fObj);
		}
	}
	bool IsAvailable() override {
		return GRaceStatus::fObj->mPlayMode == GRaceStatus::kPlayMode_Racing;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_DisableBarriers;