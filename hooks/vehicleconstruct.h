uint32_t ForcedPlayerVehicle = 0;
std::string ForcedPlayerVehicleModel;
uint32_t ForcedOpponentVehicle = 0;
uint32_t ForcedTrafficVehicle = 0;
bool PlayerFullyTuned = false;
bool OpponentsFullyTuned = false;
bool RandomizePlayerTuning = false;
bool RandomizeOpponentTuning = false;
bool OpponentPlayerCar = false;
bool OpponentPlayerCarRandom = false;
uint32_t PlayerCarModel;
FECustomizationRecord PlayerCarCustomizations;
uint32_t LastOpponentVehicleSpawn = 0;
bool LastOpponentPlayerCar = 0;
bool LastOpponentPlayerCarRandom = 0;
bool LastOpponentFullyTuned = 0;

ISimable* VehicleConstructHooked(Sim::Param params) {
	const char* modelBackup = nullptr;

	auto vehicle = (VehicleParams*)params.mData;
	if (vehicle->carClass == DRIVER_HUMAN) {
		if (ForcedPlayerVehicle) {
			if (!ForcedPlayerVehicleModel.empty()) {
				auto model = GetPVehicleModelPointer(ForcedPlayerVehicle);
				modelBackup = *model;
				*model = ForcedPlayerVehicleModel.c_str();
			}
			vehicle->carType = ForcedPlayerVehicle;
			vehicle->customization = nullptr;
		}
		if (PlayerFullyTuned) {
			if (!vehicle->matched) vehicle->matched = new Physics::Info::Performance;
			vehicle->matched->Acceleration = 100;
			vehicle->matched->Handling = 100;
			vehicle->matched->TopSpeed = 100;
		}
		if (RandomizePlayerTuning) {
			vehicle->customization = new FECustomizationRecord;
			*vehicle->customization = CreateRandomCustomizations(vehicle->carType);
		}
		PlayerCarModel = vehicle->carType;
		if (vehicle->customization) {
			PlayerCarCustomizations = *vehicle->customization;
		}
		else {
			PlayerCarCustomizations = CreateStockCustomizations(vehicle->carType);
		}
	}
	if (vehicle->carClass == DRIVER_RACER) {
		if (ForcedOpponentVehicle) {
			vehicle->carType = ForcedOpponentVehicle;
			vehicle->customization = nullptr;
			LastOpponentVehicleSpawn = ForcedOpponentVehicle;
		}
		else {
			LastOpponentVehicleSpawn = 0;
		}
		if (OpponentPlayerCar) {
			vehicle->carType = PlayerCarModel;
			vehicle->customization = new FECustomizationRecord;
			*vehicle->customization = PlayerCarCustomizations;
			vehicle->matched = nullptr;
			LastOpponentPlayerCar = true;
		}
		else {
			LastOpponentPlayerCar = false;
		}
		if (OpponentPlayerCarRandom) {
			if (auto car = GetRandomCareerCar()) {
				vehicle->carType = car->VehicleKey;
				vehicle->customization = new FECustomizationRecord;
				*vehicle->customization = *FEPlayerCarDB::GetCustomizationRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->Customization);
				vehicle->matched = nullptr;
			}
			else {
				vehicle->carType = PlayerCarModel;
				vehicle->customization = new FECustomizationRecord;
				*vehicle->customization = PlayerCarCustomizations;
				vehicle->matched = nullptr;
			}
			LastOpponentPlayerCarRandom = true;
		}
		else {
			LastOpponentPlayerCarRandom = false;
		}
		if (OpponentsFullyTuned) {
			if (!vehicle->matched) vehicle->matched = new Physics::Info::Performance;
			vehicle->matched->Acceleration = 100;
			vehicle->matched->Handling = 100;
			vehicle->matched->TopSpeed = 100;
			LastOpponentFullyTuned = true;
		}
		else {
			LastOpponentFullyTuned = false;
		}
		if (RandomizeOpponentTuning) {
			vehicle->customization = new FECustomizationRecord;
			*vehicle->customization = CreateRandomCustomizations(vehicle->carType);
		}
	}
	if (vehicle->carClass == DRIVER_TRAFFIC && ForcedTrafficVehicle) {
		vehicle->carType = ForcedTrafficVehicle;
		vehicle->customization = nullptr;
	}
	auto simable = PVehicle::Construct(params);
	if (modelBackup) {
		auto model = GetPVehicleModelPointer(ForcedPlayerVehicle);
		*model = modelBackup;
	}
	return simable;
}

ChloeHook Hook_VehicleConstruct([]() {
	NyaHooks::aLateInitFuncs.push_back([]() { *(void**)0x92C534 = (void*)&VehicleConstructHooked; });

	// use SuspensionRacer instead of SuspensionSimple for racers - fixes popped tire behavior
	NyaHookLib::Patch(0x6380CB + 1, "SuspensionRacer");
	NyaHookLib::Patch(0x67F353 + 1, "SuspensionRacer");
	//NyaHooks::aLateInitFuncs.push_back([]() { *(uintptr_t*)0x938208 = 0x6B6A10; });
});