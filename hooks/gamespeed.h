float GameSpeedModifier = 1.0;
bool IgnoreSpeedbreakerTime = false;

auto OnManageTime = (float(__thiscall*)(Sim::ITimeManager*, float, float))0x6F4D10;
float __thiscall OnManageTimeHooked(Sim::ITimeManager* pThis, float real_time_delta, float sim_speed) {
	if (IgnoreSpeedbreakerTime || GameSpeedModifier != 1.0f) return GameSpeedModifier;
	auto speed = OnManageTime(pThis, real_time_delta, sim_speed);
	return speed * GameSpeedModifier;
}

ChloeHook Hook_GameSpeed([]() {
	NyaHookLib::Patch(0x8B09A4, &OnManageTimeHooked);
	NyaHookLib::Patch(0x8B0A14, &OnManageTimeHooked);
});