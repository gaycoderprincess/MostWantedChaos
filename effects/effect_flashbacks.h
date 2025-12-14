class Effect_Flashbacks : public ChaosEffect {
public:
	struct tCarAssoc {
		IVehicle* vehicle = nullptr;
		uint32_t model;
		CwoeeCarPhysicalState state;
		CwoeeCarMiscState miscState;
	};
	std::vector<std::vector<tCarAssoc>> lastStates;
	double timer = 0;
	int rewindTarget = -1;

	const float fCaptureInterval = 1.0 / 60.0;
	const float fRewindTime = 2 / fCaptureInterval;

	Effect_Flashbacks() : ChaosEffect("Uncategorized") {
		sName = "Forza Rewind";
		fTimerLength = 240;
		IncompatibilityGroups.push_back(Attrib::StringHash32("car_lag"));
	}

	void CaptureAllCars() {
		std::vector<tCarAssoc> states;
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			states.push_back({car, car->GetVehicleKey(), CwoeeCarPhysicalState(car), CwoeeCarMiscState(car)});
		}
		lastStates.push_back(states);
	}

	void ApplyAllCars(int id) {
		if (id < 0) id = 0;
		if (id > lastStates.size()) id = lastStates.size()-1;
		for (auto& car : lastStates[id]) {
			//if (car.vehicle == GetLocalPlayerVehicle()) continue;
			if (!IsVehicleValidAndActive(car.vehicle)) continue;
			if (car.vehicle->GetVehicleKey() != car.model) continue;
			car.state.Apply(car.vehicle);
			car.miscState.Apply(car.vehicle, true);
		}
		while (lastStates.size() >= id) {
			lastStates.pop_back();
		}
	}

	void InitFunction() override {
		timer = 0;
		lastStates.clear();
		CaptureAllCars();
	}
	void TickFunctionMain(double delta) override {
		timer += delta;
		if (timer > fCaptureInterval) {
			timer -= fCaptureInterval;
			if (rewindTarget > 0) {
				ApplyAllCars(lastStates.size() - 1);
				if (rewindTarget >= lastStates.size()) {
					rewindTarget = -1;
				}
				return;
			}
			else {
				CaptureAllCars();
			}

			if (auto ply = GetLocalPlayerInterface<IRBVehicle>()) {
				if (ply->GetInvulnerability() == INVULNERABLE_FROM_MANUAL_RESET) {
					ply->SetInvulnerability(INVULNERABLE_NONE, 0);
					rewindTarget = lastStates.size() - fRewindTime;
				}
			}
		}
	}
	void DeinitFunction() override {
		lastStates.clear();
	}
	bool HasTimer() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_Flashbacks;