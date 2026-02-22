enum eCareerBin {
	BIN_RAZOR = 1,
	BIN_BULL,
	BIN_RONNIE,
	BIN_JV,
	BIN_WEBSTER,
	BIN_MING,
	BIN_KAZE,
	BIN_JEWELS,
	BIN_EARL,
	BIN_BARON,
	BIN_BIGLOU,
	BIN_IZZY,
	BIN_VIC,
	BIN_TAZ,
	BIN_SONNY,
	BIN_ROG,
};

void WriteLog(const std::string& str) {
	static auto file = std::ofstream("NFSMWChaos_gcp.log");

	file << str;
	file << "\n";
	file.flush();
}

void DisableKeyboardInput(bool disable) {
	NyaHooks::InputBlockerHook::bInputsBlocked = disable;
}

int GetSFXVolume() {
	return FEDatabase->CurrentUserProfiles[0]->TheOptionsSettings.TheAudioSettings.SoundEffectsVol;
}

FEPlayerCarDB* GetPlayerCarDB() {
	return &FEDatabase->CurrentUserProfiles[0]->PlayersCarStable;
}

IPlayer* GetLocalPlayer() {
	auto& list = PLAYER_LIST::GetList(PLAYER_LOCAL);
	if (list.empty()) return nullptr;
	return list[0];
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

void ValueEditorMenu(int& value) {
	ChloeMenuLib::BeginMenu();

	static char inputString[1024] = {};
	ChloeMenuLib::AddTextInputToString(inputString, 1024, true);
	ChloeMenuLib::SetEnterHint("Apply");

	if (DrawMenuOption(inputString + (std::string)"...", "", false, false) && inputString[0]) {
		value = std::stoi(inputString);
		memset(inputString,0,sizeof(inputString));
		ChloeMenuLib::BackOut();
	}

	ChloeMenuLib::EndMenu();
}

void ValueEditorMenu(char* value, int len) {
	ChloeMenuLib::BeginMenu();

	static char inputString[1024] = {};
	ChloeMenuLib::AddTextInputToString(inputString, 1024, false);
	ChloeMenuLib::SetEnterHint("Apply");

	if (DrawMenuOption(inputString + (std::string)"...", "", false, false) && inputString[0]) {
		strcpy_s(value, len, inputString);
		memset(inputString,0,sizeof(inputString));
		ChloeMenuLib::BackOut();
	}

	ChloeMenuLib::EndMenu();
}

void QuickValueEditor(const char* name, float& value) {
	if (DrawMenuOption(std::format("{} - {}", name, value))) { ValueEditorMenu(value); }
}

void QuickValueEditor(const char* name, int& value) {
	if (DrawMenuOption(std::format("{} - {}", name, value))) { ValueEditorMenu(value); }
}

void QuickValueEditor(const char* name, bool& value) {
	if (DrawMenuOption(std::format("{} - {}", name, value))) { value = !value; }
}

void QuickValueEditor(const char* name, char* value, int len) {
	if (DrawMenuOption(std::format("{} - {}", name, value))) { ValueEditorMenu(value, len); }
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
	auto sourcePos = *toVehicle->GetPosition();
	UMath::Vector3 sourceFwd;
	toVehicle->mCOMObject->Find<IRigidBody>()->GetForwardVector(&sourceFwd);
	IVehicle* out = nullptr;
	float distance = 99999;
	auto cars = GetActiveVehicles();
	for (auto& car : cars) {
		if (car == toVehicle) continue;
		auto targetPos = *car->GetPosition();
		if ((sourcePos - targetPos).length() < distance) {
			out = car;
			distance = (sourcePos - targetPos).length();
		}
	}
	return out;
}

IVehicle* GetMostInFrontActiveVehicle(IVehicle* toVehicle, float maxDistance = 99999, float inFrontThreshold = 0) {
	auto sourcePos = *toVehicle->GetPosition();
	UMath::Vector3 sourceFwd;
	toVehicle->mCOMObject->Find<IRigidBody>()->GetForwardVector(&sourceFwd);
	IVehicle* out = nullptr;
	float dot = 99999;
	auto cars = GetActiveVehicles();
	for (auto& car : cars) {
		if (car == toVehicle) continue;
		auto targetPos = *car->GetPosition();
		auto dir = (sourcePos - targetPos);
		dir.Normalize();
		if (dir.Dot(sourceFwd) > -inFrontThreshold) continue;
		if ((sourcePos - targetPos).length() > maxDistance) continue;
		if (dir.Dot(sourceFwd) < dot) {
			out = car;
			dot = dir.Dot(sourceFwd);
		}
	}
	return out;
}

IVehicle* GetClosestActiveVehicle(NyaVec3 toCoords) {
	auto sourcePos = toCoords;
	IVehicle* out = nullptr;
	float distance = 99999;
	auto cars = GetActiveVehicles();
	for (auto& car : cars) {
		auto targetPos = *car->GetPosition();
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

bool IsInMovie() {
	return gMoviePlayer;
}

FECarRecord* GetCurrentCareerCar() {
	auto id = FEDatabase->CurrentUserProfiles[0]->TheCareerSettings.CurrentCar;
	if (id < 0) return nullptr;
	if (id >= 200) return nullptr;
	return &GetPlayerCarDB()->CarTable[id];
}

FECarRecord* GetRandomCareerCar() {
	static int counter = rand() % 4;
	std::vector<FECarRecord*> records;
	auto cars = GetPlayerCarDB();
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
	auto cars = GetPlayerCarDB();
	for (auto& car : cars->CarTable) {
		if (car.FilterBits != 0xF0042) continue;
		if (car.VehicleKey != model) continue;
		return true;
	}
	return false;
}

uint32_t GetCarFEKey(uint32_t modelHash) {
	auto collection = Attrib::FindCollection(Attrib::StringHash32("pvehicle"), modelHash);
	if (!collection) return modelHash;

	if (auto type = (uint32_t*)Attrib::Collection::GetData(collection, Attrib::StringHash32("frontend"), 0)) {
		return type[1];
	}
	return modelHash;
}

FECarRecord* CreateStockCarRecord(const char* carModel) {
	uint32_t rideHash = Attrib::StringHash32(carModel);
	auto cars = GetPlayerCarDB();
	for (auto& car : cars->CarTable) {
		if (car.VehicleKey == rideHash) {
			return &car;
		}
	}

	for (auto& car : cars->CarTable) {
		if (car.Handle == 0xFFFFFFFF) {
			car.Handle = rideHash;
			car.FEKey = GetCarFEKey(rideHash);
			car.VehicleKey = rideHash;
			car.FilterBits = 0x10001;
			car.Customization = -1;
			car.CareerHandle = -1;
			return &car;
		}
	}
	return nullptr;
}

int nNumPlayerCarChangesThisRace = 0;
IVehicle* ChangePlayerCarInWorld(uint32_t hash, FECustomizationRecord* record, bool forceNOS = false) {
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

	if (forceNOS && record && record->InstalledPhysics.Part[Physics::Upgrades::PUT_NOS] <= 0) {
		static auto temp = new FECustomizationRecord;
		*temp = *record;
		temp->InstalledPhysics.Part[Physics::Upgrades::PUT_NOS] = 1;
		vehicleParams.customization = temp;
	}

	if (auto newCar = PVehicle::Construct(param)) {
		nNumPlayerCarChangesThisRace++;

		auto racer = GRaceStatus::fObj->GetRacerInfo(GetLocalPlayerSimable());
		newCar->Attach(GetLocalPlayer());
		oldCar->Detach(GetLocalPlayer());
		oldCar->Kill();
		newCar->mCOMObject->Find<IPerpetrator>()->SetHeat(oldHeat);
		newCar->mCOMObject->Find<IRigidBody>()->SetPosition(&oldPos);
		newCar->mCOMObject->Find<IRigidBody>()->SetOrientation(&oldOrient);
		newCar->mCOMObject->Find<IRigidBody>()->SetLinearVelocity(&oldVel);
		newCar->mCOMObject->Find<IRigidBody>()->SetAngularVelocity(&oldAVel);
		newCar->mCOMObject->Find<ITransmission>()->Shift(oldGear);
		if (racer->mhSimable == oldHandle) {
			racer->mhSimable = newCar->GetOwnerHandle();
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

void DrawBottomBar(float progress, NyaDrawing::CNyaRGBA32 rgb, IDirect3DTexture9* texture = nullptr) {
	if (texture) {
		// reverse the order - cover up a texture with the black background
		DrawRectangle(0, 1, 0.975, 1, rgb);
		DrawRectangle(1 - (GetAspectRatioInv() / 0.5625), 1, 0.975, 1, texture ? NyaDrawing::CNyaRGBA32(255,255,255,255) : rgb, 0, texture);
		DrawRectangle(progress, 1, 0.975, 1, {0, 0, 0, 255});
	}
	else {
		DrawRectangle(0, 1, 0.975, 1, {0, 0, 0, 255});
		DrawRectangle(0, progress, 0.975, 1, rgb);
	}
}

struct CwoeeCarPhysicalState {
	UMath::Vector3 pos;
	UMath::Vector4 orient;
	UMath::Vector3 vel;
	UMath::Vector3 avel;

	CwoeeCarPhysicalState() {}
	explicit CwoeeCarPhysicalState(IVehicle* veh) { Capture(veh); }

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

struct CwoeeCarMiscState {
	GearID gear;
	float wheelSpeed[4];
	eTireDamage tireDamage[4];
	bool destroyed = false;
	bool engineDamaged = false;

	CwoeeCarMiscState() {}
	explicit CwoeeCarMiscState(IVehicle* veh) { Capture(veh); }

	void Capture(IVehicle* veh) {
		if (auto i = veh->mCOMObject->Find<ITransmission>()) {
			gear = i->GetGear();
		}
		if (auto i = veh->mCOMObject->Find<ISuspension>()) {
			for (int j = 0; j < 4; j++) {
				wheelSpeed[j] = i->GetWheelAngularVelocity(j);
			}
		}
		if (auto i = veh->mCOMObject->Find<ISpikeable>()) {
			for (int j = 0; j < 4; j++) {
				tireDamage[j] = i->GetTireDamage(j);
			}
		}
		if (auto i = veh->mCOMObject->Find<IDamageable>()) {
			destroyed = i->IsDestroyed();
		}
		if (auto i = veh->mCOMObject->Find<IEngineDamage>()) {
			engineDamaged = i->IsBlown() || i->IsSabotaged();
		}
	}
	void Apply(IVehicle* veh, bool fix) {
		if (auto i = veh->mCOMObject->Find<ITransmission>()) {
			if (i->GetGear() != gear) i->Shift(gear);
		}
		if (auto i = veh->mCOMObject->Find<ISuspension>()) {
			for (int j = 0; j < 4; j++) {
				i->SetWheelAngularVelocity(j, wheelSpeed[j]);
			}
		}

		// only using these to repair a car
		if (!fix) return;

		if (auto i = veh->mCOMObject->Find<ISpikeable>()) {
			for (int j = 0; j < 4; j++) {
				if (i->GetTireDamage(j) > tireDamage[j]) {
					veh->mCOMObject->Find<IDamageable>()->ResetDamage();
				}
			}
		}
		if (auto i = veh->mCOMObject->Find<IDamageable>()) {
			if (i->IsDestroyed() && !destroyed) i->ResetDamage();
		}
		if (auto i = veh->mCOMObject->Find<IEngineDamage>()) {
			if ((i->IsBlown() || i->IsSabotaged()) && !engineDamaged) i->Repair();
		}
	}
};

void TeleportPlayer(UMath::Vector3 pos, UMath::Vector3 fwd) {
	Sim::SetStream(&pos, true);
	GetLocalPlayerVehicle()->SetVehicleOnGround(&pos, &fwd);
}

bool IsCarDestroyed(IVehicle* car, bool tirePopsCount = false) {
	if (auto engine = car->mCOMObject->Find<IEngineDamage>()) {
		if (engine->IsBlown()) return true;
		if (engine->IsSabotaged()) return true;
	}
	if (tirePopsCount) {
		if (auto sus = car->mCOMObject->Find<ISpikeable>()) {
			int count = 0;
			for (int i = 0; i < 4; i++) {
				if (sus->GetTireDamage(i) > TIRE_DAMAGE_NONE) count++;
			}
			if (count >= 2) return true;
		}
	}
	return car->IsDestroyed();
}

bool IsChaosBlocked() {
	if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return true;
	if (IsInLoadingScreen() || IsInNIS() || IsInMovie()) return true;
	if (FEManager::mPauseRequest) return true;
	return false;
}

Camera* GetLocalPlayerCamera() {
	return eViews[EVIEW_PLAYER1].pCamera;
}

NyaVec3 WorldToRenderCoords(NyaVec3 world) {
	return {world.z, -world.x, world.y};
}

NyaVec3 RenderToWorldCoords(NyaVec3 render) {
	return {-render.y, render.z, render.x};
}

NyaMat4x4 WorldToRenderMatrix(NyaMat4x4 world) {
	NyaMat4x4 out;
	out.x = WorldToRenderCoords(world.x);
	out.y = -WorldToRenderCoords(world.y); // v1, up
	out.z = WorldToRenderCoords(world.z);
	out.p = WorldToRenderCoords(world.p);
	return out;
}

// view to world
NyaMat4x4 PrepareCameraMatrix(Camera* pCamera) {
	return pCamera->CurrentKey.Matrix.Invert();
}

// world to view
void ApplyCameraMatrix(Camera* pCamera, NyaMat4x4 mat) {
	auto inv = mat.Invert();
	pCamera->bClearVelocity = true;
	Camera::SetCameraMatrix(pCamera, (bMatrix4*)&inv, 0);
}

// todo this is probably bad
IVehicle* GetRacerFromHandle(HSIMABLE handle) {
	auto cars = GetActiveVehicles();
	for (auto& car : cars) {
		if (car->mCOMObject->Find<ISimable>()->GetOwnerHandle() == handle) return car;
	}
	return nullptr;
}

bool IsLocalPlayerStaging() {
	return GetLocalPlayerVehicle() && GetLocalPlayerVehicle()->IsStaging();
}

uint8_t* GetRaceNumLaps() {
	auto race = GRaceStatus::fObj;
	if (!race) return nullptr;
	if (race->mPlayMode == GRaceStatus::kPlayMode_Roaming) return nullptr;
	if (!GRaceParameters::GetIsLoopingRace(race->mRaceParms)) return nullptr;
	if (GRaceParameters::GetIsPursuitRace(race->mRaceParms)) return nullptr;
	if (auto index = race->mRaceParms->mIndex) {
		return &index->mNumLaps;
	}
	return (uint8_t*)Attrib::Instance::GetAttributePointer(race->mRaceParms->mRaceRecord, Attrib::StringHash32("NumLaps"), 0);
}

NyaVec3 GetRelativeCarOffset(IVehicle* veh, NyaVec3 v) {
	UMath::Matrix4 mat;
	GetLocalPlayerInterface<IRigidBody>()->GetMatrix4(&mat);
	return mat.Invert() * v;
}

// returns false in pursuit races
bool IsInNormalRace() {
	if (!GRaceStatus::fObj) return false;
	if (!GRaceStatus::fObj->mRaceParms) return false;
	if (GRaceParameters::GetIsPursuitRace(GRaceStatus::fObj->mRaceParms)) return false;
	return true;
}

bool IsInPursuitRace() {
	if (!GRaceStatus::fObj) return false;
	if (!GRaceStatus::fObj->mRaceParms) return false;
	if (!GRaceParameters::GetIsPursuitRace(GRaceStatus::fObj->mRaceParms)) return false;
	return true;
}

bool IsPlayerApproachingOldBridge() {
	return (*GetLocalPlayerVehicle()->GetPosition() - NyaVec3(-2878, 220, -729)).length() < 25;
}

const char** GetPVehicleModelPointer(uint32_t pvehicleHash) {
	auto collection = Attrib::FindCollection(Attrib::StringHash32("pvehicle"), pvehicleHash);
	auto model = (uintptr_t)Attrib::Collection::GetData(collection, Attrib::StringHash32("MODEL"), 0);
	return (const char**)(model + 0xC);
}

void ExecuteRenderData_WithHooks();

Camera* pMoverCamera = nullptr;

bool IsInCareerMode() {
	return GRaceStatus::fObj && GRaceStatus::fObj->mRaceContext == GRace::kRaceContext_Career;
}

void ForceEnableCops() {
	ICopMgr::mDisableCops = false;
	ICopMgr::mInstance->LockoutCops(false);
	ICopMgr::mInstance->VehicleSpawningEnabled(true);
}

const char* GetReplacedAICarName(const std::string& aiCar, bool includeOriginalCS) {
	struct tReplacedCar {
		std::string baseVehicle;
		std::string vehicleModel;
	};
	static std::vector<tReplacedCar> replacedCarsCS = {
			{"cs_clio_trafpizza", "PIZZA"},
			{"cs_clio_traftaxi", "TAXI"},
			{"cs_cts_traf_minivan", "MINIVAN"},
			{"cs_cts_traffictruck", "PICKUPA"},
			{"cs_trafcement", "CEMTR"},
			{"cs_trafgarb", "GARB"},

			// cops
			{"cs_viper_copmidsize", "COPMIDSIZE"},
			{"cs_gto_copgto", "COPGTO"},
			{"cs_c6_copsporthench", "COPSPORTHENCH"},
			{"cs_mustang_copsuv", "COPSUV"},
	};
	static std::vector<tReplacedCar> replacedCars = {
			// challenge series traffic
			{"cs_clio_trafpizza", "TRAFPIZZA"},
			{"cs_clio_traftaxi", "TRAFTAXI"},
			{"cs_cts_traf_minivan", "TRAFMINIVAN"},
			{"cs_cts_traffictruck", "TRAFPICKUPA"},
			{"cs_trafcement", "TRAFCEMTR"},
			{"cs_trafgarb", "TRAFGARB"},

			// traffic
			{"cs_cts_traffictruck", "TRAFAMB"}, // looks weird but then again the ambulance looks weird anyway
			{"cs_cts_traffictruck", "TRAFSUVA"},
			{"cs_cts_traf_minivan", "TRAFNEWS"},
			{"cs_cts_traf_minivan", "TRAFSTWAG"}, // floating a bit? im sure noone will notice
			{"cs_cts_traf_minivan", "TRAFVANB"},
			{"cs_clio_trafpizza", "TRAF4DSEDA"},
			{"cs_clio_trafpizza", "TRAF4DSEDB"},
			{"cs_clio_trafpizza", "TRAF4DSEDC"},
			{"cs_clio_trafpizza", "TRAFCOURT"},
			{"cs_clio_trafpizza", "TRAFFICCOUP"},
			{"cs_clio_trafpizza", "TRAFHA"},
			{"cs_trafcement", "TRAFDMPTR"},
			{"cs_trafcement", "TRAFFIRE"},

			// cops
			{"cs_viper_copmidsize", "COPGHOST"},
			{"cs_gto_copgto", "COPGTOGHOST"},
			{"cs_c6_copsporthench", "COPSPORTGHOST"},
			{"cs_c6_copsporthench", "COPSPORT"},
			{"cs_mustang_copsuv", "COPSUVL"},
	};
	if (includeOriginalCS) {
		for (auto& car : replacedCarsCS) {
			if (aiCar == car.vehicleModel) {
				return car.baseVehicle.c_str();
			}
		}
	}
	for (auto& car : replacedCars) {
		if (aiCar == car.vehicleModel) {
			return car.baseVehicle.c_str();
		}
	}
	return nullptr;
}

bool PercentageChanceCheck(int percent) {
	return rand() % 100 < percent;
}

wchar_t gDLLDir[MAX_PATH];
class DLLDirSetter {
public:
	wchar_t backup[MAX_PATH];

	DLLDirSetter() {
		GetCurrentDirectoryW(MAX_PATH, backup);
		SetCurrentDirectoryW(gDLLDir);
	}
	~DLLDirSetter() {
		SetCurrentDirectoryW(backup);
	}
};