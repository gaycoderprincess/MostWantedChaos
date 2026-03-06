bool bPrologueM3Flow_JumpToFreeRoam = false;
void PrologueM3Flow() {
	if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_IN_FRONTEND) return;
	if (IsInLoadingScreen()) return;
	if (IsInMovie()) return;

	if (bPrologueM3Flow_JumpToFreeRoam) {
		GameFlowManager::UnloadFrontend(&TheGameFlowManager);
		bPrologueM3Flow_JumpToFreeRoam = false;
	}
}

void JumpToCarLotHooked() {
	if (GetUserProfile()->TheCareerSettings.CurrentBin == 16 && FEPlayerCarDB::GetNumCareerCars(GetPlayerCarDB()) > 0) {
		EEnterBin::Create(15);
		EQuitToFE::Create(GARAGETYPE_CAREER_SAFEHOUSE, "MainMenu_Sub.fng");
		bPrologueM3Flow_JumpToFreeRoam = true; // to finish the mia activity, launch straight into freeroam
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

bool MovieExistsHooked(const char* name) {
	auto str = (std::string)name;
	if (str.find("storyfmv_bus12") != std::string::npos && GetUserProfile()->TheCareerSettings.CurrentBin == 16 && FEPlayerCarDB::GetNumCareerCars(GetPlayerCarDB()) > 0) {
		return false;
	}
	return bFileExists(name);
}

ChloeHook Hook_PrologueM3([]() {
	NyaHookLib::Patch(0x61EF39 + 1, &JumpToCarLotHooked);
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x611F55, &GetNoPostRaceScreenHooked);
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x559727, &MovieExistsHooked);
	aDrawingLoopFunctions.push_back(PrologueM3Flow);
});