bool ForceManualGearbox = false;
bool ForceAutomaticGearbox = false;

inline auto IsAutomaticShift = (bool(__thiscall*)(IInput*))0x68D090;
static bool __thiscall IsAutomaticShiftHooked(IInput* pThis) {
	if (ForceManualGearbox) return false;
	if (ForceAutomaticGearbox) return true;
	return IsAutomaticShift(pThis);
}

ChloeHook Hook_GearType([]() {
	NyaHookLib::Patch(0x8AC6B0, &IsAutomaticShiftHooked); // normal races
	NyaHookLib::Patch(0x8AC748, &IsAutomaticShiftHooked); // drag races
});