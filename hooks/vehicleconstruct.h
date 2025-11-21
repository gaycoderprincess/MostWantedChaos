uint32_t ForcedPlayerVehicle = 0;
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

ISimable* VehicleConstructHooked(Sim::Param params) {
	auto vehicle = (VehicleParams*)params.mData;
	if (vehicle->carClass == DRIVER_HUMAN) {
		if (ForcedPlayerVehicle) {
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
		}
		if (OpponentPlayerCar) {
			vehicle->carType = PlayerCarModel;
			vehicle->customization = new FECustomizationRecord;
			*vehicle->customization = PlayerCarCustomizations;
			vehicle->matched = nullptr;
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
		}
		if (OpponentsFullyTuned) {
			if (!vehicle->matched) vehicle->matched = new Physics::Info::Performance;
			vehicle->matched->Acceleration = 100;
			vehicle->matched->Handling = 100;
			vehicle->matched->TopSpeed = 100;
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
	return PVehicle::Construct(params);
}

ChloeHook Hook_VehicleConstruct([]() {
	aMainLoopFunctions.push_back([]() { *(void**)0x92C534 = (void*)&VehicleConstructHooked; });
});