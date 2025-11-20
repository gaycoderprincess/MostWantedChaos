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

class Effect_DestroyRandomCar : public EffectBase_ActiveCarsConditional {
public:
	Effect_DestroyRandomCar() : EffectBase_ActiveCarsConditional() {
		sName = "Destroy Random Car";
	}
	
	void InitFunction() override {
		auto cars = GetActiveVehicles();
		int i = rand() % cars.size();
		auto racer = cars[i];
		auto damage = racer->mCOMObject->Find<IDamageable>();
		if (!damage) return;
		damage->Destroy();
	}
} E_DestroyRandomCar;

class Effect_RacerAITraffic : public ChaosEffect {
public:
	double fTimer = 0;

	Effect_RacerAITraffic() : ChaosEffect() {
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
} E_RacerAITraffic;

class Effect_LobotomyTraffic : public EffectBase_ActiveCarsConditional {
public:
	Effect_LobotomyTraffic() : EffectBase_ActiveCarsConditional() {
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
} E_LobotomyTraffic;

class Effect_SpinningTraffic : public EffectBase_ActiveCarsConditional {
public:
	Effect_SpinningTraffic() : EffectBase_ActiveCarsConditional() {
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
} E_SpinningTraffic;