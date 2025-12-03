#define EFFECT_CATEGORY_TEMP "Traffic"

class Effect_RacerAITraffic : public EffectBase_ActiveTrafficConditional {
public:
	double fTimer = 0;

	Effect_RacerAITraffic() : EffectBase_ActiveTrafficConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Racer AI For Traffic";
		fTimerLength = 120;
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x4231BD + 1, "AIGoalRacer"); // AIVehicleTraffic::StartDriving
		NyaHookLib::Patch<uint16_t>(0x411E9E, 0x1BEB); // never set DriveSpeed for AIVehicleTraffic
	}
	void TickFunction(double delta) override {
		fTimer += delta;
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() != DRIVER_TRAFFIC) continue;
			car->mCOMObject->Find<IVehicleAI>()->SetDriveSpeed(100);
		}
		if (fTimer > 3) {
			fTimer -= 3;
		}
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x4231BD + 1, "AIGoalTraffic");
		NyaHookLib::Patch<uint16_t>(0x411E9E, 0x1674);
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
} E_RacerAITraffic;

class Effect_LobotomyTraffic : public EffectBase_ActiveTrafficConditional {
public:
	Effect_LobotomyTraffic() : EffectBase_ActiveTrafficConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Lobotomized Traffic";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() != DRIVER_TRAFFIC) continue;
			car->mCOMObject->Find<IVehicle>()->SetSpeed(TOMPS(200));
		}
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; };
} E_LobotomyTraffic;

class Effect_SpinningTraffic : public EffectBase_ActiveTrafficConditional {
public:
	Effect_SpinningTraffic() : EffectBase_ActiveTrafficConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Spinning Traffic";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() != DRIVER_TRAFFIC) continue;
			auto rb = car->mCOMObject->Find<IRigidBody>();
			auto vel = *rb->GetAngularVelocity();
			vel.y += 25 * delta;
			rb->SetAngularVelocity(&vel);
		}
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; };
} E_SpinningTraffic;

class Effect_SetTrafficMassInf : public EffectBase_ActiveTrafficConditional {
public:
	Effect_SetTrafficMassInf() : EffectBase_ActiveTrafficConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Infinite Traffic Mass";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() != DRIVER_TRAFFIC) continue;
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				ply->SetCollisionMass(100000);
			}
		}
	}
	void DeinitFunction() override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() != DRIVER_TRAFFIC) continue;
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				ply->SetCollisionMass(car->mCOMObject->Find<IRigidBody>()->GetMass());
			}
		}
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; };
} E_SetTrafficMassInf;

/*class Effect_HeavyTraffic : public EffectBase_NotInPursuitConditional {
public:
	Effect_HeavyTraffic() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Rush Hour Traffic";
		fTimerLength = 90;
	}

	void InitFunction() override {
		NyaHookLib::Patch<uint64_t>(0x4263DC, 0x4C8D909090909090); // AITrafficManager::GetAvailableTrafficVehicle
		NyaHookLib::Patch<uint16_t>(0x42634C, 0x9090); // AITrafficManager::NextSpawn
	}
	void DeinitFunction() override {
		NyaHookLib::Patch<uint64_t>(0x4263DC, 0x4C8D000000A9840F);
		NyaHookLib::Patch<uint16_t>(0x42634C, 0x0773);
	}
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_HeavyTraffic;*/