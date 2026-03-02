void OnWinRace();

auto AdjustStableImpound_EventWinOrig = (void(__cdecl*)(int))nullptr;
void __cdecl AdjustStableImpound_EventWinHooked(int a1) {
	AdjustStableImpound_EventWinOrig(a1);
	OnWinRace();
}

void JumpToCarLotHooked() {
	if (GetUserProfile()->TheCareerSettings.CurrentBin == 16 && FEPlayerCarDB::GetNumCareerCars(GetPlayerCarDB()) > 0) {
		EQuitToFE::Create(GARAGETYPE_CAREER_SAFEHOUSE, "MainMenu_Sub.fng");
		return;
	}
	Game_JumpToCarLot();
}

ChloeHook Hook_OnWinRace([]() {
	AdjustStableImpound_EventWinOrig = (void(__cdecl*)(int))NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x611C1D, &AdjustStableImpound_EventWinHooked);
	NyaHookLib::Patch(0x61EF39 + 1, &JumpToCarLotHooked);
});