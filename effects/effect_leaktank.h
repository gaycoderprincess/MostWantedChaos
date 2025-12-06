class Effect_LeakTank : public ChaosEffect {
public:
	double tankAmount = 100;
	bool abort = false;

	static inline float TankDrainRate = 0.015;

	Effect_LeakTank() : ChaosEffect("Uncategorized") {
		sName = "Leaking Gas Tank";
	}

	void InitFunction() override {
		tankAmount = 100;
		abort = false;
	}
	void TickFunction(double delta) override {
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
	Effect_LeakTankRefuel() : ChaosEffect("Uncategorized") {
		sName = "Refuel Player Car";
	}

	void InitFunction() override {
		E_LeakTank.tankAmount = 100;
	}
	bool IsAvailable() override {
		return IsEffectRunning(&E_LeakTank);
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_LeakTankRefuel;