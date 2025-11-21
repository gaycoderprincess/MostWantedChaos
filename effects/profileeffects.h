class Effect_EnableMomentCam : public ChaosEffect {
public:
	Effect_EnableMomentCam() : ChaosEffect() {
		sName = "Enable Jump Camera";
	}

	void InitFunction() override {
		FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.JumpCam = true;
	}
	bool IsAvailable() override {
		return !FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.JumpCam;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
} E_EnableMomentCam;

class Effect_NoAutosave : public ChaosEffect {
public:
	Effect_NoAutosave() : ChaosEffect() {
		sName = "Disable Autosave";
		fTimerLength = 240;
	}

	void TickFunction(double delta) override {
		FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn = false;
	}
	void DeinitFunction() override {
		FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn = true;
	}
	bool IsAvailable() override {
		return FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
} E_NoAutosave;

class Effect_Millionaire : public ChaosEffect {
public:
	Effect_Millionaire() : ChaosEffect() {
		sName = "Millionaire";
	}

	void InitFunction() override {
		FEDatabase->mUserProfile->TheCareerSettings.CurrentCash += 1000000;
	}
} E_Millionaire;

class Effect_Millionaire2 : public ChaosEffect {
public:
	Effect_Millionaire2() : ChaosEffect() {
		sName = "Anti-Millionaire";
	}

	void InitFunction() override {
		FEDatabase->mUserProfile->TheCareerSettings.CurrentCash -= 1000000;
	}
} E_Millionaire2;

class Effect_ManualTrans : public ChaosEffect {
public:
	Effect_ManualTrans() : ChaosEffect() {
		sName = "Force Manual Transmission";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("transmission");
	}

	static bool __thiscall IsAutomaticShiftHooked(IInput* pThis) {
		return false;
	}

	void TickFunction(double delta) override {
		NyaHookLib::Patch(0x8AC6B0, &IsAutomaticShiftHooked); // normal races
		NyaHookLib::Patch(0x8AC748, &IsAutomaticShiftHooked); // drag races
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8AC6B0, 0x68D090);
		NyaHookLib::Patch(0x8AC748, 0x68D090);
	}
	bool IsAvailable() override {
		return GetLocalPlayerInterface<IInput>()->IsAutomaticShift();
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
} E_ManualTrans;

class Effect_AutoTrans : public ChaosEffect {
public:
	Effect_AutoTrans() : ChaosEffect() {
		sName = "Force Automatic Transmission";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("transmission");
	}

	static bool __thiscall IsAutomaticShiftHooked(IInput* pThis) {
		return true;
	}

	void TickFunction(double delta) override {
		NyaHookLib::Patch(0x8AC6B0, &IsAutomaticShiftHooked); // normal races
		NyaHookLib::Patch(0x8AC748, &IsAutomaticShiftHooked); // drag races
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8AC6B0, 0x68D090);
		NyaHookLib::Patch(0x8AC748, 0x68D090);
	}
	bool IsAvailable() override {
		return !GetLocalPlayerInterface<IInput>()->IsAutomaticShift();
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
} E_AutoTrans;

class Effect_PlayerCarRandomTuning : public EffectBase_TriggerInMenu {
public:
	Effect_PlayerCarRandomTuning() : EffectBase_TriggerInMenu() {
		sName = "Randomize Active Car's Visuals";
	}

	void InitFunction() override {
		auto car = GetCurrentCareerCar();
		if (!car) return;
		auto customization = FEPlayerCarDB::GetCustomizationRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->Customization);
		if (!customization) return;
		auto random = CreateRandomCustomizations(car->VehicleKey);
		memcpy(customization->InstalledPartIndices, random.InstalledPartIndices, sizeof(random.InstalledPartIndices));
		//ChangePlayerCarInWorld(car->VehicleKey, customization);
	}
	//bool IsAvailable() override {
	//	return GRaceStatus::fObj && GRaceStatus::fObj->mRaceContext == kRaceContext_Career;
	//}
	//bool IsConditionallyAvailable() override { return true; }
	//bool AbortOnConditionFailed() override { return true; }
} E_PlayerCarRandomTuning;

class Effect_PlayerCarImpoundStrike : public ChaosEffect {
public:
	Effect_PlayerCarImpoundStrike() : ChaosEffect() {
		sName = "Add A Strike To Active Career Car";
	}

	void InitFunction() override {
		auto car = GetCurrentCareerCar();
		if (!car) return;
		auto career = FEPlayerCarDB::GetCareerRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->CareerHandle);
		if (!career) return;
		career->TheImpoundData.mTimesBusted++;
	}
} E_PlayerCarImpoundStrike;

