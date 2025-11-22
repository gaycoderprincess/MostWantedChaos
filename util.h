void WriteLog(const std::string& str) {
	static auto file = std::ofstream("NFSMWChaos_gcp.log");

	file << str;
	file << "\n";
	file.flush();
}

void DisableKeyboardInput(bool disable) {
	NyaHooks::bInputsBlocked = disable;
}

IPlayer* GetLocalPlayer() {
	auto& list = PLAYER_LIST::GetList(PLAYER_LOCAL);
	if (list.empty()) return nullptr;
	return list[0];
}

IVehicle* GetOpponentPlayer(int id) {
	auto& list = VEHICLE_LIST::GetList(VEHICLE_AIRACERS);
	if (list.size() < id) return nullptr;
	return list[id];
}

ISimable* GetLocalPlayerSimable() {
	auto ply = GetLocalPlayer();
	if (!ply) return nullptr;
	return ply->GetSimable();
}

template<typename T>
T* GetLocalPlayerInterface() {
	auto ply = GetLocalPlayerSimable();
	if (!ply) return nullptr;
	T* out;
	if (!ply->QueryInterface<T>(&out)) return nullptr;
	return out;
}

auto GetLocalPlayerVehicle() { return GetLocalPlayerInterface<IVehicle>(); }
auto GetLocalPlayerEngine() { return GetLocalPlayerInterface<IEngine>(); }

// from easing-functions by nicolausYes
double easeInOutQuart(double t) {
	if (t < 0.5) {
		t *= t;
		return 8 * t * t;
	} else {
		t = (--t) * t;
		return 1 - 8 * t * t;
	}
}

void ValueEditorMenu(float& value) {
	ChloeMenuLib::BeginMenu();

	static char inputString[1024] = {};
	ChloeMenuLib::AddTextInputToString(inputString, 1024, true);
	ChloeMenuLib::SetEnterHint("Apply");

	if (DrawMenuOption(inputString + (std::string)"...", "", false, false) && inputString[0]) {
		value = std::stof(inputString);
		memset(inputString,0,sizeof(inputString));
		ChloeMenuLib::BackOut();
	}

	ChloeMenuLib::EndMenu();
}

void QuickValueEditor(const char* name, float& value) {
	if (DrawMenuOption(std::format("{} - {}", name, value))) { ValueEditorMenu(value); }
}

inline float TOMPS(float kmh) { return kmh / 3.6; }
inline float TOKMH(float mps) { return mps * 3.6; }

float* GetMaxHeat() {
	auto race = GRaceStatus::fObj;
	if (!race) return nullptr;
	return (float*)Attrib::Instance::GetAttributePointer(&race->mRaceBin->mBinRecord, Attrib::StringHash32("MaxOpenWorldHeat"), 0);
}

std::vector<IVehicle*> GetActiveVehicles(int driverClass = -1) {
	auto& list = VEHICLE_LIST::GetList(VEHICLE_ALL);
	std::vector<IVehicle*> cars;
	for (int i = 0; i < list.size(); i++) {
		if (!list[i]->IsActive()) continue;
		if (list[i]->IsLoading()) continue;
		if (driverClass >= 0 && list[i]->GetDriverClass() != driverClass) continue;
		cars.push_back(list[i]);
	}
	return cars;
}

bool IsVehicleValidAndActive(IVehicle* vehicle) {
	auto cars = GetActiveVehicles();
	for (auto& car : cars) {
		if (car == vehicle) return true;
	}
	return false;
}

IVehicle* GetClosestActiveVehicle(IVehicle* toVehicle) {
	auto sourcePos = *(NyaVec3*)toVehicle->GetPosition();
	IVehicle* out = nullptr;
	float distance = 99999;
	auto cars = GetActiveVehicles();
	for (auto& car : cars) {
		if (car == toVehicle) continue;
		auto targetPos = *(NyaVec3*)car->GetPosition();
		if ((sourcePos - targetPos).length() < distance) {
			out = car;
			distance = (sourcePos - targetPos).length();
		}
	}
	return out;
}

class ChloeHook {
public:
	static inline std::vector<void(*)()> aHooks;

	ChloeHook(void(*pFunction)()) {
		aHooks.push_back(pFunction);
	}
};

bool IsInLoadingScreen() {
	if (FadeScreen::IsFadeScreenOn()) return true;
	if (cFEng::IsPackagePushed(cFEng::mInstance, "Loading.fng")) return true;
	if (cFEng::IsPackagePushed(cFEng::mInstance, "WS_Loading.fng")) return true;
	return false;
}

