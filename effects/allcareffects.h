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
		if (racer == GetLocalPlayerVehicle()) {
			Achievements::AwardAchievement(GetAchievement("DESTROY_RANDOM_PLAYER"));
		}
		damage->Destroy();
	}
} E_DestroyRandomCar;

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
				// todo incorporate GetGroundNormal somehow

				auto vel = *rb->GetLinearVelocity();
				vel.y = 3;
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

	void InitFunction() override {
		NoResetCount++;
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
	void DeinitFunction() override {
		NoResetCount--;
	}
	bool HasTimer() override { return true; }
} E_CarFloaty;

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
			if (IsCarDestroyed(racer)) continue;
			UMath::Vector3 up;
			rb->GetUpVector(&up);
			if (up.y < 0) {
				damage->Destroy();
			}
		}
	}
	bool HasTimer() override { return true; }
} E_WreckOnFlip;

class Effect_FreezeEveryone : public EffectBase_ActiveCarsConditional {
public:
	struct tCarAssoc {
		IVehicle* vehicle = nullptr;
		uint32_t model;
		CwoeeCarPhysicalState state;
		CwoeeCarMiscState miscState;
	};
	std::vector<tCarAssoc> lastStates;
	double timer = 0;

	Effect_FreezeEveryone() : EffectBase_ActiveCarsConditional() {
		sName = "DDOS The Server";
		sFriendlyName = "Lag For All Cars";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("car_lag"));
	}

	void CaptureAllCars() {
		lastStates.clear();

		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			lastStates.push_back({car, car->GetVehicleKey(), CwoeeCarPhysicalState(car), CwoeeCarMiscState(car)});
		}
	}

	void ApplyAllCars() {
		for (auto& car : lastStates) {
			//if (car.vehicle == GetLocalPlayerVehicle()) continue;
			if (!IsVehicleValidAndActive(car.vehicle)) continue;
			if (car.vehicle->GetVehicleKey() != car.model) continue;
			car.state.Apply(car.vehicle);
			car.miscState.Apply(car.vehicle, false);
		}
	}

	void InitFunction() override {
		timer = 0;
		CaptureAllCars();
	}
	void TickFunction(double delta) override {
		timer += delta;
		if (timer > 1) {
			int applyChance = 40;
			int captureChance = 60;
			if (rand() % 100 < applyChance) {
				ApplyAllCars();
			}
			else if (rand() % 100 < captureChance) {
				CaptureAllCars();
			}
			timer -= 1;
		}
	}
	void DeinitFunction() override {
		lastStates.clear();
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
} E_FreezeEveryone;

class Effect_FreezeEveryoneNoPlayer : public EffectBase_ActiveCarsConditional {
public:
	struct tCarAssoc {
		IVehicle* vehicle = nullptr;
		uint32_t model;
		CwoeeCarPhysicalState state;
		CwoeeCarMiscState miscState;
	};
	std::vector<tCarAssoc> lastStates;
	double timer = 0;

	Effect_FreezeEveryoneNoPlayer() : EffectBase_ActiveCarsConditional() {
		sName = "Lag For All Other Cars";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("car_lag"));
	}

	void CaptureAllCars() {
		lastStates.clear();

		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			lastStates.push_back({car, car->GetVehicleKey(), CwoeeCarPhysicalState(car), CwoeeCarMiscState(car)});
		}
	}

	void ApplyAllCars() {
		for (auto& car : lastStates) {
			if (car.vehicle == GetLocalPlayerVehicle()) continue;
			if (!IsVehicleValidAndActive(car.vehicle)) continue;
			if (car.vehicle->GetVehicleKey() != car.model) continue;
			car.state.Apply(car.vehicle);
			car.miscState.Apply(car.vehicle, false);
		}
	}

	void InitFunction() override {
		timer = 0;
		CaptureAllCars();
	}
	void TickFunction(double delta) override {
		timer += delta;
		if (timer > 1) {
			int applyChance = 25;
			int captureChance = 50;
			if (!GRaceStatus::fObj->mRaceParms && GetLocalPlayerInterface<IPerpetrator>()->IsBeingPursued()) {
				applyChance = 40;
				captureChance = 60;

				// this effect makes cooldowns essentially free, disable during cooldowns
				// todo how do i get the player's IPursuit?
				//if (ICopMgr::mInstance->IsPlayerPursuitActive() && GetLocalPlayerInterface<IPursuit>()->GetPursuitStatus() == PS_COOL_DOWN) {
				//	return;
				//}
			}

			if (rand() % 100 < applyChance) {
				ApplyAllCars();
			}
			else if (rand() % 100 < captureChance) {
				CaptureAllCars();
			}
			timer -= 1;
		}
	}
	void DeinitFunction() override {
		lastStates.clear();
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
} E_FreezeEveryoneNoPlayer;

