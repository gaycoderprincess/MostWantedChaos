#define EFFECT_CATEGORY_TEMP "Profile"

class Effect_EnableMomentCam : public ChaosEffect {
public:
	Effect_EnableMomentCam() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Enable Game Moment Camera";
	}

	void InitFunction() override {
		FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.JumpCam = true;
	}
	bool IsAvailable() override {
		return !FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.JumpCam;
	}
	bool AbortOnConditionFailed() override { return true; }
} E_EnableMomentCam;

class Effect_NoAutosave : public ChaosEffect {
public:
	Effect_NoAutosave() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Autosave";
		fTimerLength = 240;
	}

	void TickFunctionMain(double delta) override {
		FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn = false;
	}
	void DeinitFunction() override {
		FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn = true;
	}
	bool IsAvailable() override {
		return FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn;
	}
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_NoAutosave;

class Effect_NoVisualTreatment : public ChaosEffect {
public:
	Effect_NoVisualTreatment() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Visual Treatment";
		fTimerLength = 60;
	}

	void InitFunction() override {
		g_VisualTreatment = false;
	}
	void DeinitFunction() override {
		g_VisualTreatment = true;
	}
	bool IsAvailable() override {
		return g_VisualTreatment;
	}
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_NoVisualTreatment;


class Effect_Millionaire : public ChaosEffect {
public:
	Effect_Millionaire() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Millionaire";
	}

	void InitFunction() override {
		FEDatabase->mUserProfile->TheCareerSettings.CurrentCash += 1000000;
	}
} E_Millionaire;

class Effect_Millionaire2 : public EffectBase_NotInPrologueConditional {
public:
	Effect_Millionaire2() : EffectBase_NotInPrologueConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Anti-Millionaire";
	}

	void InitFunction() override {
		FEDatabase->mUserProfile->TheCareerSettings.CurrentCash -= 1000000;
	}
} E_Millionaire2;

class Effect_ManualTrans : public ChaosEffect {
public:
	Effect_ManualTrans() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Force Manual Transmission";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("transmission"));
	}

	void TickFunctionMain(double delta) override {
		ForceManualGearbox = true;
	}
	void DeinitFunction() override {
		ForceManualGearbox = false;
	}
	bool IsAvailable() override {
		return GetLocalPlayerInterface<IInput>()->IsAutomaticShift();
	}
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
} E_ManualTrans;

class Effect_AutoTrans : public ChaosEffect {
public:
	Effect_AutoTrans() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Force Automatic Transmission";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("transmission"));
	}

	void TickFunctionMain(double delta) override {
		ForceAutomaticGearbox = true;
	}
	void DeinitFunction() override {
		ForceAutomaticGearbox = false;
	}
	bool IsAvailable() override {
		return !GetLocalPlayerInterface<IInput>()->IsAutomaticShift();
	}
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
} E_AutoTrans;

class Effect_PlayerCarRandomTuning : public EffectBase_TriggerInMenu {
public:
	Effect_PlayerCarRandomTuning() : EffectBase_TriggerInMenu(EFFECT_CATEGORY_TEMP) {
		sName = "Randomize Active Car's Visuals";
		fUnhideTime = 0;
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
	bool CanQuickTrigger() override { return false; }
} E_PlayerCarRandomTuning;

class Effect_PlayerCarImpoundStrike : public EffectBase_CareerConditional {
public:
	Effect_PlayerCarImpoundStrike() : EffectBase_CareerConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Add A Strike To Active Career Car";
	}

	void InitFunction() override {
		auto car = GetCurrentCareerCar();
		if (!car) return;
		auto career = FEPlayerCarDB::GetCareerRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->CareerHandle);
		if (!career) return;
		career->TheImpoundData.mTimesBusted++;
	}
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_PlayerCarImpoundStrike;

class Effect_PlayerCarImpoundStrikeRemove : public ChaosEffect {
public:
	Effect_PlayerCarImpoundStrikeRemove() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Remove A Strike From Active Career Car";
	}

	void InitFunction() override {
		auto car = GetCurrentCareerCar();
		if (!car) return;
		auto career = FEPlayerCarDB::GetCareerRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->CareerHandle);
		if (!career) return;
		career->TheImpoundData.mTimesBusted--;
	}
	bool IsAvailable() override {
		if (GRaceStatus::fObj && GRaceStatus::fObj->mRaceContext != kRaceContext_Career) return false;

		auto car = GetCurrentCareerCar();
		if (!car) return false;
		auto career = FEPlayerCarDB::GetCareerRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->CareerHandle);
		if (!career) return false;
		return career->TheImpoundData.mTimesBusted > 0;
	}
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_PlayerCarImpoundStrikeRemove;

class Effect_PlayerCarImpoundMarker : public EffectBase_CareerConditional {
public:
	Effect_PlayerCarImpoundMarker() : EffectBase_CareerConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Add Extra Impound Slot To Active Career Car";
	}

	void InitFunction() override {
		auto car = GetCurrentCareerCar();
		if (!car) return;
		auto career = FEPlayerCarDB::GetCareerRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->CareerHandle);
		if (!career) return;
		career->TheImpoundData.mMaxBusted++;
	}
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_PlayerCarImpoundMarker;

class Effect_AddRandomTuningMarker : public EffectBase_CareerConditional {
public:
	Effect_AddRandomTuningMarker() : EffectBase_CareerConditional(EFFECT_CATEGORY_TEMP) {
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

		EffectInstance->sNameToDisplay = std::format("{} ({})", sName, selectedMarker.name);
		FEMarkerManager::AddMarkerToInventory(&TheFEMarkerManager, selectedMarker.type, 0);
	}
} E_AddRandomTuningMarker;

