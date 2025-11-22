class Effect_ResetProps : public ChaosEffect {
public:
	Effect_ResetProps() : ChaosEffect() {
		sName = "Respawn All Props";
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() { EResetProps::Create(); });
	}
} E_ResetProps;

class Effect_NoTraffic : public ChaosEffect {
public:
	Effect_NoTraffic() : ChaosEffect() {
		sName = "Disable Traffic";
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() != DRIVER_TRAFFIC) continue;
			car->Deactivate();
		}
	}
	bool HasTimer() override { return true; }
} E_NoTraffic;

class Effect_SpeedSlow : public ChaosEffect {
public:
	Effect_SpeedSlow() : ChaosEffect() {
		sName = "Slow Motion";
	}

	void TickFunction(double delta) override {
		GameSpeedModifier = 0.5;
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedSlow;

class Effect_SpeedFast : public ChaosEffect {
public:
	Effect_SpeedFast() : ChaosEffect() {
		sName = "Fast Motion";
	}

	void TickFunction(double delta) override {
		GameSpeedModifier = 2;
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedFast;