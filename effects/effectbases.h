class EffectBase_PlayerCarHasNitro : public ChaosEffect {
public:
	EffectBase_PlayerCarHasNitro(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Player Has Nitro";
	}

	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerEngine()) {
			return ply->HasNOS();
		}
		return false;
	}
};

class EffectBase_OpponentInRaceOrRoamingConditional : public ChaosEffect {
public:
	EffectBase_OpponentInRaceOrRoamingConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Opponent In Race Or Roaming Conditional";
	}

	bool IsAvailable() override {
		return !VEHICLE_LIST::GetList(VEHICLE_AIRACERS).empty();
	}
};

class EffectBase_OpponentInRaceConditional : public ChaosEffect {
public:
	EffectBase_OpponentInRaceConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Opponent In Race Conditional";
	}

	bool IsAvailable() override {
		return !VEHICLE_LIST::GetList(VEHICLE_AIRACERS).empty() && GRaceStatus::fObj && GRaceStatus::fObj->mRaceParms;
	}
};

class EffectBase_OpponentAliveInRaceConditional : public ChaosEffect {
public:
	EffectBase_OpponentAliveInRaceConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Opponent Alive & In Race Conditional";
	}

	bool IsAvailable() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			if (!IsCarDestroyed(car)) {
				return GRaceStatus::fObj && GRaceStatus::fObj->mRaceParms;
			}
		}
		return false;
	}
};

class EffectBase_MultiOpponentAliveInRaceConditional : public ChaosEffect {
public:
	EffectBase_MultiOpponentAliveInRaceConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Multiple Opponents Alive & In Race Conditional";
	}

	bool IsAvailable() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		int count = 0;
		for (auto& car : cars) {
			if (!IsCarDestroyed(car)) {
				count++;
			}
		}
		if (count > 1) {
			return GRaceStatus::fObj && GRaceStatus::fObj->mRaceParms;
		}
		return false;
	}
};

class EffectBase_PursuitConditional : public ChaosEffect {
public:
	EffectBase_PursuitConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Pursuit Conditional";
	}

	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			return ply->IsBeingPursued();
		}
		return false;
	}
};

class EffectBase_PursuitNoRaceConditional : public ChaosEffect {
public:
	EffectBase_PursuitNoRaceConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Pursuit No Race Conditional";
	}

	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			return ply->IsBeingPursued() && !IsInNormalRace();
		}
		return false;
	}
};

class EffectBase_NotInPursuitConditional : public ChaosEffect {
public:
	EffectBase_NotInPursuitConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Not In Pursuit Conditional";
	}

	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			return !ply->IsBeingPursued();
		}
		return false;
	}
};

class EffectBase_ActiveCarsConditional : public ChaosEffect {
public:
	EffectBase_ActiveCarsConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Active Cars Conditional";
	}

	bool IsAvailable() override {
		return GetActiveVehicles().size() > 1;
	}
};

class EffectBase_ActiveTrafficConditional : public ChaosEffect {
public:
	EffectBase_ActiveTrafficConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Active Traffic Conditional";
	}

	bool IsAvailable() override {
		return GetActiveVehicles(DRIVER_TRAFFIC).size() > 0;
	}
};

class EffectBase_ActiveCopsConditional : public ChaosEffect {
public:
	EffectBase_ActiveCopsConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Active Cops Conditional";
	}

	bool IsAvailable() override {
		return !GetActiveVehicles(DRIVER_COP).empty();
	}
};

class EffectBase_ManyActiveCopsConditional : public ChaosEffect {
public:
	EffectBase_ManyActiveCopsConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Many Active Cops Conditional";
	}

	bool IsAvailable() override {
		return GetActiveVehicles(DRIVER_COP).size() >= 3;
	}
};

class EffectBase_TriggerInMenu : public ChaosEffect {
public:
	EffectBase_TriggerInMenu(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Trigger In Menu";
	}

	// runs in menus to trigger the 3 second available cooldown, this way it'll only be visible once you spawn into the world again
	bool RunInMenus() override { return true; }
	bool IsAvailable() override {
		return TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND;
	}
};

class EffectBase_InRaceConditional : public ChaosEffect {
public:
	EffectBase_InRaceConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) In Race Conditional";
	}

	bool IsAvailable() override {
		return GRaceStatus::fObj && GRaceStatus::fObj->mRaceParms;
	}
};

class EffectBase_HasTuningConditional : public ChaosEffect {
public:
	EffectBase_HasTuningConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Has Tuning Conditional";
	}

	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			return ply->GetTunings() != nullptr;
		}
		return false;
	}
};

class EffectBase_NotInPrologueConditional : public ChaosEffect {
public:
	EffectBase_NotInPrologueConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Not In Prologue Conditional";
	}

	bool IsAvailable() override {
		return FEPlayerCarDB::GetNumCareerCars(&FEDatabase->mUserProfile->PlayersCarStable) > 0;
	}
	bool AbortOnConditionFailed() override { return true; }
};

class EffectBase_CareerConditional : public ChaosEffect {
public:
	EffectBase_CareerConditional(const char* category) : ChaosEffect(category) {
		sName = "(EFFECT BASE) Career Conditional";
	}

	bool IsAvailable() override {
		if (!IsInCareerMode()) return false;
		return true;
	}
	bool AbortOnConditionFailed() override { return true; }
};