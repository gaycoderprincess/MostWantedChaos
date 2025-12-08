#define EFFECT_CATEGORY_TEMP "Car Model"

class Effect_OpponentsRazor : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsRazor() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
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
	Effect_OpponentsGolf() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
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
	Effect_OpponentsPlayer() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
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
	Effect_OpponentsPlayerRandom() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
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
	Effect_OpponentsJunkman() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
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
	Effect_AllTrafficPizza() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
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
	Effect_AllTrafficTruck() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
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
	Effect_PlayerCarCopCorvette() : EffectBase_TriggerInMenu(EFFECT_CATEGORY_TEMP) {
		sName = "Spawn Player As Random Cop Car";
		fTimerLength = 120;
		fUnhideTime = 0;
		IncompatibilityGroups.push_back(Attrib::StringHash32("player_car_model"));
	}

	struct tReplacedCar {
		std::string baseVehicle;
		std::string vehicleModel;
	};
	static inline std::vector<tReplacedCar> aCopCars = {
		{"cs_viper_copmidsize", "COPMIDSIZE"},
		{"cs_viper_copmidsize", "COPGHOST"},
		{"cs_gto_copgto", "COPGTO"},
		{"cs_gto_copgto", "COPGTOGHOST"},
		{"cs_c6_copsporthench", "COPSPORTHENCH"},
		{"cs_c6_copsporthench", "COPSPORTGHOST"},
		{"cs_c6_copsporthench", "COPSPORT"},
		{"cs_mustang_copsuv", "COPSUV"},
		{"cs_mustang_copsuv", "COPSUVL"},
	};
	tReplacedCar* pCurrentReplacedCar = nullptr;

	void InitFunction() override {
		pCurrentReplacedCar = &aCopCars[rand()%aCopCars.size()];
	}
	void TickFunction(double delta) override {
		ForcedPlayerVehicle = Attrib::StringHash32(pCurrentReplacedCar->baseVehicle.c_str());
		ForcedPlayerVehicleModel = pCurrentReplacedCar->vehicleModel;
	}
	void DeinitFunction() override {
		ForcedPlayerVehicle = 0;
		ForcedPlayerVehicleModel = "";
	}
	bool HasTimer() override { return true; }
} E_PlayerCarCopCorvette;

class Effect_PlayerCarTraffic : public EffectBase_TriggerInMenu {
public:
	Effect_PlayerCarTraffic() : EffectBase_TriggerInMenu(EFFECT_CATEGORY_TEMP) {
		sName = "Spawn Player As Random Traffic Car";
		fTimerLength = 120;
		fUnhideTime = 0;
		IncompatibilityGroups.push_back(Attrib::StringHash32("player_car_model"));
	}

	struct tReplacedCar {
		std::string baseVehicle;
		std::string vehicleModel;
	};
	static inline std::vector<tReplacedCar> aTrafficCars = {
		{"cs_clio_trafpizza", "PIZZA"},
		{"cs_clio_traftaxi", "TAXI"},
		{"cs_cts_traf_minivan", "MINIVAN"},
		{"cs_cts_traffictruck", "PICKUPA"},
		{"cs_trafcement", "CEMTR"},
		{"cs_trafgarb", "GARB"},

		// new cars
		{"cs_cts_traffictruck", "TRAFAMB"}, // looks weird but then again the ambulance looks weird anyway
		{"cs_cts_traffictruck", "TRAFSUVA"},
		{"cs_cts_traf_minivan", "TRAFNEWS"},
		{"cs_cts_traf_minivan", "TRAFSTWAG"}, // floating a bit? im sure noone will notice
		{"cs_cts_traf_minivan", "TRAFVANB"},
		{"cs_clio_trafpizza", "TRAF4DSEDA"},
		{"cs_clio_trafpizza", "TRAF4DSEDB"},
		{"cs_clio_trafpizza", "TRAF4DSEDC"},
		{"cs_clio_trafpizza", "TRAFCOURT"},
		{"cs_clio_trafpizza", "TRAFFICCOUP"},
		{"cs_clio_trafpizza", "TRAFHA"},
		{"cs_trafcement", "TRAFDMPTR"},
		{"cs_trafcement", "TRAFFIRE"},
	};
	tReplacedCar* pCurrentReplacedCar = nullptr;

	void InitFunction() override {
		pCurrentReplacedCar = &aTrafficCars[rand()%aTrafficCars.size()];
	}
	void TickFunction(double delta) override {
		// this should be rerollable, there are way too many bad traffic cars
		// going back to the menu will randomize your car again
		if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND) {
			InitFunction();
		}

		ForcedPlayerVehicle = Attrib::StringHash32(pCurrentReplacedCar->baseVehicle.c_str());
		ForcedPlayerVehicleModel = pCurrentReplacedCar->vehicleModel;
	}
	void DeinitFunction() override {
		ForcedPlayerVehicle = 0;
		ForcedPlayerVehicleModel = "";
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTraffic;

class Effect_PlayerCarStockPunto : public EffectBase_TriggerInMenu {
public:
	Effect_PlayerCarStockPunto() : EffectBase_TriggerInMenu(EFFECT_CATEGORY_TEMP) {
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