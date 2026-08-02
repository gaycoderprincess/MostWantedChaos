void PlayerInputLoop();

bool bForcePlayerNOS = false;

InputControls* gCustomPlayerInput = nullptr;
auto GetControls_orig = (InputControls*(__thiscall*)(IInputPlayer*))0x69CBB0;
InputControls* __thiscall GetControlsHooked(IInputPlayer* pThis) {
	auto orig = GetControls_orig(pThis);
	static auto tmp = *orig;
	tmp = *orig;
	gCustomPlayerInput = &tmp;
	if (bForcePlayerNOS) {
		gCustomPlayerInput->fNOS = true;
	}
	PlayerInputLoop();
	return &tmp;
}

ChloeHook Hook_Input([]() {
	NyaHookLib::Patch(0x8AC678, &GetControlsHooked); // race
	NyaHookLib::Patch(0x8AC710, &GetControlsHooked); // drag
});