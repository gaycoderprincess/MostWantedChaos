int NoResetCount = 0;
bool ManualResetImmunity = false;

static inline auto ResetVehicle = (bool(__thiscall*)(IResetable*, bool))0x6B08C0;
static bool __thiscall ResetVehicleHooked(IResetable* pThis, bool manual) {
	if (NoResetCount) return false;
	if (ManualResetImmunity) manual = false;
	return ResetVehicle(pThis, manual);
}

ChloeHook Hook_NoReset([]() {
	NyaHookLib::Patch(0x8ABA18, &ResetVehicleHooked);
});