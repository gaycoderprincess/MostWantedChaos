#define EFFECT_CATEGORY_TEMP "Pursuit"

/*class Effect_SpawnCop : public ChaosEffect {
public:
	Effect_SpawnCop() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spawn Cops";
	}

	void InitFunction() override {
		ICopMgr::mDisableCops = false;
		if (auto pursuit = GetLocalPlayerInterface<IVehicleAI>()->GetPursuit()) {
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
	void TickFunctionMain(double delta) override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AICOPS);
		for (int i = 0; i < list.size(); i++) {
			auto veh = list[i];
			if (!veh->IsActive() && !veh->IsLoading()) veh->Activate();
		}
	}
} E_SpawnCop;*/

class Effect_Pursuit : public ChaosEffect {
public:
	Effect_Pursuit() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
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

/*class Effect_ClearCostToState : public EffectBase_PursuitNoRaceConditional {
public:
	Effect_ClearCostToState() : EffectBase_PursuitNoRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Reset Cost To State";
	}

	void InitFunction() override {
		if (auto pursuit = GetLocalPlayerInterface<IVehicleAI>()->GetPursuit()) {
			pursuit->NotifyPropertyDamaged(-100000);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_ClearCostToState;*/

class Effect_AddCostToState : public EffectBase_PursuitNoRaceConditional {
public:
	Effect_AddCostToState() : EffectBase_PursuitNoRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Add 100K Cost To State";
	}