class Effect_SnakeCars : public EffectBase_ActiveCarsConditional {
public:
	std::vector<CwoeeCarPhysicalState> lastStates;
	double timer = 0;

	Effect_SnakeCars() : EffectBase_ActiveCarsConditional() {
		sName = "Snake";
		sFriendlyName = "All Cars Follow Player";
		fTimerLength = 45;
		IncompatibilityGroups.push_back(Attrib::StringHash32("car_lag"));
	}

	void ApplyAllCars() {
		if (lastStates.empty()) return;

		auto cars = GetActiveVehicles();
		int count = 0;
		for (auto& car : cars) {
			if (car == GetLocalPlayerVehicle()) continue;

			if (auto col = car->mCOMObject->Find<IRBVehicle>()) {
				col->EnableObjectCollisions(false);
			}
			int i = lastStates.size() - 1 - (++count * 10);
			if (i < 0) break;
			lastStates[i].Apply(car);
		}
	}

	void InitFunction() override {
		timer = 0;
		lastStates.clear();
	}
	void TickFunction(double delta) override {
		timer += delta;
		if (timer > 1.0 / 60.0) {
			lastStates.push_back(CwoeeCarPhysicalState(GetLocalPlayerVehicle()));
			timer -= 1.0 / 60.0;
		}
		ApplyAllCars();
	}
	void DeinitFunction() override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car == GetLocalPlayerVehicle()) continue;
			if (auto col = car->mCOMObject->Find<IRBVehicle>()) {
				col->EnableObjectCollisions(true);
			}
		}
		lastStates.clear();
	}
	bool HasTimer() override { return true; }
} E_SnakeCars;

class Effect_BouncyMod : public EffectBase_ActiveCarsConditional {
public:
	Effect_BouncyMod() : EffectBase_ActiveCarsConditional() {
		sName = "Bouncy Mod";
		fTimerLength = 90;
	}

	static inline const char* collections[] = {
			"cars",
			"racers",
			"cops",
			"tractors",
			"traffic",
			"trailers",
	};

	void TickFunction(double delta) override {
		for (auto& name : collections) {
			auto collection = Attrib::FindCollection(Attrib::StringHash32("rigidbodyspecs"), Attrib::StringHash32(name));
			if (!collection) continue;
			auto obj = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("OBJ_ELASTICITY"), 0);
			auto wall = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("WALL_ELASTICITY"), 0);
			// THE NUMBERS MASON WHAT DO THEY MEAN????
			obj[0] = 4;
			obj[1] = 4;
			obj[2] = 4;
			obj[3] = 1;
			wall[0] = 2;
			wall[1] = 2;
			wall[2] = 2;
			wall[3] = 1;
		}
	}
	void DeinitFunction() override {
		for (auto& name : collections) {
			auto collection = Attrib::FindCollection(Attrib::StringHash32("rigidbodyspecs"), Attrib::StringHash32(name));
			if (!collection) continue;
			auto obj = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("OBJ_ELASTICITY"), 0);
			auto wall = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("WALL_ELASTICITY"), 0);
			obj[0] = 0;
			obj[1] = 0;
			obj[2] = 0.05;
			obj[3] = 0;
			wall[0] = 0;
			wall[1] = 0;
			wall[2] = 0;
			wall[3] = 0;
		}
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
} E_BouncyMod;