bool IsInNIS() {
	return INIS::mInstance && INIS::mInstance->IsPlaying();
}

FECustomizationRecord CreateStockCustomizations(uint32_t carModel) {
	FECustomizationRecord record;
	FECustomizationRecord::Default(&record);

	FECarRecord tmp;
	tmp.FEKey = carModel;
	tmp.VehicleKey = carModel;
	RideInfo info;
	RideInfo::Init(&info, FECarRecord::GetType(&tmp), CarRenderUsage_Player, false, false);
	RideInfo::SetStockParts(&info);
	FECustomizationRecord::WriteRideIntoRecord(&record, &info);
	return record;
}

FECustomizationRecord CreateRandomCustomizations(uint32_t carModel) {
	FECustomizationRecord record;
	FECustomizationRecord::Default(&record);

	FECarRecord tmp;
	tmp.FEKey = carModel;
	tmp.VehicleKey = carModel;
	RideInfo info;
	RideInfo::Init(&info, FECarRecord::GetType(&tmp), CarRenderUsage_Player, false, false);
	RideInfo::SetRandomParts(&info);
	RideInfo::SetRandomPaint(&info);
	FECustomizationRecord::WriteRideIntoRecord(&record, &info);
	return record;
}

FECarRecord* GetCurrentCareerCar() {
	auto id = FEDatabase->mUserProfile->TheCareerSettings.CurrentCar;
	if (id < 0) return nullptr;
	if (id >= 200) return nullptr;
	return &FEDatabase->mUserProfile->PlayersCarStable.CarTable[id];
}

FECarRecord* GetRandomCareerCar() {
	static int counter = rand() % 4;
	std::vector<FECarRecord*> records;
	auto cars = &FEDatabase->mUserProfile->PlayersCarStable;
	for (auto& car : cars->CarTable) {
		if (car.Handle == 0xFFFFFFFF) continue;
		auto career = FEPlayerCarDB::GetCareerRecordByHandle(cars, car.CareerHandle);
		auto customization = FEPlayerCarDB::GetCustomizationRecordByHandle(cars, car.Customization);
		if (!career) continue;
		if (!customization) continue;
		records.push_back(&car);
	}
	if (records.empty()) return nullptr;

	// not truly random but i wanna reduce repeats
	if (counter >= records.size()) counter = 0;
	return records[counter++];
}

bool HasPinkSlip(uint32_t model) {
	auto cars = &FEDatabase->mUserProfile->PlayersCarStable;
	for (auto& car : cars->CarTable) {
		if (car.FilterBits != 0xF0042) continue;
		if (car.VehicleKey != model) continue;
		return true;
	}
	return false;
}

FECarRecord* CreatePinkSlipPreset(const char* presetName) {
	uint32_t rideHash = FEngHashString(presetName);
	auto cars = &FEDatabase->mUserProfile->PlayersCarStable;
	for (auto& car : cars->CarTable) {
		if (car.Handle == rideHash) {
			return &car;
		}
	}
	return FEPlayerCarDB::CreateNewPresetCar(cars, presetName);
}