	void InitFunction() override {
		if (auto pursuit = GetLocalPlayerInterface<IVehicleAI>()->GetPursuit()) {
			pursuit->NotifyPropertyDamaged(100000);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_AddCostToState;

class Effect_AddBounty : public EffectBase_PursuitNoRaceConditional {
public:
	Effect_AddBounty() : EffectBase_PursuitNoRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Add 100K Pursuit Bounty";
	}

	void InitFunction() override {
		if (auto pursuit = GetLocalPlayerInterface<IPerpetrator>()) {
			pursuit->AddToPendingRepPointsNormal(100000);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_AddBounty;

class Effect_SubtractBounty : public EffectBase_PursuitNoRaceConditional {
public:
	Effect_SubtractBounty() : EffectBase_PursuitNoRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Subtract 100K Pursuit Bounty";
	}

	void InitFunction() override {
		if (auto pursuit = GetLocalPlayerInterface<IPerpetrator>()) {
			pursuit->AddToPendingRepPointsNormal(-100000);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_SubtractBounty;

// needs a way to force cooldown
/*class Effect_HiddenFromCops : public EffectBase_PursuitConditional {
public:
	Effect_HiddenFromCops() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
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

class Effect_NeverBusted : public EffectBase_PursuitConditional {
public:
	Effect_NeverBusted() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Never Busted";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		ICopMgr::mInstance->SetAllBustedTimersToZero();
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; };
	bool AbortOnConditionFailed() override { return true; }
} E_NeverBusted;

// todo this softlocks the game when starting a milestone or bounty event
class Effect_NoCops : public ChaosEffect {
public:
	Effect_NoCops() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Cops";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
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
	Effect_NoHidingSpots() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
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
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_NoHidingSpots;

class Effect_SetCopMassInf : public EffectBase_ActiveCopsConditional {
public:
	Effect_SetCopMassInf() : EffectBase_ActiveCopsConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Infinite Cop Mass";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AICOPS);
		for (int i = 0; i < list.size(); i++) {
			auto car = list[i];
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				ply->SetCollisionMass(100000);
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
	bool IsRehideable() override { return true; }
} E_SetCopMassInf;

class Effect_GetBusted : public ChaosEffect {
public:
	bool pass = false;

	Effect_GetBusted() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "10% Chance Of Getting Busted";
		fTimerLength = 2;
	}

	void InitFunction() override {
		if (pass = rand() % 100 < 10) {
			EffectInstance->sNameToDisplay = std::format("{} (Succeeded)", sName);

			ICopMgr::mDisableCops = false;
			ICopMgr::mInstance->PursueAtHeatLevel(GetLocalPlayerInterface<IPerpetrator>()->GetHeat());
		}
		else {
			EffectInstance->sNameToDisplay = std::format("{} (Failed)", sName);
			EffectInstance->fTimer = 15;
		}
	}
	void TickFunctionMain(double delta) override {
		if (pass) {
			static float f = -999.0;
			NyaHookLib::Patch(0x4445CC + 2, &f);
		}
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x4445CC + 2, 0x890DA4);
	}
} E_GetBusted;

class Effect_RuthlessCopSpawns : public EffectBase_PursuitConditional {
public:
	Effect_RuthlessCopSpawns() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "EVERYONE!!!";
		sFriendlyName = "Ruthless Cop Spawns";
		fTimerLength = 90;
		IncompatibilityGroups.push_back(Attrib::StringHash32("coprequest"));
	}

	static const char* __thiscall CopRequestHooked(void* pThis) {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			float heat = ply->GetHeat();
			if (heat < 2) return "copmidsize";
			if (heat < 3) return "copghost";
			if (heat < 4) return "copgto";
			if (heat < 5) return "copgtoghost";
			if (heat < 6) return "copsporthench";
			if (heat < 7) return rand() % 100 > 50 ? "copcross" : "copsportghost";
			if (heat < 8) return "copsuvpatrol";
			if (heat < 9) return "copsportghost";
			if (heat < 10) return "copmidsize";
			return "copsport";
		}
		return "copmidsize";
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x8927C0, &CopRequestHooked);
		NyaHookLib::Patch<uint16_t>(0x43EB90, 0x9090);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8927C0, 0x42BA50);
		NyaHookLib::Patch<uint16_t>(0x43EB90, 0x517D);
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_RuthlessCopSpawns;

class Effect_RuthlessCopCross : public EffectBase_PursuitConditional {
public:
	Effect_RuthlessCopCross() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "It's Cross";
		sFriendlyName = "Ruthless Cross Spawns";
		fTimerLength = 90;
		IncompatibilityGroups.push_back(Attrib::StringHash32("coprequest"));
	}

	static const char* __thiscall CopRequestHooked(void* pThis) {
		return "copcross";
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x8927C0, &CopRequestHooked);
		NyaHookLib::Patch<uint16_t>(0x43EB90, 0x9090);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8927C0, 0x42BA50);
		NyaHookLib::Patch<uint16_t>(0x43EB90, 0x517D);
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_RuthlessCopCross;

class Effect_NoCopSpawns : public EffectBase_PursuitConditional {
public:
	Effect_NoCopSpawns() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Cop Spawning";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("coprequest"));
	}

	static const char* __thiscall CopRequestHooked(void* pThis) {
		return nullptr;
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x8927C0, &CopRequestHooked);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8927C0, 0x42BA50);
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_NoCopSpawns;

/*class Effect_EnterCooldown : public EffectBase_PursuitConditional {
public:
	Effect_EnterCooldown() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Enter Pursuit Cooldown";
	}

	void TickFunctionMain(double delta) override {
		NyaHookLib::Patch<uint64_t>(0x4448EF, 0x44D9909090909090);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch<uint64_t>(0x4448EF, 0x44D90000012A850F);
	}
} E_EnterCooldown;*/

class Effect_InvincibleTires : public EffectBase_PursuitConditional {
public:
	Effect_InvincibleTires() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Invincible Player Tires";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IDamageable>()) {
			if (IsCarDestroyed(GetLocalPlayerVehicle())) return;
			ply->ResetDamage();
		}
	}
	bool HasTimer() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_InvincibleTires;

class Effect_InvincibleCops : public EffectBase_PursuitConditional {
public:
	Effect_InvincibleCops() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Invincible Cops";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles(DRIVER_COP);
		for (auto& car : cars) {
			car->mCOMObject->Find<IDamageable>()->ResetDamage();
		}

		NyaHookLib::Patch<uint8_t>(0x6B6769, 0xEB); // DamageCopCar::CheckUpright
		NyaHookLib::Patch<uint8_t>(0x42E930, 0xC3); // AICopManager::ApplyBreakerZones
	}
	void DeinitFunction() override {
		NyaHookLib::Patch<uint8_t>(0x6B6769, 0x75);
		NyaHookLib::Patch<uint8_t>(0x42E930, 0x83);
	}
	bool HasTimer() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_InvincibleCops;

class Effect_CopJenga : public EffectBase_ManyActiveCopsConditional {
public:
	Effect_CopJenga() : EffectBase_ManyActiveCopsConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Cop Jenga Tower";
		fTimerLength = 45;
	}

	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles(DRIVER_COP);
		IVehicle* bottom = nullptr;
		for (auto& car : cars) {
			if (bottom && strcmp(car->GetVehicleName(), "copheli")) {
				DoPiggyback(bottom, car);
			}
			bottom = car;
		}
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_CopJenga;

class Effect_DropCash : public EffectBase_PursuitConditional {
public:
	Effect_DropCash() : EffectBase_PursuitConditional("Uncategorized") {
		sName = "Cops Drop Cash";
		fTimerLength = 90;
	}

	struct tCarAssoc {
		IVehicle* vehicle = nullptr;
		uint32_t model;
		bool destroyed;
	};
	std::vector<tCarAssoc> lastStates;

	void CaptureAllCars() {
		lastStates.clear();
		std::vector<tCarAssoc> state;
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			state.push_back({car, car->GetVehicleKey(), IsCarDestroyed(car)});
		}
		lastStates = state;
	}

	static int GetCashRewardForCarModel(const char* model) {
		if (auto collection = Attrib::FindCollection(Attrib::StringHash32("aivehicle"), Attrib::StringHash32(model))) {
			if (auto rep = Attrib::Collection::GetData(collection, Attrib::StringHash32("RepPointsForDestroying"), 0)) {
				return *(int*)rep;
			}
		}
		return 1000;
	}

	void CheckAllCars() {
		for (auto& state : lastStates) {
			if (!IsVehicleValidAndActive(state.vehicle)) continue;
			if (state.destroyed) continue;
			if (!IsCarDestroyed(state.vehicle)) continue;

			FEDatabase->mUserProfile->TheCareerSettings.CurrentCash += GetCashRewardForCarModel(state.vehicle->GetVehicleName());
		}
	}

	void InitFunction() override {
		CaptureAllCars();
	}
	void TickFunctionMain(double delta) override {
		CheckAllCars();
		CaptureAllCars();
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
} E_DropCash;