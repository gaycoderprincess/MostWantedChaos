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

class EffectBase_OpponentConditional : public ChaosEffect {
public:
	EffectBase_OpponentConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Opponent Conditional";
	}

	bool IsAvailable() override {
		return !VEHICLE_LIST::GetList(VEHICLE_AIRACERS).empty();
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