class Effect_AddRandomBonusMarker : public EffectBase_CareerConditional {
public:
	Effect_AddRandomBonusMarker() : EffectBase_CareerConditional(EFFECT_CATEGORY_TEMP) {
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
				{"Earl", "BL9", "lancerevo8"},
				{"Baron", "BL10", "caymans"},
				{"Big Lou", "BL11", "eclipsegt"},
				{"Izzy", "BL12", nullptr},//"rx8"},
				{"Vic", "BL13", nullptr},//"supra"},
				{"Taz", "BL14", "is300"},
				{"Sonny", "BL15", "gti"},
				{"Rog", "OPM_MUSTANG_BOSS", "gto"},
				{"Mia", "E3_DEMO_RX8", nullptr},//"rx8"}, // mia isn't actually a blacklist member, is this fine to have?
				{"Razor - Mustang", "RAZORMUSTANG", nullptr},//"mustanggt"},
				{"Ronnie - Supra", "DDAYSUPRA", nullptr},//"supra"},
				{"Cross", nullptr, "copcross"},
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
		EffectInstance->sNameToDisplay = std::format("{} ({})", sName, selectedMarker.name);
		if (selectedMarker.type == FEMarkerManager::MARKER_CASH) {
			FEDatabase->mUserProfile->TheCareerSettings.CurrentCash += 50000;
		}
		else if (selectedMarker.type == FEMarkerManager::MARKER_PINK_SLIP) {
			Achievements::AwardAchievement(GetAchievement("WIN_PINKSLIP"));
			auto ride = unearnedPinkSlips[rand() % unearnedPinkSlips.size()];
			EffectInstance->sNameToDisplay = std::format("{} ({} - {})", sName, selectedMarker.name, ride.name);
			if (ride.preset == nullptr) {
				if (auto car = CreateStockCarRecord(ride.carType)) {
					FEPlayerCarDB::CreateNewCareerCar(&FEDatabase->mUserProfile->PlayersCarStable, car->Handle);
				}
			}
			else {
				CreatePinkSlipPreset(ride.preset);
				FEPlayerCarDB::AwardRivalCar(&FEDatabase->mUserProfile->PlayersCarStable, FEngHashString(ride.preset));
			}
		}
		else {
			FEMarkerManager::AddMarkerToInventory(&TheFEMarkerManager, selectedMarker.type, 0);
		}
	}
	bool CanQuickTrigger() override { return false; }
} E_AddRandomBonusMarker;

// todo this breaks speedtraps and milestones
/*class Effect_SkipChance : public ChaosEffect {
public:
	bool chanceFired = false;

	Effect_SkipChance() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "10% Chance Of Boss Skip";
	}

	void InitFunction() override {
		chanceFired = false;
	}
	void TickFunctionMain(double delta) override {
		if (!chanceFired && EffectInstance->fTimer < fTimerLength - 3) {
			if (rand() % 100 < 10) {
				FEDatabase->mUserProfile->TheCareerSettings.CurrentBin--;
				aMainLoopFunctionsOnce.push_back([]() { EQuitToFE::Create(GARAGETYPE_CAREER_SAFEHOUSE, "MainMenu_Sub.fng"); });
				EffectInstance->sNameToDisplay = std::format("{} (Succeeded)", sName);
			}
			else {
				EffectInstance->sNameToDisplay = std::format("{} (Failed)", sName);
			}
			chanceFired = true;
		}
	}
	bool IsAvailable() override {
		return FEDatabase->mUserProfile->TheCareerSettings.CurrentBin > 1;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool IgnoreHUDState() override { return true; }
} E_SkipChance;*/

class Effect_SkipMusic : public ChaosEffect {
public:
	Effect_SkipMusic() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Skip Current Song";
	}

	void InitFunction() override {
		auto obj = (SFXObj_PFEATrax*)EAXSound::GetSFXBase_Object(g_pEAXSound, 0x40010010);
		if (!obj) return;
		auto song = Songs[rand()%Songs.size()];
		obj->StartLicensedMusic(song->PathEvent);
	}
	bool IsAvailable() override {
		return FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.IGMusicVol > 0 && !GetLocalPlayerInterface<IPerpetrator>()->IsBeingPursued();
	}
	bool AbortOnConditionFailed() override { return true; }
} E_SkipMusic;

class Effect_OverwriteCareerCar : public ChaosEffect {
public:
	Effect_OverwriteCareerCar() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Overwrite Active Career Car";
	}

	void InitFunction() override {
		auto car = GetCurrentCareerCar();
		if (!car) return;
		car->VehicleKey = Attrib::StringHash32(GetLocalPlayerVehicle()->GetVehicleName());
		car->FEKey = GetCarFEKey(car->VehicleKey);

		if (auto customization = FEPlayerCarDB::GetCustomizationRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->Customization)) {
			auto oldId = customization->Handle;
			if (auto current = GetLocalPlayerVehicle()->GetCustomizations()) {
				*customization = *current;
			}
			else {
				*customization = CreateStockCustomizations(car->VehicleKey);
			}
			customization->Handle = oldId;
		}
	}
	// only activate in career mode if your car model is different from your active career car
	bool IsAvailable() override {
		if (!IsInCareerMode()) return false;
		auto car = GetCurrentCareerCar();
		if (!car) return false;
		auto carModel = Attrib::StringHash32(GetLocalPlayerVehicle()->GetVehicleName());
		if (carModel == car->VehicleKey) return false;
		// special case for overwritten ai cars
		auto model1 = GetPVehicleModelPointer(car->VehicleKey);
		if (model1) {
			auto aiCar = GetReplacedAICarName(*model1, false);
			if (aiCar && carModel == Attrib::StringHash32(aiCar)) return false;
		}
		return true;
	}
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_OverwriteCareerCar;