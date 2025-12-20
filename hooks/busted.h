int nTimesBusted = 0;

void __thiscall MessageBustedHooked(NISListenerActivity* pThis, int a2) {
	if (!INIS::mInstance) {
		nTimesBusted++;
		if (nTimesBusted >= 25) {
			Achievements::AwardAchievement(GetAchievement("BUSTED_MANY"));
		}
	}
	return NISListenerActivity::MessageBusted(pThis, a2);
}

ChloeHook Hook_Busted([]() {
	NyaHookLib::Patch(0x44F1C2 + 1, &MessageBustedHooked);
});