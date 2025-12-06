void OnWinRace();

auto AdjustStableImpound_EventWinOrig = (void(__cdecl*)(int))nullptr;
void __cdecl AdjustStableImpound_EventWinHooked(int a1) {
	AdjustStableImpound_EventWinOrig(a1);
	OnWinRace();
}

ChloeHook Hook_OnWinRace([]() {
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x611C1D, &AdjustStableImpound_EventWinHooked);
});