class Effect_WideCars : public ChaosEffect {
public:
	Effect_WideCars() : ChaosEffect() {
		sName = "Wide Cars";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("car_scale"));
	}

	void TickFunction(double delta) override {
		CarScaleMatrix = UMath::Matrix4::kIdentity;

		auto fadeIn = GetEffectFadeInOut(this, 1, true);
		CarScaleMatrix._v1.x *= 1 + (fadeIn * 3);
		CarScaleMatrix._v1.y *= 1 + (fadeIn * 3);
		CarScaleMatrix._v1.z *= 1 + (fadeIn * 3);
	}
	void DeinitFunction() override {
		CarScaleMatrix = UMath::Matrix4::kIdentity;
	}
	bool HasTimer() override { return true; }
} E_WideCars;

class Effect_GroovyCars : public ChaosEffect {
public:
	double state = 0;
	bool goBack = false;

	static inline float GroovySpeed = 4.5;

	Effect_GroovyCars() : ChaosEffect() {
		sName = "Why's This Dealer?";
		sFriendlyName = "Groovy Cars";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("car_scale"));
	}

	NyaAudio::NyaSound sound = 0;

	void InitFunction() override {
		state = 0;

		if (!sound) sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/dealer.mp3");
	}
	void TickFunction(double delta) override {
		state += delta * (goBack ? -GroovySpeed : GroovySpeed);
		if (state > 1) goBack = true;
		if (state < 0) goBack = false;

		CarScaleMatrix = UMath::Matrix4::kIdentity;

		CarScaleMatrix._v0.x *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix._v0.y *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix._v0.z *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix._v1.x *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix._v1.y *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix._v1.z *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix._v2.x *= 1 + (easeInOutQuart(state) * 0.33);
		CarScaleMatrix._v2.y *= 1 + (easeInOutQuart(state) * 0.33);
		CarScaleMatrix._v2.z *= 1 + (easeInOutQuart(state) * 0.33);

		if (sound && NyaAudio::IsFinishedPlaying(sound)) {
			NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
			NyaAudio::Play(sound);
		}
	}
	void DeinitFunction() override {
		CarScaleMatrix = UMath::Matrix4::kIdentity;

		if (sound && !NyaAudio::IsFinishedPlaying(sound)) {
			NyaAudio::Stop(sound);
		}
	}
	bool HasTimer() override { return true; }
} E_GroovyCars;

class Effect_TeleportAllCars : public EffectBase_ActiveCarsConditional {
public:
	Effect_TeleportAllCars() : EffectBase_ActiveCarsConditional() {
		sName = "Teleport All Cars To Player";
	}

	void InitFunction() override {
		auto pos = *GetLocalPlayerVehicle()->GetPosition();
		auto fwd = *GetLocalPlayerInterface<ICollisionBody>()->GetForwardVector();
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			//if (car == GetLocalPlayerVehicle()) continue;
			car->SetVehicleOnGround(&pos, &fwd);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_TeleportAllCars;

class Effect_LaggyLookingCars : public ChaosEffect {
public:
	Effect_LaggyLookingCars() : ChaosEffect() {
		sName = "Snap To Grid";
		sFriendlyName = "Shaky Cars";
		fTimerLength = 60;
	}

	void InitFunction() override {
		CarRender_Truncate = true;
		CarRender_TruncateRotation = true;
		CarRender_TruncateRotationAccuracy = 10;
	}
	void DeinitFunction() override {
		CarRender_Truncate = false;
		CarRender_TruncateRotation = false;
	}
	bool HasTimer() override { return true; }
} E_LaggyLookingCars;

class Effect_LaggyLookingCars2 : public ChaosEffect {
public:
	Effect_LaggyLookingCars2() : ChaosEffect() {
		sName = "Low Quality Movement";
		fTimerLength = 60;
	}

	void InitFunction() override {
		CarRender_TruncateRotation = true;
		CarRender_TruncateRotationAccuracy = 5;
	}
	void DeinitFunction() override {
		CarRender_TruncateRotation = false;
	}
	bool HasTimer() override { return true; }
} E_LaggyLookingCars2;