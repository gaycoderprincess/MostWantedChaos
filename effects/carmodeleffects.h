class Effect_OpponentsRazor : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsRazor() : EffectBase_NotInPursuitConditional() {
		sName = "New Opponents Are BMWs";
		fTimerLength = 240;
		IncompatibilityGroups.push_back(Attrib::StringHash32("opponent_car_model"));
	}

	void TickFunction(double delta) override {
		ForcedOpponentVehicle = Attrib::StringHash32("bmwm3gtre46");
		RandomizeOpponentTuning = true;
	}
	void DeinitFunction() override {
		ForcedOpponentVehicle = 0;
		RandomizeOpponentTuning = false;
	}
	bool InfiniteTimer() override { return LastOpponentVehicleSpawn != Attrib::StringHash32("bmwm3gtre46"); }
	bool HasTimer() override { return !InfiniteTimer(); }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsRazor;

class Effect_OpponentsGolf : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsGolf() : EffectBase_NotInPursuitConditional() {
		sName = "New Opponents Are Golfs";
		fTimerLength = 240;
		IncompatibilityGroups.push_back(Attrib::StringHash32("opponent_car_model"));
	}

	void TickFunction(double delta) override {
		ForcedOpponentVehicle = Attrib::StringHash32("gti");
		RandomizeOpponentTuning = true;
	}
	void DeinitFunction() override {
		ForcedOpponentVehicle = 0;
		RandomizeOpponentTuning = false;
	}
	bool InfiniteTimer() override { return LastOpponentVehicleSpawn != Attrib::StringHash32("gti"); }
	bool HasTimer() override { return !InfiniteTimer(); }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsGolf;

class Effect_OpponentsPlayer : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsPlayer() : EffectBase_NotInPursuitConditional() {
		sName = "New Opponents Copy Your Car";
		fTimerLength = 240;
		IncompatibilityGroups.push_back(Attrib::StringHash32("opponent_car_model"));
	}

	void TickFunction(double delta) override {
		OpponentPlayerCar = true;
	}
	void DeinitFunction() override {
		OpponentPlayerCar = false;
	}
	bool InfiniteTimer() override { return !LastOpponentPlayerCar; }
	bool HasTimer() override { return !InfiniteTimer(); }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsPlayer;

class Effect_OpponentsPlayerRandom : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsPlayerRandom() : EffectBase_NotInPursuitConditional() {
		sName = "New Opponents Steal Your Cars";
		fTimerLength = 240;
		IncompatibilityGroups.push_back(Attrib::StringHash32("opponent_car_model"));
	}

	void TickFunction(double delta) override {
		OpponentPlayerCarRandom = true;
	}
	void DeinitFunction() override {
		OpponentPlayerCarRandom = false;
	}
	bool InfiniteTimer() override { return !LastOpponentPlayerCarRandom; }
	bool HasTimer() override { return !InfiniteTimer(); }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsPlayerRandom;

class Effect_OpponentsJunkman : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsJunkman() : EffectBase_NotInPursuitConditional() {
		sName = "New Opponents Are Junkman";
		fTimerLength = 240;
		IncompatibilityGroups.push_back(Attrib::StringHash32("opponent_car_tune"));
	}

	void TickFunction(double delta) override {
		OpponentsFullyTuned = true;
	}
	void DeinitFunction() override {
		OpponentsFullyTuned = false;
	}
	bool InfiniteTimer() override { return !LastOpponentFullyTuned; }
	bool HasTimer() override { return !InfiniteTimer(); }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsJunkman;

class Effect_AllTrafficPizza : public ChaosEffect {
public:
	Effect_AllTrafficPizza() : ChaosEffect() {
		sName = "All Traffic Is TRAFPIZZA";
		fTimerLength = 240;
		IncompatibilityGroups.push_back(Attrib::StringHash32("traffic_car_model"));
	}

	void TickFunction(double delta) override {
		ForcedTrafficVehicle = Attrib::StringHash32("trafpizza");

		auto cars = GetActiveVehicles(DRIVER_TRAFFIC);
		for (auto& car : cars) {
			if (strcmp(car->GetVehicleName(), "trafpizza")) car->mCOMObject->Find<ISimable>()->Kill();
		}
	}
	void DeinitFunction() override {
		ForcedTrafficVehicle = 0;
	}
	bool HasTimer() override { return true; }
} E_AllTrafficPizza;

/*class Effect_AllTrafficTruck : public ChaosEffect {
public:
	Effect_AllTrafficTruck() : ChaosEffect() {
		sName = "All Traffic Is Trucks";
		fTimerLength = 240;
		IncompatibilityGroups.push_back(Attrib::StringHash32("traffic_car_model");
	}

	void TickFunction(double delta) override {
		ForcedTrafficVehicle = Attrib::StringHash32("semicrate");
	}
	void DeinitFunction() override {
		ForcedTrafficVehicle = 0;
	}
	bool HasTimer() override { return true; }
} E_AllTrafficTruck;*/

class Effect_PlayerCarCopCorvette : public EffectBase_TriggerInMenu {
public:
	Effect_PlayerCarCopCorvette() : EffectBase_TriggerInMenu() {
		sName = "Spawn Player As Cop Corvette";
		fTimerLength = 120;
		fUnhideTime = 0;
		IncompatibilityGroups.push_back(Attrib::StringHash32("player_car_model"));
	}

	void TickFunction(double delta) override {
		ForcedPlayerVehicle = Attrib::StringHash32("cs_c6_copsporthench");
	}
	void DeinitFunction() override {
		ForcedPlayerVehicle = 0;
	}
	bool HasTimer() override { return true; }
} E_PlayerCarCopCorvette;

class Effect_PlayerCarStockPunto : public EffectBase_TriggerInMenu {
public:
	Effect_PlayerCarStockPunto() : EffectBase_TriggerInMenu() {
		sName = "Spawn Player As Stock Punto";
		fTimerLength = 120;
		fUnhideTime = 0;
		IncompatibilityGroups.push_back(Attrib::StringHash32("player_car_model"));
	}

	void TickFunction(double delta) override {
		ForcedPlayerVehicle = Attrib::StringHash32("punto");
	}
	void DeinitFunction() override {
		ForcedPlayerVehicle = 0;
	}
	bool HasTimer() override { return true; }
} E_PlayerCarStockPunto;