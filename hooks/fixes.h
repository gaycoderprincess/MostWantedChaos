// fixes a crash when totaling your car after quitting from a race
// in this case GRacerInfo is valid, mRaceParms is not
void __thiscall TotalVehicleFixed(GRacerInfo* pThis) {
	if (!GRaceStatus::fObj->mRaceParms) return;
	GRacerInfo::TotalVehicle(pThis);
}
void __thiscall BlowEngineFixed(GRacerInfo* pThis) {
	if (!GRaceStatus::fObj->mRaceParms) return;
	GRacerInfo::BlowEngine(pThis);
}

// fixes a crash when hitting objects using a player-driven AI car
// the result of this function is not null checked at 4F0E70
EAX_CarState* GetClosestPlayerCarFixed(bVector3* a1) {
	if (auto car = GetClosestPlayerCar(a1)) return car;
	static EAX_CarState temp;
	memset(&temp,0,sizeof(temp));
	return &temp;
}

ChloeHook Hook_GameFixes([]() {
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x63C093, &TotalVehicleFixed);
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x63839A, &BlowEngineFixed);
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x4F0E70, &GetClosestPlayerCarFixed);
});