IVehicle* ChangePlayerCarInWorld(uint32_t hash, FECustomizationRecord* record) {
	// really weird hack for transparent car skins, change to a non-skinnable car first
	// works but crashes the game after a few attempts
	//if (hash != Attrib::StringHash32("cs_clio_trafpizza") && GetActiveVehicles(DRIVER_RACER).size() >= 3) {
	//	ChangePlayerCarInWorld(Attrib::StringHash32("cs_clio_trafpizza"), nullptr);
	//}

	WriteLog(std::format("Changing car to {:X}", hash));

	auto oldCar = GetLocalPlayerSimable();
	auto oldRB =  GetLocalPlayerInterface<IRigidBody>();
	auto oldTrans =  GetLocalPlayerInterface<ITransmission>();
	auto oldHandle = oldCar->GetOwnerHandle();

	auto oldHeat = oldCar->mCOMObject->Find<IPerpetrator>()->GetHeat();
	auto oldPos = *oldCar->GetPosition();
	UMath::Vector3 oldFwd;
	oldRB->GetForwardVector(&oldFwd);
	auto oldOrient = *oldRB->GetOrientation();
	auto oldVel = *oldRB->GetLinearVelocity();
	auto oldAVel = *oldRB->GetAngularVelocity();
	auto oldGear = oldTrans->GetGear();

	Sim::Param param;
	VehicleParams vehicleParams;
	param.mType.mCRC = vehicleParams.mType.mCRC;
	param.mName.mCRC = vehicleParams.mName.mCRC;
	param.mData = &vehicleParams;
	vehicleParams.carType = hash;
	vehicleParams.initialPos = &oldPos;
	vehicleParams.initialVec = &oldFwd;
	vehicleParams.carClass = DRIVER_HUMAN;
	vehicleParams.customization = record;
	vehicleParams.VehicleCache = (IVehicleCache*)GRaceStatus::fObj; // this is what GRacerInfo::CreateVehicle does but it looks SO wrong
	if (auto newCar = PVehicle::Construct(param)) {
		newCar->Attach(GetLocalPlayer());
		oldCar->Detach(GetLocalPlayer());
		oldCar->Kill();
		newCar->mCOMObject->Find<IPerpetrator>()->SetHeat(oldHeat);
		newCar->mCOMObject->Find<IRigidBody>()->SetPosition(&oldPos);
		newCar->mCOMObject->Find<IRigidBody>()->SetOrientation(&oldOrient);
		newCar->mCOMObject->Find<IRigidBody>()->SetLinearVelocity(&oldVel);
		newCar->mCOMObject->Find<IRigidBody>()->SetAngularVelocity(&oldAVel);
		newCar->mCOMObject->Find<ITransmission>()->Shift(oldGear);
		if (GRaceStatus::fObj->mRacerInfo[0].mhSimable == oldHandle) {
			GRaceStatus::fObj->mRacerInfo[0].mhSimable = newCar->GetOwnerHandle();
		}
		if (GRaceStatus::fObj->mRaceParms && GRaceParameters::GetRaceType(GRaceStatus::fObj->mRaceParms) != 8) {
			EAXSound::ReStartRace(g_pEAXSound, true);
		}
		else {
			EAXSound::ReStartRace(g_pEAXSound, false);
		}
		return newCar->mCOMObject->Find<IVehicle>();
	}
	return nullptr;
}

IVehicle* SpawnCarInWorld(uint32_t hash, FECustomizationRecord* record, IVehicleCache* cache = ITrafficMgr::mInstance->mCOMObject->Find<IVehicleCache>()) {
	if (!cache) return nullptr;

	auto pos = *GetLocalPlayerSimable()->GetPosition();
	auto fwd = *GetLocalPlayerInterface<ICollisionBody>()->GetForwardVector();

	Sim::Param param;
	VehicleParams vehicleParams;
	param.mType.mCRC = vehicleParams.mType.mCRC;
	param.mName.mCRC = vehicleParams.mName.mCRC;
	param.mData = &vehicleParams;
	vehicleParams.carType = hash;
	vehicleParams.initialPos = &pos;
	vehicleParams.initialVec = &fwd;
	vehicleParams.carClass = DRIVER_NONE;
	vehicleParams.customization = record;
	vehicleParams.VehicleCache = cache;
	if (auto newCar = PVehicle::Construct(param)) {
		return newCar->mCOMObject->Find<IVehicle>();
	}
	return nullptr;
}

void DrawTopBar(float progress, NyaDrawing::CNyaRGBA32 rgb) {
	DrawRectangle(0, 1, 0, 0.025, {0, 0, 0, 255});
	DrawRectangle(0, progress, 0, 0.025, rgb);
}

void DrawBottomBar(float progress, NyaDrawing::CNyaRGBA32 rgb) {
	DrawRectangle(0, 1, 0.975, 1, {0, 0, 0, 255});
	DrawRectangle(0, progress, 0.975, 1, rgb);
}

struct CwoeeCarPhysicalState {
	UMath::Vector3 pos;
	UMath::Vector4 orient;
	UMath::Vector3 vel;
	UMath::Vector3 avel;

	CwoeeCarPhysicalState() {}
	CwoeeCarPhysicalState(IVehicle* veh) { Capture(veh); }

	void Capture(IVehicle* veh) {
		auto rb = veh->mCOMObject->Find<IRigidBody>();
		pos = *rb->GetPosition();
		orient = *rb->GetOrientation();
		vel = *rb->GetLinearVelocity();
		avel = *rb->GetAngularVelocity();
	}
	void Apply(IVehicle* veh) {
		auto rb = veh->mCOMObject->Find<IRigidBody>();
		rb->SetPosition(&pos);
		rb->SetOrientation(&orient);
		rb->SetLinearVelocity(&vel);
		rb->SetAngularVelocity(&avel);
	}
};