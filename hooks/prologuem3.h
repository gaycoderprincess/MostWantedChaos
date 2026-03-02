void JumpToCarLotHooked() {
	if (GetUserProfile()->TheCareerSettings.CurrentBin == 16 && FEPlayerCarDB::GetNumCareerCars(GetPlayerCarDB()) > 0) {
		EEnterBin::Create(15);
		EQuitToFE::Create(GARAGETYPE_CAREER_SAFEHOUSE, "MainMenu_Sub.fng");
		return;
	}
	Game_JumpToCarLot();
}

// disable the instant transition to the razor cutscene
bool __thiscall GetNoPostRaceScreenHooked(GRaceParameters* pThis) {
	if (GetUserProfile()->TheCareerSettings.CurrentBin == 16 && FEPlayerCarDB::GetNumCareerCars(GetPlayerCarDB()) > 0) {
		return false;
	}
	return GRaceParameters::GetNoPostRaceScreen(pThis);
}

ChloeHook Hook_PrologueM3([]() {
	NyaHookLib::Patch(0x61EF39 + 1, &JumpToCarLotHooked);
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x611F55, &GetNoPostRaceScreenHooked);
});