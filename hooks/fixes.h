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

// copies the attrib lap count to GRaceIndexData, fixes lap glitch from the in-game menu and makes ingame lap count data persist in general
void FixIndexLapCount() {
	auto race = GRaceStatus::fObj;
	if (!race) return;
	if (!race->mRaceParms) return;
	if (!GRaceParameters::GetIsLoopingRace(race->mRaceParms)) return;
	if (GRaceParameters::GetIsPursuitRace(race->mRaceParms)) return;
	if (auto index = race->mRaceParms->mIndex) {
		index->mNumLaps = *(uint32_t*)Attrib::Instance::GetAttributePointer(race->mRaceParms->mRaceRecord, Attrib::StringHash32("NumLaps"), 0);
	}
}

// always unlock all cars in the career garage
auto IsCarUnlockedOrig = (bool(__cdecl*)(void* a1, uint32_t a2, int a3))nullptr;
bool IsCarUnlockedFixed(void* a1, uint32_t a2, int a3) {
	auto cars = &FEDatabase->mUserProfile->PlayersCarStable;
	if (auto rec = FEPlayerCarDB::GetCarRecordByHandle(cars, a2)) {
		if (FEPlayerCarDB::GetCareerRecordByHandle(cars, rec->CareerHandle) != nullptr) return true;
	}
	return IsCarUnlockedOrig(a1, a2, a3);
}

ChloeHook Hook_GameFixes([]() {
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x63C093, &TotalVehicleFixed);
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x63839A, &BlowEngineFixed);
	NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x4F0E70, &GetClosestPlayerCarFixed);

	// always reuse dummy_skin1 if there are no free skin slots left
	NyaHookLib::Patch<uint16_t>(0x75D2B9, 0x9090);
	NyaHookLib::Patch<uint64_t>(0x75D2BB, 0x9014EB00000001BA); // change UsePrecompositeVinyls path to set edx to 1 and jmp out

	IsCarUnlockedOrig = (bool(__cdecl*)(void* a1, uint32_t a2, int a3))NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x7BF8A2, &IsCarUnlockedFixed);

	aMainLoopFunctions.push_back(FixIndexLapCount);
});