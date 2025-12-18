void PlayerInputLoop();

auto FetchInput = (void(__thiscall*)(IInputPlayer*))0x6A8870;
void __thiscall FetchInputHooked(IInputPlayer* pThis) {
	FetchInput(pThis);
	PlayerInputLoop();
}

ChloeHook Hook_Input([]() {
	NyaHookLib::Patch(0x8AC668, &FetchInputHooked);
});