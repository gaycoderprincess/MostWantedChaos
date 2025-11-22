float TankDrainRate = 0.015;
class Effect_LeakTank : public ChaosEffect {
public:
	double tankAmount = 100;
	bool abort = false;

	Effect_LeakTank() : ChaosEffect() {
		sName = "Leaking Gas Tank";
	}

	void InitFunction() override {
		tankAmount = 1;
		abort = false;
	}
	void TickFunction(double delta) override {
		DrawRectangle(0, 1, 0.975, 1, {0,0,0,255});
		NyaDrawing::CNyaRGBA32 rgb = {219,100,193,255};
		if (tankAmount < 15) rgb = {200,0,0,255};
		DrawRectangle(0, tankAmount / 100.0, 0.975, 1, rgb);

		auto playerSpeed = GetLocalPlayerVehicle()->GetSpeed();
		tankAmount -= TankDrainRate * playerSpeed * delta;
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