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