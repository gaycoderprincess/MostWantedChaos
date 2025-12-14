#define EFFECT_CATEGORY_TEMP "Fuel"

class Effect_LeakTank : public ChaosEffect {
public:
	double tankAmount = 100;
	bool abort = false;

	static inline float TankDrainRate = 0.015;

	Effect_LeakTank() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Leaking Gas Tank";
		IncompatibilityGroups.push_back(Attrib::StringHash32("top_bar"));
	}

	void InitFunction() override {
		tankAmount = 100;
		abort = false;
	}
	void TickFunctionMain(double delta) override {
		NyaDrawing::CNyaRGBA32 rgb = {190,240,64,255};
		if (tankAmount < 15) rgb = {200,0,0,255};
		DrawTopBar(tankAmount / 100.0, rgb);

		auto playerSpeed = GetLocalPlayerVehicle()->GetSpeed();
		if (playerSpeed < 5) playerSpeed = 5;
		tankAmount -= TankDrainRate * playerSpeed * Sim::Internal::mSystem->mSpeed * delta;
		if (tankAmount <= 0) {
			if (auto ply = GetLocalPlayerInterface<IEngineDamage>()) {
				ply->Sabotage(3);
			}
			else if (auto ply = GetLocalPlayerInterface<IDamageable>()) {
				ply->Destroy();
			}
			abort = true;
		}
	}
	bool InfiniteTimer() override { return true; }
	bool ShouldAbort() override {
		return abort;
	}
} E_LeakTank;

class Effect_LeakTankRefuel : public ChaosEffect {
public:
	Effect_LeakTankRefuel() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Refuel Player Car";
	}

	void InitFunction() override {
		E_LeakTank.tankAmount = 100;
		Achievements::AwardAchievement(GetAchievement("REFUEL"));
	}
	bool IsAvailable() override {
		return IsEffectRunning(&E_LeakTank);
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_LeakTankRefuel;

class Effect_LeakTankCash : public EffectBase_NotInPrologueConditional {
public:
	float TankTimer = 0;

	static inline float TankDrainRate = 0.5;

	Effect_LeakTankCash() : EffectBase_NotInPrologueConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Realistic Fuel Prices";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		auto playerSpeed = GetLocalPlayerVehicle()->GetSpeed();
		if (playerSpeed < 5) playerSpeed = 5;
		TankTimer += playerSpeed * Sim::Internal::mSystem->mSpeed * delta;
		while (TankTimer > TankDrainRate) {
			FEDatabase->mUserProfile->TheCareerSettings.CurrentCash -= 1;
			TankTimer -= TankDrainRate;
		}
	}
	bool HasTimer() override { return true; }
} E_LeakTankCash;