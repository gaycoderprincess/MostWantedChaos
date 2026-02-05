#define EFFECT_CATEGORY_TEMP "Opponents"

// same effect achieved by puncturing their tires
/*class Effect_BlowEngineOpponents : public EffectBase_MultiOpponentAliveInRaceConditional {
public:
	Effect_BlowEngineOpponents() : EffectBase_MultiOpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Blow Opponents' Engines";
	}

	void InitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& racer : cars) {
			auto engine = racer->mCOMObject->Find<IEngineDamage>();
			if (!engine) continue;
			engine->Sabotage(1);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_BlowEngineOpponents;*/

class Effect_BlowEngineOpponents1 : public EffectBase_OpponentAliveInRaceConditional {
public:
	Effect_BlowEngineOpponents1() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Blow One Opponent's Engine";
		bAbortOnConditionFailed = true;
	}

	void InitFunction() override {
		auto list = GetActiveVehicles(DRIVER_RACER);
		int i = rand() % list.size();
		auto racer = list[i];
		auto engine = racer->mCOMObject->Find<IEngineDamage>();
		if (!engine) return;
		engine->Sabotage(1);
	}
} E_BlowEngineOpponents1;

// todo this doesnt do anything...?
/*class Effect_LaunchCarFwdOpponents : public EffectBase_OpponentInRaceConditional {
public:
	Effect_LaunchCarFwdOpponents() : EffectBase_OpponentInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Launch Opponents Forward";
	}

	void InitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& racer : cars) {
			racer->SetSpeed(TOMPS(400));
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_LaunchCarFwdOpponents;*/

class Effect_LaunchCarBwdOpponents : public EffectBase_OpponentInRaceConditional {
public:
	Effect_LaunchCarBwdOpponents() : EffectBase_OpponentInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Launch Opponents Backwards";
		bAbortOnConditionFailed = true;
	}

	void InitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& racer : cars) {
			racer->SetSpeed(TOMPS(-200));
		}
	}
} E_LaunchCarBwdOpponents;

class Effect_LaunchCarSideOpponents : public EffectBase_OpponentInRaceConditional {
public:
	Effect_LaunchCarSideOpponents() : EffectBase_OpponentInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Launch Opponents Sideways";
		bAbortOnConditionFailed = true;
	}

	void InitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& racer : cars) {
			auto ply = racer->mCOMObject->Find<IRigidBody>();
			UMath::Vector3 side;
			ply->GetRightVector(&side);
			side.x *= TOMPS(200);
			side.y *= TOMPS(200);
			side.z *= TOMPS(200);
			ply->SetLinearVelocity(&side);
		}
	}
} E_LaunchCarSideOpponents;

class Effect_LaunchCarUpOpponents : public EffectBase_OpponentInRaceConditional {
public:
	Effect_LaunchCarUpOpponents() : EffectBase_OpponentInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Launch Opponents Up";
		bAbortOnConditionFailed = true;
	}

	void InitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& racer : cars) {
			auto ply = racer->mCOMObject->Find<IRigidBody>();
			if (!ply) continue;
			UMath::Vector3 vel = *ply->GetLinearVelocity();
			vel.y = TOMPS(200);
			ply->SetLinearVelocity(&vel);
		}
	}
} E_LaunchCarUpOpponents;

class Effect_SpinningOpponents : public EffectBase_OpponentInRaceConditional {
public:
	Effect_SpinningOpponents() : EffectBase_OpponentInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Spinning Opponents";
		fTimerLength = 30;
		bAbortOnConditionFailed = true;
	}

	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			auto rb = car->mCOMObject->Find<IRigidBody>();
			auto vel = *rb->GetAngularVelocity();
			vel.y += 25 * delta;
			rb->SetAngularVelocity(&vel);
		}
	}
	bool HasTimer() override { return true; }
} E_SpinningOpponents;

