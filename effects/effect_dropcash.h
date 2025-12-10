class Effect_DropCash : public EffectBase_PursuitConditional {
public:
	Effect_DropCash() : EffectBase_PursuitConditional("Uncategorized") {
		sName = "Cops Drop Cash";
		fTimerLength = 90;
	}

	struct tCarAssoc {
		IVehicle* vehicle = nullptr;
		uint32_t model;
		bool destroyed;
	};
	std::vector<tCarAssoc> lastStates;

	void CaptureAllCars() {
		lastStates.clear();
		std::vector<tCarAssoc> state;
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			state.push_back({car, car->GetVehicleKey(), IsCarDestroyed(car)});
		}
		lastStates = state;
	}

	static int GetCashRewardForCarModel(const char* model) {
		if (auto collection = Attrib::FindCollection(Attrib::StringHash32("aivehicle"), Attrib::StringHash32(model))) {
			if (auto rep = Attrib::Collection::GetData(collection, Attrib::StringHash32("RepPointsForDestroying"), 0)) {
				return *(int*)rep;
			}
		}
		return 1000;
	}

	void CheckAllCars() {
		for (auto& state : lastStates) {
			if (!IsVehicleValidAndActive(state.vehicle)) continue;
			if (state.destroyed) continue;
			if (!IsCarDestroyed(state.vehicle)) continue;

			FEDatabase->mUserProfile->TheCareerSettings.CurrentCash += GetCashRewardForCarModel(state.vehicle->GetVehicleName());
		}
	}

	void InitFunction() override {
		CaptureAllCars();
	}
	void TickFunction(double delta) override {
		CheckAllCars();
		CaptureAllCars();
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
} E_DropCash;