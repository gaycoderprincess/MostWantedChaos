class EffectBase_PlayerCarHasNitro : public ChaosEffect {
public:
	EffectBase_PlayerCarHasNitro() : ChaosEffect() {
		sName = "(EFFECT BASE) Player Has Nitro";
	}

	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerEngine()) {
			return ply->HasNOS();
		}
		return false;
	}
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_OpponentInRaceOrRoamingConditional : public ChaosEffect {
public:
	EffectBase_OpponentInRaceOrRoamingConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Opponent In Race Or Roaming Conditional";
	}

	bool IsAvailable() override {
		return !VEHICLE_LIST::GetList(VEHICLE_AIRACERS).empty();
	}
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_OpponentInRaceConditional : public ChaosEffect {
public:
	EffectBase_OpponentInRaceConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Opponent In Race Conditional";
	}

	bool IsAvailable() override {
		return !VEHICLE_LIST::GetList(VEHICLE_AIRACERS).empty() && GRaceStatus::fObj && GRaceStatus::fObj->mRaceParms;
	}
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_OpponentAliveInRaceConditional : public ChaosEffect {
public:
	EffectBase_OpponentAliveInRaceConditional() : ChaosEffect() {
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
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_PursuitConditional : public ChaosEffect {
public:
	EffectBase_PursuitConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Pursuit Conditional";
	}

	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			return ply->IsBeingPursued();
		}
		return false;
	}
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_NotInPursuitConditional : public ChaosEffect {
public:
	EffectBase_NotInPursuitConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Not In Pursuit Conditional";
	}

	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			return !ply->IsBeingPursued();
		}
		return false;
	}
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_ActiveCarsConditional : public ChaosEffect {
public:
	EffectBase_ActiveCarsConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Active Cars Conditional";
	}

	bool IsAvailable() override {
		return GetActiveVehicles().size() > 1;
	}
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_ActiveTrafficConditional : public ChaosEffect {
public:
	EffectBase_ActiveTrafficConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Active Traffic Conditional";
	}

	bool IsAvailable() override {
		return GetActiveVehicles(DRIVER_TRAFFIC).size() > 0;
	}
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_ActiveCopsConditional : public ChaosEffect {
public:
	EffectBase_ActiveCopsConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Active Cops Conditional";
	}

	bool IsAvailable() override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() == DRIVER_COP) return true;
		}
		return false;
	}
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_TriggerInMenu : public ChaosEffect {
public:
	EffectBase_TriggerInMenu() : ChaosEffect() {
		sName = "(EFFECT BASE) Trigger In Menu";
	}

	// runs in menus to trigger the 3 second available cooldown, this way it'll only be visible once you spawn into the world again
	bool RunInMenus() override { return true; }
	bool IsAvailable() override {
		return TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND;
	}
	bool IsConditionallyAvailable() override { return true; }
};

class EffectBase_SafelyChangePlayerCar : public ChaosEffect {
public:
	EffectBase_SafelyChangePlayerCar() : ChaosEffect() {
		sName = "(EFFECT BASE) Safely Change Player Car";
		fUnhideTime = 3.5;
	}

	bool IsAvailable() override {
		// if there's 3 opponents, the car usually spawns with an invisible skin
		auto cars = GetActiveVehicles(DRIVER_RACER);
		return cars.size() < 3;
	}
	bool IsConditionallyAvailable() override { return true; }
};