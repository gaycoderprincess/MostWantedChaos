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

/*class Effect_SpawnCop : public ChaosEffect {
public:
	Effect_SpawnCop() : ChaosEffect() {
		sName = "Spawn Cops";
	}

	// -384.4419861, 89.61328125, 2439.11499
	// -0.9194986224 0 -0.3930932879
	// copmidsize
	// 0
	// 0

	void InitFunction() override {
		ICopMgr::mDisableCops = false;
		auto ply = GetLocalPlayerInterface<IRigidBody>();
		UMath::Vector3 pos = *ply->GetPosition();
		pos.y += 5;
		UMath::Vector3 fwd;
		ply->GetForwardVector(&fwd);
		ICopMgr::mInstance->SpawnCop(&pos, &fwd, "copghost", false, false);
		ICopMgr::mInstance->SpawnCop(&pos, &fwd, "copmidsize", false, false);
		ICopMgr::mInstance->SpawnCop(&pos, &fwd, "copgto", false, false);
		ICopMgr::mInstance->SpawnCop(&pos, &fwd, "copsporthench", false, false);
	}
	void TickFunction(double delta) override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AICOPS);
		for (int i = 0; i < list.size(); i++) {
			auto veh = list[i];
			if (!veh->IsActive() && !veh->IsLoading()) veh->Activate();
		}
	}
} E_SpawnCop;*/

class Effect_Pursuit : public ChaosEffect {
public:
	Effect_Pursuit() : ChaosEffect() {
		sName = "Trigger Police Pursuit";
	}

	void InitFunction() override {
		ICopMgr::mDisableCops = false;
		ICopMgr::mInstance->PursueAtHeatLevel(GetLocalPlayerInterface<IPerpetrator>()->GetHeat());
	}
	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			return !ply->IsBeingPursued();
		}
		return true;
	}
	bool IsConditionallyAvailable() override { return true; }
} E_Pursuit;

/*class Effect_ClearCostToState : public EffectBase_PursuitConditional {
public:
	Effect_ClearCostToState() : EffectBase_PursuitConditional() {
		sName = "Reset Cost To State";
	}

	void InitFunction() override {
		GetLocalPlayerInterface<IPerpetrator>()->SetCostToState(0);
	}
} E_ClearCostToState;

class Effect_AddCostToState : public EffectBase_PursuitConditional {
public:
	Effect_AddCostToState() : EffectBase_PursuitConditional() {
		sName = "Add 100K Cost To State";
	}

	void InitFunction() override {
		GetLocalPlayerInterface<IPerpetrator>()->SetCostToState(100000);
	}
} E_AddCostToState;*/

// needs a way to force cooldown
/*class Effect_HiddenFromCops : public EffectBase_PursuitConditional {
public:
	Effect_HiddenFromCops() : EffectBase_PursuitConditional() {
		sName = "Player Hidden From Cops";
	}

	static bool __thiscall HiddenFromCarsHooked(IPerpetrator* pThis) {
		return true;
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x8929B4, &HiddenFromCarsHooked);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8929B4, 0x433360);
	}
	bool HasTimer() override { return true; }
} E_HiddenFromCops;*/

// needs a way to force cooldown
class Effect_NeverBusted : public EffectBase_PursuitConditional {
public:
	Effect_NeverBusted() : EffectBase_PursuitConditional() {
		sName = "Never Busted";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		ICopMgr::mInstance->SetAllBustedTimersToZero();
	}
	bool HasTimer() override { return true; }
} E_NeverBusted;

// needs a way to force cooldown
class Effect_NoCops : public ChaosEffect {
public:
	Effect_NoCops() : ChaosEffect() {
		sName = "Disable Cops";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		ICopMgr::mDisableCops = true;
	}
	void DeinitFunction() override {
		ICopMgr::mDisableCops = false;
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		return !ICopMgr::mDisableCops;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_NoCops;