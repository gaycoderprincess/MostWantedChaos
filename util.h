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

std::vector<IVehicle*> GetActiveVehicles() {
	auto& list = VEHICLE_LIST::GetList(VEHICLE_ALL);
	std::vector<IVehicle*> cars;
	for (int i = 0; i < list.size(); i++) {
		if (!list[i]->IsActive()) continue;
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