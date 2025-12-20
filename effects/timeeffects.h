#define EFFECT_CATEGORY_TEMP "Time"

class Effect_SpeedSlow : public ChaosEffect {
public:
	Effect_SpeedSlow() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Slow Motion";
		fTimerLength = 30;
		AddToIncompatiblityGroup("gamespeed");
		MakeIncompatibleWithFilterGroup("speedbreaker");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		GameSpeedModifier = 1 - (GetEffectFadeInOut(this, 1, false) * 0.5);
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedSlow;

class Effect_SpeedFast : public ChaosEffect {
public:
	Effect_SpeedFast() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Fast Motion";
		fTimerLength = 30;
		AddToIncompatiblityGroup("gamespeed");
		MakeIncompatibleWithFilterGroup("speedbreaker");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		GameSpeedModifier = 1 + GetEffectFadeInOut(this, 1, false);
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedFast;

class Effect_SpeedSpeedBased : public ChaosEffect {
public:
	Effect_SpeedSpeedBased() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Reverse Superhot";
		fTimerLength = 30;
		AddToIncompatiblityGroup("gamespeed");
		MakeIncompatibleWithFilterGroup("speedbreaker");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

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

class Effect_RealTimeSpeedbrk : public ChaosEffect {
public:
	Effect_RealTimeSpeedbrk() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Real-Time Speedbreaker";
		fTimerLength = 90;
	}

	void TickFunctionMain(double delta) override {
		IgnoreSpeedbreakerTime = true;
	}
	void DeinitFunction() override {
		IgnoreSpeedbreakerTime = false;
	}
	bool HasTimer() override { return true; }
} E_RealTimeSpeedbrk;