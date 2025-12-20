#define EFFECT_CATEGORY_TEMP "Car Model"

class Effect_OpponentsRazor : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsRazor() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "New Opponents Are M3 GTRs";
		fTimerLength = 240;
		AddToIncompatiblityGroup("opponent_car_model");
	}

	void TickFunctionMain(double delta) override {
		if (!HasTimer()) {
			EffectInstance->fTimer = fTimerLength;
		}
		ForcedOpponentVehicle = Attrib::StringHash32("bmwm3gtre46");
		RandomizeOpponentTuning = true;
	}
	void DeinitFunction() override {
		ForcedOpponentVehicle = 0;
		RandomizeOpponentTuning = false;
	}
	bool HasTimer() override { return LastOpponentVehicleSpawn == Attrib::StringHash32("bmwm3gtre46"); }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsRazor;

class Effect_OpponentsGolf : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsGolf() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "New Opponents Are Golfs";
		fTimerLength = 240;
		AddToIncompatiblityGroup("opponent_car_model");
	}

	void TickFunctionMain(double delta) override {
		if (!HasTimer()) {
			EffectInstance->fTimer = fTimerLength;
		}
		ForcedOpponentVehicle = Attrib::StringHash32("gti");
		RandomizeOpponentTuning = true;
	}
	void DeinitFunction() override {
		ForcedOpponentVehicle = 0;
		RandomizeOpponentTuning = false;
	}
	bool HasTimer() override { return LastOpponentVehicleSpawn == Attrib::StringHash32("gti"); }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsGolf;

class Effect_OpponentsPlayer : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsPlayer() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "New Opponents Copy Your Car";
		fTimerLength = 240;
		AddToIncompatiblityGroup("opponent_car_model");
	}

	void TickFunctionMain(double delta) override {
		if (!HasTimer()) {
			EffectInstance->fTimer = fTimerLength;
		}
		OpponentPlayerCar = true;
	}
	void DeinitFunction() override {
		OpponentPlayerCar = false;
	}
	bool HasTimer() override { return LastOpponentPlayerCar; }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsPlayer;

class Effect_OpponentsPlayerRandom : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsPlayerRandom() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "New Opponents Steal Your Cars";
		fTimerLength = 240;
		AddToIncompatiblityGroup("opponent_car_model");
	}

	void TickFunctionMain(double delta) override {
		if (!HasTimer()) {
			EffectInstance->fTimer = fTimerLength;
		}
		OpponentPlayerCarRandom = true;
	}
	void DeinitFunction() override {
		OpponentPlayerCarRandom = false;
	}
	bool HasTimer() override { return LastOpponentPlayerCarRandom; }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsPlayerRandom;

class Effect_OpponentsJunkman : public EffectBase_NotInPursuitConditional {
public:
	Effect_OpponentsJunkman() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "New Opponents Are Junkman";
		fTimerLength = 240;
		AddToIncompatiblityGroup("opponent_car_tune");
	}

	void TickFunctionMain(double delta) override {
		if (!HasTimer()) {
			EffectInstance->fTimer = fTimerLength;
		}
		OpponentsFullyTuned = true;
	}
	void DeinitFunction() override {
		OpponentsFullyTuned = false;
	}
	bool HasTimer() override { return LastOpponentFullyTuned; }
	bool AbortOnConditionFailed() override { return true; }
} E_OpponentsJunkman;

// pretty lame, also reduces traffic density
/*class Effect_AllTrafficPizza : public ChaosEffect {
public:
	Effect_AllTrafficPizza() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "All Traffic Is TRAFPIZZA";
		fTimerLength = 240;
		AddToIncompatiblityGroup("traffic_car_model");
	}

	void TickFunctionMain(double delta) override {
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
} E_AllTrafficPizza;*/

/*class Effect_AllTrafficTruck : public ChaosEffect {
public:
	Effect_AllTrafficTruck() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "All Traffic Is Trucks";
		fTimerLength = 240;
		IncompatibilityGroups.push_back(Attrib::StringHash32("traffic_car_model");
	}

	void TickFunctionMain(double delta) override {
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
		AddToIncompatiblityGroup("player_car_model");
	}

	static inline std::vector<std::string> aCopCars = {
		"COPMIDSIZE",
		"COPGHOST",
		"COPGTO",
		"COPGTOGHOST",
		"COPSPORTHENCH",
		"COPSPORTGHOST",
		"COPSPORT",
		"COPSUV",
		"COPSUVL",
	};
	std::string sCurrentReplacedCar;

	void InitFunction() override {
		sCurrentReplacedCar = aCopCars[rand()%aCopCars.size()];
	}
	void TickFunctionMain(double delta) override {
		// going back to the menu will randomize your car again
		if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND) {
			InitFunction();
		}

		auto baseVehicle = GetReplacedAICarName(sCurrentReplacedCar, true);
		if (!baseVehicle) {
			MessageBoxA(nullptr, std::format("Failed to find base model for car {}", sCurrentReplacedCar).c_str(), "nya?!~", MB_ICONERROR);
			exit(0);
		}
		ForcedPlayerVehicle = Attrib::StringHash32(baseVehicle);
		ForcedPlayerVehicleModel = sCurrentReplacedCar;
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
		AddToIncompatiblityGroup("player_car_model");
	}

	static inline std::vector<std::string> aTrafficCars = {
		"PIZZA",
		"TAXI",
		"MINIVAN",
		"PICKUPA",
		"CEMTR",
		"GARB",

		// new cars
		"TRAFAMB", // looks weird but then again the ambulance looks weird anyway
		"TRAFSUVA",
		"TRAFNEWS",
		"TRAFSTWAG", // floating a bit? im sure noone will notice
		"TRAFVANB",
		"TRAF4DSEDA",
		"TRAF4DSEDB",
		"TRAF4DSEDC",
		"TRAFCOURT",
		"TRAFFICCOUP",
		"TRAFHA",
		"TRAFDMPTR",
		"TRAFFIRE",
	};
	std::string sCurrentReplacedCar;

	void InitFunction() override {
		sCurrentReplacedCar = aTrafficCars[rand()%aTrafficCars.size()];
	}
	void TickFunctionMain(double delta) override {
		// going back to the menu will randomize your car again
		if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND) {
			InitFunction();
		}

		auto baseVehicle = GetReplacedAICarName(sCurrentReplacedCar, true);
		if (!baseVehicle) {
			MessageBoxA(nullptr, std::format("Failed to find base model for car {}", sCurrentReplacedCar).c_str(), "nya?!~", MB_ICONERROR);
			exit(0);
		}
		ForcedPlayerVehicle = Attrib::StringHash32(baseVehicle);
		ForcedPlayerVehicleModel = sCurrentReplacedCar;
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
		AddToIncompatiblityGroup("player_car_model");
	}

	void TickFunctionMain(double delta) override {
		ForcedPlayerVehicle = Attrib::StringHash32("punto");
	}
	void DeinitFunction() override {
		ForcedPlayerVehicle = 0;
	}
	bool HasTimer() override { return true; }
} E_PlayerCarStockPunto;