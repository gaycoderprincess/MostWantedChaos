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
		fTimerLength = 60;
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
		fTimerLength = 30;
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
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		GameSpeedModifier = 2;
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedFast;

class Effect_SpeedSpeedBased : public ChaosEffect {
public:
	Effect_SpeedSpeedBased() : ChaosEffect() {
		sName = "Reverse Superhot";
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		auto speed = GetLocalPlayerVehicle()->GetSpeed();
		if (speed < TOMPS(10)) {
			GameSpeedModifier = TOMPS(150) / TOMPS(10);
		}
		else {
			GameSpeedModifier = TOMPS(150) / speed;
		}
		if (GameSpeedModifier > 3) GameSpeedModifier = 3;
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedSpeedBased;

class Effect_InvisibleWorld : public ChaosEffect {
public:
	Effect_InvisibleWorld() : ChaosEffect() {
		sName = "Invisible World";
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		NyaHookLib::Patch(0x723FA0, 0x530008C2);
		NyaHookLib::Patch<uint8_t>(0x6DEF49, 0xEB); // FE
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x723FA0, 0x5314EC83);
		NyaHookLib::Patch<uint8_t>(0x6DEF49, 0x74);
	}
	bool HasTimer() override { return true; }
} E_InvisibleWorld;

// todo change ColourBloomTint in visuallook for a rainbow world thing?