class Effect_RubberbandOpponents : public EffectBase_OpponentAliveInRaceConditional {
public:
	Effect_RubberbandOpponents() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "FUCKING RUBBERBAND";
		sFriendlyName = "Opponents Rubberband";
		fTimerLength = 60;
		AddToIncompatiblityGroup("rubberband");
		bAbortOnConditionFailed = true;
	}

	static float __thiscall GetCatchupCheatHooked(ICheater* pThis) {
		return 50;
	}

	static bool __thiscall GetPerformanceHooked(IVehicle* pThis, Physics::Info::Performance* out) {
		out->Acceleration = 1.0;
		out->Handling = 1.0;
		out->TopSpeed = 1.0;
		return true;
	}

	static float __thiscall GetTopSpeedHooked(IVehicle* pThis) {
		return 1000;
	}

	static float __thiscall GetAccelerationHooked(IVehicle* pThis, float a2) {
		return 1000;
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x8925C8, &GetCatchupCheatHooked);
		NyaHookLib::Patch(0x8AA8FC, &GetPerformanceHooked);
		NyaHookLib::Patch(0x892A64, &GetTopSpeedHooked);
		NyaHookLib::Patch(0x892A68, &GetAccelerationHooked);
	}
	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			car->mCOMObject->Find<IVehicleAI>()->SetDriveSpeed(TOMPS(400));
		}
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8925C8, 0x409390);
		NyaHookLib::Patch(0x8AA8FC, 0x688270);
		NyaHookLib::Patch(0x892A64, 0x431D60);
		NyaHookLib::Patch(0x892A68, 0x4223E0);
	}
	bool HasTimer() override { return true; }
} E_RubberbandOpponents;

class Effect_NoRubberbandOpponents : public EffectBase_OpponentAliveInRaceConditional {
public:
	Effect_NoRubberbandOpponents() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Rubberbanding";
		fTimerLength = 90;
		AddToIncompatiblityGroup("rubberband");
		bAbortOnConditionFailed = true;
	}

	static float __thiscall GetCatchupCheatHooked(ICheater* pThis) {
		return 0;
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x8925C8, &GetCatchupCheatHooked);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8925C8, 0x409390);
	}
	bool HasTimer() override { return true; }
} E_NoRubberbandOpponents;

class Effect_FairOpponents : public EffectBase_OpponentAliveInRaceConditional {
public:
	Effect_FairOpponents() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Realistic Opponent Performance";
		fTimerLength = 90;
		AddToIncompatiblityGroup("rubberband");
		bAbortOnConditionFailed = true;
	}

	static inline bool bPerfCandidateSearch  = false;
	static void OnPerfCandidateSearch() {
		bPerfCandidateSearch = true;
	}

	static inline uintptr_t PerformanceCandidatesHookedASM_jmp = 0x60D580;
	static void __attribute__((naked)) PerformanceCandidatesHookedASM() {
		__asm__ (
			"pushad\n\t"
			"call %1\n\t"
			"popad\n\t"
			"jmp %0\n\t"
				:
				:  "m" (PerformanceCandidatesHookedASM_jmp), "i" (OnPerfCandidateSearch)
		);
	}

	static float __thiscall GetCatchupCheatHooked(ICheater* pThis) {
		return 0;
	}

	static bool __thiscall GetPerformanceHooked(uint32_t pThis, Physics::Info::Performance* out) {
		if (bPerfCandidateSearch) {
			bPerfCandidateSearch = false;
			out->TopSpeed = *(float*)(pThis + 0xEC);
			out->Handling = *(float*)(pThis + 0xF0);
			out->Acceleration = *(float*)(pThis + 0xF4);
			return *(bool*)(pThis + 0xF8);
		}
		else {
			out->Acceleration = 1.0;
			out->Handling = 1.0;
			out->TopSpeed = 1.0;
		}
		return true;
	}

	static float __thiscall GetTopSpeedHooked(IVehicle* pThis) {
		return 1000;
	}

	static float __thiscall GetAccelerationHooked(IVehicle* pThis, float a2) {
		return 1000;
	}

	void InitFunction() override {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x61DDE3, &PerformanceCandidatesHookedASM);
		NyaHookLib::Patch(0x8925C8, &GetCatchupCheatHooked);
		NyaHookLib::Patch(0x8AA8FC, &GetPerformanceHooked);
		NyaHookLib::Patch(0x892A64, &GetTopSpeedHooked);
		NyaHookLib::Patch(0x892A68, &GetAccelerationHooked);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8925C8, 0x409390);
		NyaHookLib::Patch(0x8AA8FC, 0x688270);
		NyaHookLib::Patch(0x892A64, 0x431D60);
		NyaHookLib::Patch(0x892A68, 0x4223E0);
	}
	bool HasTimer() override { return true; }
} E_FairOpponents;

