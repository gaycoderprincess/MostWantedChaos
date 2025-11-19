uint8_t* GetRaceNumLaps() {
	auto race = GRaceStatus::fObj;
	if (!race) return nullptr;
	if (race->mPlayMode == GRaceStatus::kPlayMode_Roaming) return nullptr;
	auto index = race->mRaceParms->mIndex;
	if (!index) {
		return (uint8_t*)Attrib::Instance::GetAttributePointer(race->mRaceParms->mRaceRecord, 0xEBDC165, 0);
	}
	return &index->mNumLaps;
}

class Effect_RemoveLapProgress : public ChaosEffect {
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
} E_RemoveLapProgress;

class Effect_RemoveLap : public ChaosEffect {
public:
	Effect_RemoveLap() : ChaosEffect() {
		sName = "Remove A Lap";
	}

	virtual void InitFunction() {
		(*GetRaceNumLaps())--;
	}
	virtual bool IsAvailable() {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps < 2) return false;
		return true;
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
	}
	virtual bool IsAvailable() {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps >= 10) return false;
		return true;
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
	}
	virtual bool IsAvailable() {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps >= 10) return false;
		return true;
	}
	virtual bool IsConditionallyAvailable() { return true; }
} E_Add3Laps;