class Effect_PlayerCarImpoundStrikeRemove : public ChaosEffect {
public:
	Effect_PlayerCarImpoundStrikeRemove() : ChaosEffect() {
		sName = "Remove A Strike From Active Career Car";
	}

	void InitFunction() override {
		auto car = GetCurrentCareerCar();
		if (!car) return;
		auto career = FEPlayerCarDB::GetCareerRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->CareerHandle);
		if (!career) return;
		career->TheImpoundData.mTimesBusted++;
	}
	bool IsAvailable() override {
		auto car = GetCurrentCareerCar();
		if (!car) return false;
		auto career = FEPlayerCarDB::GetCareerRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->CareerHandle);
		if (!career) return false;
		return career->TheImpoundData.mTimesBusted > 0;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_PlayerCarImpoundStrikeRemove;

class Effect_PlayerCarImpoundMarker : public ChaosEffect {
public:
	Effect_PlayerCarImpoundMarker() : ChaosEffect() {
		sName = "Add An Extra Marker To Active Career Car";
	}

	void InitFunction() override {
		auto car = GetCurrentCareerCar();
		if (!car) return;
		auto career = FEPlayerCarDB::GetCareerRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->CareerHandle);
		if (!career) return;
		career->TheImpoundData.mMaxBusted++;
	}
} E_PlayerCarImpoundMarker;

class Effect_AddRandomTuningMarker : public ChaosEffect {
public:
	Effect_AddRandomTuningMarker() : ChaosEffect() {
		sName = "Add A Random Tuning Marker";
	}

	void InitFunction() override {
		struct tMarker {
			std::string name;
			int type;
		};
		std::vector<tMarker> markers = {
				{"Junkman Brakes", FEMarkerManager::MARKER_BRAKES},
				{"Junkman Engine", FEMarkerManager::MARKER_ENGINE},
				{"Junkman Nitrous", FEMarkerManager::MARKER_NOS},
				{"Junkman Supercharger", FEMarkerManager::MARKER_INDUCTION},
				{"Junkman Suspension", FEMarkerManager::MARKER_CHASSIS},
				{"Junkman Tires", FEMarkerManager::MARKER_TIRES},
				{"Junkman Transmission", FEMarkerManager::MARKER_TRANSMISSION},
				{"Special Body", FEMarkerManager::MARKER_BODY},
				{"Special Hood", FEMarkerManager::MARKER_HOOD},
				{"Special Spoiler", FEMarkerManager::MARKER_SPOILER},
				{"Special Rims", FEMarkerManager::MARKER_RIMS},
				{"Special Roof Scoop", FEMarkerManager::MARKER_ROOF_SCOOP},
				//{"Special Gauge", FEMarkerManager::MARKER_CUSTOM_HUD},
				//{"Special Vinyl", FEMarkerManager::MARKER_VINYL},
				//{"Special Decal", FEMarkerManager::MARKER_DECAL},
				//{"Special Paint", FEMarkerManager::MARKER_PAINT},
		};

		tMarker selectedMarker = markers[rand()%markers.size()];

		auto effectName = new char[64];
		strcpy_s(effectName, 64, std::format("{} ({})", sName, selectedMarker.name).c_str());
		EffectInstance->sNameToDisplay = effectName;
		FEMarkerManager::AddMarkerToInventory(&TheFEMarkerManager, selectedMarker.type, 0);
	}
} E_AddRandomTuningMarker;