class Effect_SpikeAllOpponents : public EffectBase_OpponentAliveInRaceConditional {
public:
	Effect_SpikeAllOpponents() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Puncture Opponents' Tires";
		bAbortOnConditionFailed = true;
	}

	void InitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			auto ply = car->mCOMObject->Find<ISpikeable>();
			ply->Puncture(0);
			ply->Puncture(1);
			ply->Puncture(2);
			ply->Puncture(3);
		}
	}
} E_SpikeAllOpponents;

/*class Effect_SendOffAllOpponents : public EffectBase_OpponentAliveInRaceConditional {
public:
	Effect_SendOffAllOpponents() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Make All Opponents Leave";
		bAbortOnConditionFailed = true;
	}

	void InitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			auto ply = car->mCOMObject->Find<IRacer>();
			ply->QuitRace();
		}
	}
} E_SendOffAllOpponents;*/

class Effect_SetOpponentMass0 : public EffectBase_OpponentAliveInRaceConditional {
public:
	Effect_SetOpponentMass0() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Zero Opponent Mass";
		fTimerLength = 60;
		bAbortOnConditionFailed = true;
	}

	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				ply->SetCollisionMass(0.01);
			}
		}
	}
	void DeinitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				ply->SetCollisionMass(car->mCOMObject->Find<IRigidBody>()->GetMass());
			}
		}
	}
	bool HasTimer() override { return true; }
} E_SetOpponentMass0;

class Effect_SetOpponentPittable : public EffectBase_OpponentAliveInRaceConditional {
public:
	Effect_SetOpponentPittable() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Pittable Opponents";
		fTimerLength = 90;
		bAbortOnConditionFailed = true;
	}

	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				auto inst = Attrib::Instance(Attrib::FindCollection(Attrib::StringHash32("collisionreactions"), Attrib::StringHash32("cops")), 0, nullptr);
				ply->SetPlayerReactions(&inst);
			}
		}
	}
	void DeinitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				auto inst = Attrib::Instance(Attrib::FindCollection(Attrib::StringHash32("collisionreactions"), Attrib::StringHash32("racing")), 0, nullptr);
				ply->SetPlayerReactions(&inst);
			}
		}
	}
	bool HasTimer() override { return true; }
} E_SetOpponentPittable;

class Effect_NoNitroOpponents : public EffectBase_OpponentAliveInRaceConditional {
public:
	Effect_NoNitroOpponents() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Opponents' Nitro";
		fTimerLength = 90;
		bAbortOnConditionFailed = true;
	}

	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			car->mCOMObject->Find<IEngine>()->ChargeNOS(-1);
		}
	}
	bool HasTimer() override { return true; }
} E_NoNitroOpponents;

class Effect_LockOpponents : public EffectBase_OpponentAliveInRaceConditional {
public:
	struct tCarAssoc {
		IVehicle* vehicle = nullptr;
		uint32_t model;
		UMath::Vector3 position;
	};
	std::vector<tCarAssoc> lastStates;

	Effect_LockOpponents() : EffectBase_OpponentAliveInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Lock Opponents In Place";
		fTimerLength = 15;
		bAbortOnConditionFailed = true;
	}

	void CaptureAllCars() {
		lastStates.clear();

		auto cars = GetActiveVehicles(DRIVER_RACER);
		for (auto& car : cars) {
			lastStates.push_back({car, car->GetVehicleKey(), *car->GetPosition()});
		}
	}

	void ApplyAllCars() {
		for (auto& car : lastStates) {
			if (car.vehicle == GetLocalPlayerVehicle()) continue;
			if (!IsVehicleValidAndActive(car.vehicle)) continue;
			if (car.vehicle->GetVehicleKey() != car.model) continue;
			car.vehicle->mCOMObject->Find<IRigidBody>()->SetPosition(&car.position);
		}
	}

	void InitFunction() override {
		CaptureAllCars();
	}
	void TickFunctionMain(double delta) override {
		ApplyAllCars();
	}
	bool HasTimer() override { return true; }
} E_LockOpponents;