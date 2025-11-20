class Effect_OpponentsRazor : public ChaosEffect {
public:
	Effect_OpponentsRazor() : ChaosEffect() {
		sName = "All New Opponents Are BMWs";
		fTimerLength = 240;
		IncompatibilityGroup = Attrib::StringHash32("opponent_car_model");
	}

	void TickFunction(double delta) override {
		ForcedOpponentVehicle = Attrib::StringHash32("bmwm3gtre46");
		RandomizeOpponentTuning = true;
	}
	void DeinitFunction() override {
		ForcedOpponentVehicle = 0;
		RandomizeOpponentTuning = false;
	}
	bool HasTimer() override { return true; }
} E_OpponentsRazor;

class Effect_OpponentsGolf : public ChaosEffect {
public:
	Effect_OpponentsGolf() : ChaosEffect() {
		sName = "All New Opponents Are Golfs";
		fTimerLength = 240;
		IncompatibilityGroup = Attrib::StringHash32("opponent_car_model");
	}

	void TickFunction(double delta) override {
		ForcedOpponentVehicle = Attrib::StringHash32("gti");
		RandomizeOpponentTuning = true;
	}
	void DeinitFunction() override {
		ForcedOpponentVehicle = 0;
		RandomizeOpponentTuning = false;
	}
	bool HasTimer() override { return true; }
} E_OpponentsGolf;

class Effect_OpponentsPlayer : public ChaosEffect {
public:
	Effect_OpponentsPlayer() : ChaosEffect() {
		sName = "All New Opponents Copy Your Car";
		fTimerLength = 240;
		IncompatibilityGroup = Attrib::StringHash32("opponent_car_model");
	}

	void TickFunction(double delta) override {
		OpponentPlayerCar = true;
	}
	void DeinitFunction() override {
		OpponentPlayerCar = false;
	}
	bool HasTimer() override { return true; }
} E_OpponentsPlayer;

class Effect_OpponentsJunkman : public ChaosEffect {
public:
	Effect_OpponentsJunkman() : ChaosEffect() {
		sName = "All New Opponents Are Junkman";
		fTimerLength = 240;
		IncompatibilityGroup = Attrib::StringHash32("opponent_car_tune");
	}

	void TickFunction(double delta) override {
		OpponentsFullyTuned = true;
	}
	void DeinitFunction() override {
		OpponentsFullyTuned = false;
	}
	bool HasTimer() override { return true; }
} E_OpponentsJunkman;

class Effect_AllTrafficPizza : public ChaosEffect {
public:
	Effect_AllTrafficPizza() : ChaosEffect() {
		sName = "All Traffic Is TRAFPIZZA";
		fTimerLength = 240;
		IncompatibilityGroup = Attrib::StringHash32("traffic_car_model");
	}

	void TickFunction(double delta) override {
		ForcedTrafficVehicle = Attrib::StringHash32("trafpizza");
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
		IncompatibilityGroup = Attrib::StringHash32("traffic_car_model");
	}

	void TickFunction(double delta) override {
		ForcedTrafficVehicle = Attrib::StringHash32("semicrate");
	}
	void DeinitFunction() override {
		ForcedTrafficVehicle = 0;
	}
	bool HasTimer() override { return true; }
} E_AllTrafficTruck;*/

class Effect_PlayerCarTrafPizza : public ChaosEffect {
public:
	Effect_PlayerCarTrafPizza() : ChaosEffect() {
		sName = "Spawn As TRAFPIZZA";
		fTimerLength = 240;
		IncompatibilityGroup = Attrib::StringHash32("player_car_model");
	}

	void TickFunction(double delta) override {
		ForcedPlayerVehicle = Attrib::StringHash32("cs_clio_trafpizza");
	}
	void DeinitFunction() override {
		ForcedPlayerVehicle = 0;
	}
	bool HasTimer() override { return true; }

	// runs in menus to trigger the 3 second available cooldown, this way it'll only be visible once you spawn into the world again
	bool RunInMenus() override { return true; }
	bool IsAvailable() override {
		return TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND;
	}
	bool IsConditionallyAvailable() override { return true; }
} E_PlayerCarTrafPizza;