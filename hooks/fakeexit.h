int FakeExitFlag = 0;

bool CanAltF4() {
	// allow alt f4 if autosave is off
	//if (!GetUserProfile()->TheOptionsSettings.TheGameplaySettings.AutoSaveOn) return false;

	// in-game, only block alt f4 if in a pursuit or nis (general nis check but should be for busted nis)
	if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING) {
		if (IsInNIS()) return false;
		auto ply = GetLocalPlayerInterface<IPerpetrator>();
		if (ply && ply->IsBeingPursued()) return false;
		return true;
	}

	// otherwise make sure a savegame has been loaded and we're not in the main menu
	if (IsInSplashScreenOrIntros()) return true;

	if (FEDatabase->FEGameMode == eFE_GAME_MODE_NONE) return true;
	if (FEDatabase->FEGameMode == eFE_GAME_MODE_BEAT_GAME) return true;
	if (FEDatabase->FEGameMode == eFE_GAME_MODE_POST_RIVAL) return true;

	return false;
}

bool ShouldBlockQuitBind() {
	if (cFEng::mInstance->IsPackagePushed("Dialog.fng")) return true;
	if (cFEng::mInstance->IsPackagePushed("Infractions.fng")) return true;
	return false;
}

void FakeExitLoop() {
	NyaHookLib::Patch<uint8_t>(0x5A8728, ShouldBlockQuitBind() ? 0xEB : 0x75);

	if (FakeExitFlag) {
		if (CanAltF4()) {
			ExitTheGameFlag = FakeExitFlag;
		}
		else {
			// this is funny but autosave doesnt save infractions due
			//EAutoSave::Create();
			//CwoeeHints::AddHint("Game Saved.");

			CwoeeHints::AddHint("No :3");
		}
		FakeExitFlag = 0;
	}
}

ChloeHook Hook_FakeExit([]() {
	NyaHookLib::Patch(0x6DB7D6, &FakeExitFlag);
	NyaHooks::D3DEndSceneHook::aFunctions.push_back(FakeExitLoop);
});