class EffectBase_ActiveCarsConditional : public ChaosEffect {
public:
	
	EffectBase_ActiveCarsConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Active Cars Conditional";
	}

	bool IsAvailable() override {
		return GetActiveVehicles().size() > 1;
	}
	bool IsConditionallyAvailable() override { return true; }
};

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