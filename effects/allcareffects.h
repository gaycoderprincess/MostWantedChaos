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
	bool IsRehideable() override { return true; };
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
	bool IsRehideable() override { return true; };
} E_SpinningTraffic;

class Effect_CarMagnetRandom : public EffectBase_ActiveCarsConditional {
public:
	IVehicle* pRandomCar = nullptr;

	Effect_CarMagnetRandom() : EffectBase_ActiveCarsConditional() {
		sName = "Magnet On Random Car";
		fTimerLength = 15;
	}

	void GenerateRandomCar() {
		auto cars = GetActiveVehicles();
		pRandomCar = cars[rand()%cars.size()];
	}

	void InitFunction() override {
		GenerateRandomCar();
	}
	void TickFunction(double delta) override {
		if (!IsVehicleValidAndActive(pRandomCar)) {
			GenerateRandomCar();
			return;
		}

		DoCarMagnet(pRandomCar, delta);
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; };
} E_CarMagnetRandom;

class Effect_CarForcefieldRandom : public EffectBase_ActiveCarsConditional {
public:
	IVehicle* pRandomCar = nullptr;

	Effect_CarForcefieldRandom() : EffectBase_ActiveCarsConditional() {
		sName = "Forcefield On Random Car";
		fTimerLength = 60;
	}

	void GenerateRandomCar() {
		auto cars = GetActiveVehicles();
		pRandomCar = cars[rand()%cars.size()];
	}

	void InitFunction() override {
		GenerateRandomCar();
	}
	void TickFunction(double delta) override {
		if (!IsVehicleValidAndActive(pRandomCar)) {
			GenerateRandomCar();
			return;
		}

		DoCarForcefield(pRandomCar);
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; };
} E_CarForcefieldRandom;

class Effect_CarBouncy : public ChaosEffect {
public:

	Effect_CarBouncy() : ChaosEffect() {
		sName = "Bouncy Cars";
		fTimerLength = 15;
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			auto collision = car->mCOMObject->Find<ICollisionBody>();
			auto rb = car->mCOMObject->Find<IRigidBody>();
			auto sus = car->mCOMObject->Find<ISuspension>();
			if (!collision || !rb || !sus) continue;
			if (sus->GetNumWheelsOnGround() > 0 || collision->GetNumContactPoints() > 0) {
				auto vel = *rb->GetLinearVelocity();
				vel.y = 2;
				rb->SetLinearVelocity(&vel);
			}
		}
	}
	bool HasTimer() override { return true; }
} E_CarBouncy;

class Effect_CarTumbly : public ChaosEffect {
public:

	Effect_CarTumbly() : ChaosEffect() {
		sName = "Tumbling Cars";
		fTimerLength = 15;
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			auto rb = car->mCOMObject->Find<IRigidBody>();
			auto vel = *rb->GetAngularVelocity();
			UMath::Vector3 right;
			rb->GetForwardVector(&right);
			vel.x += 25 * delta * right.x;
			vel.y += 25 * delta * right.y;
			vel.z += 25 * delta * right.z;
			rb->SetAngularVelocity(&vel);
		}
	}
	bool HasTimer() override { return true; }
} E_CarTumbly;

class Effect_CarFlippy : public ChaosEffect {
public:

	Effect_CarFlippy() : ChaosEffect() {
		sName = "Flippy Cars";
		fTimerLength = 15;
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			auto rb = car->mCOMObject->Find<IRigidBody>();
			auto vel = *rb->GetAngularVelocity();
			UMath::Vector3 right;
			rb->GetRightVector(&right);
			vel.x += 25 * delta * right.x;
			vel.y += 25 * delta * right.y;
			vel.z += 25 * delta * right.z;
			rb->SetAngularVelocity(&vel);
		}
	}
	bool HasTimer() override { return true; }
} E_CarFlippy;

class Effect_CarFloaty : public ChaosEffect {
public:

	Effect_CarFloaty() : ChaosEffect() {
		sName = "Cars Float Away";
		fTimerLength = 15;
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			auto rb = car->mCOMObject->Find<IRigidBody>();
			if (!rb) continue;
			auto vel = *rb->GetLinearVelocity();
			vel.y = 2;
			rb->SetLinearVelocity(&vel);
		}
	}
	bool HasTimer() override { return true; }
} E_CarFloaty;

class Effect_SetTrafficMassInf : public EffectBase_ActiveCarsConditional {
public:
	Effect_SetTrafficMassInf() : EffectBase_ActiveCarsConditional() {
		sName = "Infinite Traffic Mass";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() != DRIVER_TRAFFIC) continue;
			if (auto ply = car->mCOMObject->Find<IRBVehicle>()) {
				ply->SetCollisionMass(10000);
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

class Effect_WreckOnFlip : public ChaosEffect {
public:
	Effect_WreckOnFlip() : ChaosEffect() {
		sName = "Cars Wreck When Flipped";
		fTimerLength = 120;
	}

	void TickFunction(double delta) override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_RACERS);
		for (int i = 0; i < list.size(); i++) {
			auto racer = list[i];
			auto rb = racer->mCOMObject->Find<IRigidBody>();
			auto damage = racer->mCOMObject->Find<IDamageable>();
			if (!rb || !damage) continue;
			if (damage->IsDestroyed()) continue;
			UMath::Vector3 up;
			rb->GetUpVector(&up);
			if (up.y < 0) {
				damage->Destroy();
			}
		}
	}
	bool HasTimer() override { return true; }
} E_WreckOnFlip;