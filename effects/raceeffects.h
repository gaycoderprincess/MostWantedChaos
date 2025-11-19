uint8_t* GetRaceNumLaps() {
	auto race = GRaceStatus::fObj;
	if (!race) return nullptr;
	if (race->mPlayMode == GRaceStatus::kPlayMode_Roaming) return nullptr;
	if (!GRaceParameters::GetIsLoopingRace(race->mRaceParms)) return nullptr;
	auto index = race->mRaceParms->mIndex;
	if (!index) {
		return (uint8_t*)Attrib::Instance::GetAttributePointer(race->mRaceParms->mRaceRecord, Attrib::StringHash32("NumLaps"), 0);
	}
	return &index->mNumLaps;
}

int GetLocalPlayerCurrentLap() {
	return GRaceStatus::fObj->mRacerInfo[0].mLapsCompleted;
}

/*class Effect_RemoveLapProgress : public ChaosEffect {
public:
	Effect_RemoveLapProgress() : ChaosEffect() {
		sName = "Revert Progress By A Lap";
	}

	virtual void InitFunction() {
		GRaceStatus::fObj->mRacerInfo[0].mLapsCompleted--;
	}
	virtual bool IsAvailable() {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps < 2) return false;
		return GRaceStatus::fObj->mRacerInfo[0].mLapsCompleted > 0;
	}
	virtual bool IsConditionallyAvailable() { return true; }
} E_RemoveLapProgress;*/

class Effect_RemoveLap : public ChaosEffect {
public:
	Effect_RemoveLap() : ChaosEffect() {
		sName = "Remove A Lap";
	}

	virtual void InitFunction() {
		(*GetRaceNumLaps())--;
		ERestartRace::Create();
	}
	virtual bool IsAvailable() {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps < 2) return false;
		return GetLocalPlayerCurrentLap() <= 0;
	}
	virtual bool IsConditionallyAvailable() { return true; }
} E_RemoveLap;

class Effect_AddLap : public ChaosEffect {
public:
	Effect_AddLap() : ChaosEffect() {
		sName = "Add A Lap";
	}

	virtual void InitFunction() {
		(*GetRaceNumLaps())++;
		ERestartRace::Create();
	}
	virtual bool IsAvailable() {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps >= 10) return false;
		return GetLocalPlayerCurrentLap() <= 0;
	}
	virtual bool IsConditionallyAvailable() { return true; }
} E_AddLap;

class Effect_Add3Laps : public ChaosEffect {
public:
	Effect_Add3Laps() : ChaosEffect() {
		sName = "Add 3 Laps";
	}

	virtual void InitFunction() {
		auto laps = GetRaceNumLaps();
		(*laps) += 3;
		if (*laps > 10) *laps = 10;
		ERestartRace::Create();
	}
	virtual bool IsAvailable() {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps >= 10) return false;
		return GetLocalPlayerCurrentLap() <= 0;
	}
	virtual bool IsConditionallyAvailable() { return true; }
} E_Add3Laps;

class Effect_RestartRace : public ChaosEffect {
public:
	Effect_RestartRace() : ChaosEffect() {
		sName = "Restart Race";
	}

	virtual void InitFunction() {
		ERestartRace::Create();
	}
	virtual bool IsAvailable() {
		return GRaceStatus::fObj->mPlayMode == GRaceStatus::kPlayMode_Racing;
	}
	virtual bool IsConditionallyAvailable() { return true; }
} E_RestartRace;