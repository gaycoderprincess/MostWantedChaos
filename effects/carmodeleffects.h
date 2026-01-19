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

// all opponents are already fully upgraded
/*class Effect_OpponentsJunkman : public EffectBase_NotInPursuitConditional {
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
} E_OpponentsJunkman;*/

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
		//"COPSUVL", // looks about the same as the copsuv and sinks into the ground a bit too
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

class Effect_SetCarTRAFPIZZA : public EffectBase_NoCarChangeYetConditional {
public:
	Effect_SetCarTRAFPIZZA() : EffectBase_NoCarChangeYetConditional(EFFECT_CATEGORY_TEMP) {
		sName = "It's Pizza Time";
		sFriendlyName = "Change Car To TRAFPIZZA";
		AddToFilterGroup("change_player_car");
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() {
			ChangePlayerCarInWorld(Attrib::StringHash32("cs_clio_trafpizza"), nullptr);
		});
	}
	bool CanQuickTrigger() override { return false; }
} E_SetCarTRAFPIZZA;

class Effect_SetCarRazor : public EffectBase_NoCarChangeYetConditional {
public:
	Effect_SetCarRazor() : EffectBase_NoCarChangeYetConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Change Car To Razor's Mustang";
		AddToFilterGroup("change_player_car");
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() {
			auto car = CreatePinkSlipPreset("RAZORMUSTANG");
			ChangePlayerCarInWorld(Attrib::StringHash32("mustanggt"), FEPlayerCarDB::GetCustomizationRecordByHandle(GetPlayerCarDB(), car->Customization), true);
		});
	}
	bool CanQuickTrigger() override { return false; }
} E_SetCarRazor;

class Effect_SetCarRandom : public EffectBase_NoCarChangeYetConditional {
public:
	Effect_SetCarRandom() : EffectBase_NoCarChangeYetConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Change Car To Random Model";
		AddToFilterGroup("change_player_car");
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() {
			std::vector<FECarRecord*> validCars;
			auto cars = GetPlayerCarDB();
			for (auto &car: cars->CarTable) {
				if (car.Handle == 0xFFFFFFFF) continue;
				validCars.push_back(&car);
			}
			auto car = validCars[rand() % validCars.size()];
			auto pCustomization = FEPlayerCarDB::GetCustomizationRecordByHandle(cars, car->Customization);
			auto customization = pCustomization ? *pCustomization : CreateRandomCustomizations(car->VehicleKey);
			ChangePlayerCarInWorld(car->VehicleKey, &customization, (car->FilterBits & 1) != 0); // add nitro if they're stock
		});
	}
	bool CanQuickTrigger() override { return false; }
} E_SetCarRandom;

class Effect_SetCarRandomAI : public ChaosEffect {
public:
	Effect_SetCarRandomAI() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Change Car To AI Traffic/Police Car";
		AddToFilterGroup("change_player_car");
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() {
			std::vector<const char*> carModels = {
					"copgto",
					"copgtoghost",
					"copmidsize",
					"copghost",
					//"copsport",
					"copcross",
					"copsportghost",
					"copsporthench",
					"copsuv",
					"copsuvl",
					"copsuvpatrol",
					"semi",
					"semia",
					"semib",
					"semicmt",
					"semicon",
					"semicrate",
					"semilog",
					"traf4dseda",
					"traf4dsedb",
					"traf4dsedc",
					"trafcourt",
					"trafficcoup",
					"trafha",
					"trafpizza",
					"trafstwag",
					"traftaxi",
					"trafamb",
					"trafcemtr",
					"trafdmptr",
					"traffire",
					"trafgarb",
					//"trafcamper",
					"trafminivan",
					"trafnews",
					"trafpickupa",
					"trafsuva",
					"trafvanb",
			};

			uint32_t model = Attrib::StringHash32(carModels[rand()%carModels.size()]);
			ChangePlayerCarInWorld(model, nullptr);
		});
	}
	bool CanQuickTrigger() override { return false; }
} E_SetCarRandomAI;

// this can crash sometimes?
class Effect_SetCarRandomOpponent : public EffectBase_OpponentInRaceConditional {
public:
	Effect_SetCarRandomOpponent() : EffectBase_OpponentInRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Grand Theft Auto";
		sFriendlyName = "Steal Car From Random Opponent";
		AddToFilterGroup("change_player_car");
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() {
			auto cars = GetActiveVehicles(DRIVER_RACER);
			if (cars.empty()) return;

			auto car = cars[rand()%cars.size()];
			auto carModel = car->GetVehicleName();
			auto carTuning = (FECustomizationRecord*)car->GetCustomizations();
			ChangePlayerCarInWorld(Attrib::StringHash32(carModel), carTuning, true);
		});
	}
	bool IsAvailable() override {
		if (!EffectBase_OpponentInRaceConditional::IsAvailable()) return false;
		return nNumPlayerCarChangesThisRace <= 0;
	}
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_SetCarRandomOpponent;