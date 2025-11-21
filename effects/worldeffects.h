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