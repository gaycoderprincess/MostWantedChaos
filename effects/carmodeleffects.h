class Effect_AllCarsRazor : public ChaosEffect {
public:
	Effect_AllCarsRazor() : ChaosEffect() {
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
} E_AllCarsRazor;

class Effect_AllCarsGolf : public ChaosEffect {
public:
	Effect_AllCarsGolf() : ChaosEffect() {
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
} E_AllCarsGolf;

class Effect_AllCarsPlayer : public ChaosEffect {
public:
	Effect_AllCarsPlayer() : ChaosEffect() {
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
} E_AllCarsPlayer;

class Effect_AllCarsJunkman : public ChaosEffect {
public:
	Effect_AllCarsJunkman() : ChaosEffect() {
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
} E_AllCarsJunkman;

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

class Effect_AllTrafficTruck : public ChaosEffect {
public:
	Effect_AllTrafficTruck() : ChaosEffect() {
		sName = "All Traffic Is Trucks";
		fTimerLength = 240;
		IncompatibilityGroup = Attrib::StringHash32("traffic_car_model");
	}

	void TickFunction(double delta) override {
		ForcedTrafficVehicle = Attrib::StringHash32("semi");
	}
	void DeinitFunction() override {
		ForcedTrafficVehicle = 0;
	}
	bool HasTimer() override { return true; }
} E_AllTrafficTruck;