class Effect_AddRandomBonusMarker : public ChaosEffect {
public:
	Effect_AddRandomBonusMarker() : ChaosEffect() {
		sName = "Add A Random Bonus Marker";
	}

	void InitFunction() override {
		struct tMarker {
			std::string name;
			int type;
		};
		std::vector<tMarker> markers = {
				{"Get Out of Jail Free", FEMarkerManager::MARKER_GET_OUT_OF_JAIL},
				{"Cash Reward $50000", FEMarkerManager::MARKER_CASH},
				{"Add Impound Box", FEMarkerManager::MARKER_ADD_IMPOUND_BOX},
				{"Release Car from Impound", FEMarkerManager::MARKER_IMPOUND_RELEASE},
		};

		struct tPinkSlip {
			const char* name;
			const char* preset;
			const char* carType;
		};
		tPinkSlip blacklistRides[] = {
				{"Razor", "E3_DEMO_BMW", "bmwm3gtre46"},
				{"Bull", "BL2", "slr"},
				{"Ronnie", "BL3", "db9"},
				{"JV", "BL4", "viper"},
				{"Webster", "BL5", "corvette"},
				{"Ming", "BL6", "gallardo"},
				{"Kaze", "BL7", "clk500"},
				{"Jewels", "BL8", nullptr},//"mustanggt"},
				{"Earl", "BL9", "caymans"},
				{"Baron", "BL10", ""},
				{"Big Lou", "BL11", "eclipsegt"},
				{"Izzy", "BL12", nullptr},//"rx8"},
				{"Vic", "BL13", nullptr},//"supra"},
				{"Taz", "BL14", "is300"},
				{"Sonny", "BL15", "gti"},
				{"Rog", "OPM_MUSTANG_BOSS", "gto"},
				{"Mia", "E3_DEMO_RX8", nullptr},//"rx8"}, // mia isn't actually a blacklist member, is this fine to have?
				{"Razor - Mustang", "RAZORMUSTANG", nullptr},//"mustanggt"},
				{"Ronnie - Supra", "DDAYSUPRA", nullptr},//"supra"},
		};

		std::vector<tPinkSlip> unearnedPinkSlips;
		for (auto& ride : blacklistRides) {
			if (!ride.carType || !HasPinkSlip(Attrib::StringHash32(ride.carType))) {
				unearnedPinkSlips.push_back(ride);
			}
		}

		if (!unearnedPinkSlips.empty() && FEPlayerCarDB::GetNumCareerCars(&FEDatabase->mUserProfile->PlayersCarStable) < 25) {
			markers.push_back({"Pink Slip", FEMarkerManager::MARKER_PINK_SLIP});
		}

		tMarker selectedMarker = markers[rand()%markers.size()];

		auto effectName = new char[64];
		strcpy_s(effectName, 64, std::format("{} ({})", sName, selectedMarker.name).c_str());
		EffectInstance->sNameToDisplay = effectName;
		if (selectedMarker.type == FEMarkerManager::MARKER_CASH) {
			FEDatabase->mUserProfile->TheCareerSettings.CurrentCash += 50000;
		}
		else if (selectedMarker.type == FEMarkerManager::MARKER_PINK_SLIP) {
			auto ride = unearnedPinkSlips[rand() % unearnedPinkSlips.size()];
			strcpy_s(effectName, 64, std::format("{} ({} - {})", sName, selectedMarker.name, ride.name).c_str());
			CreatePinkSlipPreset(ride.preset);
			FEPlayerCarDB::AwardRivalCar(&FEDatabase->mUserProfile->PlayersCarStable, FEngHashString(ride.preset));
		}
		else {
			FEMarkerManager::AddMarkerToInventory(&TheFEMarkerManager, selectedMarker.type, 0);
		}
	}
} E_AddRandomBonusMarker;