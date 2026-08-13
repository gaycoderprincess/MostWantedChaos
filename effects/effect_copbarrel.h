class Effect_CopBarrel : public ChaosEffect {
public:
	Effect_CopBarrel() : ChaosEffect("Pursuit") {
		sName = "Helicopters Drop Explosive Barrels";
		fTimerLength = 60.0;
		bCanQuickTrigger = false;
		bIsRehideable = true;
		bAbortOnConditionFailed = true;
	}

	static void SpawnBarrelFromCar(IVehicle* veh) {
		auto plyPos = *GetLocalPlayerVehicle()->GetPosition();
		auto pos = *veh->GetPosition();
		if ((plyPos - pos).length() > 250) return;

		float y = 0.0;
		if (!WCollisionMgr::GetWorldHeightAtPoint(&pos, &y, nullptr)) return;

		auto rb = veh->mCOMObject->Find<IRigidBody>();
		auto vel = *rb->GetLinearVelocity();
		pos.y -= 5.0;
		Powerups::OilDrum::SpawnObject(pos, vel);
	}

	void TickFunctionMain(double delta) override {
		static double fTimer = 0.0;
		fTimer += delta;
		if (fTimer >= 4.0) {
			auto cars = GetActiveVehicles(DRIVER_COP);
			for (auto& veh : cars) {
				if (!strcmp(veh->GetVehicleName(), "copheli")) {
					SpawnBarrelFromCar(veh);
				}
			}
			fTimer -= 4.0;
		}
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		if (!IsInAnyPursuit()) return false;

		auto cars = GetActiveVehicles(DRIVER_COP);
		for (auto& veh : cars) {
			if (!strcmp(veh->GetVehicleName(), "copheli")) {
				return true;
			}
		}
		return false;
	}
} E_CopBarrel;