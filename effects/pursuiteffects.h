/*class Effect_SpawnCop : public ChaosEffect {
public:
	Effect_SpawnCop() : ChaosEffect() {
		sName = "Spawn Cops";
	}

	void InitFunction() override {
		ICopMgr::mDisableCops = false;
		if (auto pursuit = GetLocalPlayerInterface<IPursuit>()) {
			for (int i = 0; i < 8; i++) {
				AICopManager::SpawnCopCarNow(TheOneCopManager, pursuit);
			}
		}
		else {
			for (int i = 0; i < 8; i++) {
				AICopManager::SpawnPatrolCar(TheOneCopManager);
			}
		}
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
	bool AbortOnConditionFailed() override { return true; }
} E_Pursuit;

/*class Effect_ClearCostToState : public EffectBase_PursuitConditional {
public:
	Effect_ClearCostToState() : EffectBase_PursuitConditional() {
		sName = "Reset Cost To State";
	}

	void InitFunction() override {
		if (auto pursuit = GetLocalPlayerInterface<IPursuit>()) {
			pursuit->NotifyPropertyDamaged(100000);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_ClearCostToState;

class Effect_AddCostToState : public EffectBase_PursuitConditional {
public:
	Effect_AddCostToState() : EffectBase_PursuitConditional() {
		sName = "Add 100K Cost To State";
	}

	void InitFunction() override {
		if (auto pursuit = GetLocalPlayerInterface<IPursuit>()) {
			pursuit->NotifyPropertyDamaged(100000);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
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
	bool IsRehideable() override { return true; };
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

class Effect_NoHidingSpots : public EffectBase_PursuitConditional {
public:
	Effect_NoHidingSpots() : EffectBase_PursuitConditional() {
		sName = "Disable Hiding Spots";
		fTimerLength = 120;
	}

	void InitFunction() override {
		NyaHookLib::Patch<uint8_t>(0x429949, 0xEB);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch<uint8_t>(0x429949, 0x74);
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; };
} E_NoHidingSpots;

class Effect_SetCopMassInf : public EffectBase_ActiveCopsConditional {
public:
	Effect_SetCopMassInf() : EffectBase_ActiveCopsConditional() {
		sName = "Infinite Police Mass";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AICOPS);
		for (int i = 0; i < list.size(); i++) {
			auto car = list[i];
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				ply->SetCollisionMass(10000);
			}
		}
	}
	void DeinitFunction() override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AICOPS);
		for (int i = 0; i < list.size(); i++) {
			auto car = list[i];
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				ply->SetCollisionMass(car->mCOMObject->Find<IRigidBody>()->GetMass());
			}
		}
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; };
} E_SetCopMassInf;

// todo police jenga