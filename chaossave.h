struct tObjectSaveNoCol {
	UMath::Matrix4 matrix;
};

struct tObjectSaveWithCol {
	UMath::Matrix4 matrix;
	UMath::Vector3 colPos;
};

void DoChaosEffectSave() {
	std::ofstream file("CwoeeChaos/save/effects.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	int num = ChaosEffect::aEffects.size();
	file.write((char*)&num, sizeof(num));

	for (auto& effect : ChaosEffect::aEffects) {
		file.write((char*)&effect->bTriggeredThisCycle, sizeof(effect->bTriggeredThisCycle));
		file.write((char*)&effect->LastTriggerTime, sizeof(effect->LastTriggerTime));
		file.write((char*)&effect->nTotalTimesActivated, sizeof(effect->nTotalTimesActivated));
	}
}

void DoChaosEffectLoad() {
	std::ifstream file("CwoeeChaos/save/effects.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	int num = 0;
	file.read((char*)&num, sizeof(num));
	if (num != ChaosEffect::aEffects.size()) return;

	for (auto& effect : ChaosEffect::aEffects) {
		file.read((char*)&effect->bTriggeredThisCycle, sizeof(effect->bTriggeredThisCycle));
		file.read((char*)&effect->LastTriggerTime, sizeof(effect->LastTriggerTime));
		file.read((char*)&effect->nTotalTimesActivated, sizeof(effect->nTotalTimesActivated));
	}
}

void DoChaos173Save() {
	std::vector<tObjectSaveNoCol> save;
	for (auto& peanut : Effect_173::aPeanutsInWorld) {
		auto model = &Render3DObjects::aObjects[peanut];
		if (model->IsEmpty()) continue;
		save.push_back({model->mMatrix});
	}

	std::ofstream file("CwoeeChaos/save/173.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	file.write((char*)&Effect_173::bPeanutEverSpawned, sizeof(Effect_173::bPeanutEverSpawned));

	int count = save.size();
	file.write((char*)&count, sizeof(count));
	for (auto& data : save) {
		file.write((char*)&data, sizeof(data));
	}
}

void DoChaos173Load() {
	std::ifstream file("CwoeeChaos/save/173.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	file.read((char*)&Effect_173::bPeanutEverSpawned, sizeof(Effect_173::bPeanutEverSpawned));

	int count = 0;
	file.read((char*)&count, sizeof(count));

	for (int i = 0; i < count; i++) {
		tObjectSaveNoCol save;
		file.read((char*)&save, sizeof(save));
		Effect_173::SpawnPeanut(save.matrix);
	}
}

void DoChaosTeddieSave() {
	std::vector<tObjectSaveWithCol> save;
	for (auto& peanut : Effect_Teddie::aTeddiesInWorld) {
		auto model = &Render3DObjects::aObjects[peanut];
		if (model->IsEmpty()) continue;
		save.push_back({model->mMatrix, model->vColPosition});
	}

	std::ofstream file("CwoeeChaos/save/teddie.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	int count = save.size();
	file.write((char*)&count, sizeof(count));
	for (auto& data : save) {
		file.write((char*)&data, sizeof(data));
	}
}

void DoChaosTeddieLoad() {
	std::ifstream file("CwoeeChaos/save/teddie.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	int count = 0;
	file.read((char*)&count, sizeof(count));

	for (int i = 0; i < count; i++) {
		tObjectSaveWithCol save;
		file.read((char*)&save, sizeof(save));
		Effect_Teddie::SpawnTeddie(save.matrix, save.colPos);
	}
}

void DoChaos8DownSave() {
	std::vector<tObjectSaveWithCol> save;
	for (auto& peanut : Effect_8Down::aObjectsInWorld) {
		auto model = &Render3DObjects::aObjects[peanut];
		if (model->IsEmpty()) continue;
		save.push_back({model->mMatrix, model->vColPosition});
	}

	std::ofstream file("CwoeeChaos/save/8down.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	int count = save.size();
	file.write((char*)&count, sizeof(count));
	for (auto& data : save) {
		file.write((char*)&data, sizeof(data));
	}
}

void DoChaos8DownLoad() {
	std::ifstream file("CwoeeChaos/save/8down.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	int count = 0;
	file.read((char*)&count, sizeof(count));

	for (int i = 0; i < count; i++) {
		tObjectSaveWithCol save;
		file.read((char*)&save, sizeof(save));
		Effect_8Down::SpawnObject(save.matrix, save.colPos);
	}
}

void DoChaosBombSave() {
	std::vector<tObjectSaveNoCol> save;
	for (auto& peanut : Effect_ReVoltBomb::aBombsInWorld) {
		auto model = &Render3DObjects::aObjects[peanut];
		if (model->IsEmpty()) continue;
		save.push_back({model->mMatrix});
	}

	std::ofstream file("CwoeeChaos/save/pickup.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	int count = save.size();
	file.write((char*)&count, sizeof(count));
	for (auto& data : save) {
		file.write((char*)&data, sizeof(data));
	}
}

void DoChaosBombLoad() {
	std::ifstream file("CwoeeChaos/save/pickup.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	int count = 0;
	file.read((char*)&count, sizeof(count));

	for (int i = 0; i < count; i++) {
		tObjectSaveNoCol save;
		file.read((char*)&save, sizeof(save));
		Effect_ReVoltBomb::SpawnBomb(save.matrix);
	}
}

void DoChaosVergilSave() {
	std::vector<tObjectSaveNoCol> save;
	for (auto& peanut : Effect_Vergil::aVergilsInWorld) {
		auto model = &Render3DObjects::aObjects[peanut];
		if (model->IsEmpty()) continue;
		save.push_back({model->mMatrix});
	}

	std::ofstream file("CwoeeChaos/save/vergil.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	int count = save.size();
	file.write((char*)&count, sizeof(count));
	for (auto& data : save) {
		file.write((char*)&data, sizeof(data));
	}
}

void DoChaosVergilLoad() {
	std::ifstream file("CwoeeChaos/save/vergil.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	int count = 0;
	file.read((char*)&count, sizeof(count));

	for (int i = 0; i < count; i++) {
		tObjectSaveNoCol save;
		file.read((char*)&save, sizeof(save));
		Effect_Vergil::SpawnVergil(save.matrix);
	}
}

void DoChaosSettingsSave() {
	// using the non-mutex version here as this is called inside TriggerHighestVotedEffect
	// also defining this here so the game can't crash while accessing the irc client and corrupt the savefile
	bool connected = (ChaosVoting::sChannelName[0] && ChaosVoting::bAutoReconnect) || ChaosVoting::IsEnabled();

	std::ofstream file("CwoeeChaos/save/settings.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	file.write((char*)&bDarkMode, sizeof(bDarkMode));
	file.write((char*)&nTimesBusted, sizeof(nTimesBusted));
	file.write((char*)&fEffectCycleTimer, sizeof(fEffectCycleTimer));
	file.write((char*)&fEffectX, sizeof(fEffectX));
	file.write((char*)&fEffectY, sizeof(fEffectY));
	file.write((char*)&fEffectSize, sizeof(fEffectSize));
	file.write((char*)&fEffectSpacing, sizeof(fEffectSpacing));
	file.write((char*)&fEffectVotingSize, sizeof(fEffectVotingSize));
	file.write((char*)&connected, sizeof(connected));
	file.write(ChaosVoting::sChannelName, sizeof(ChaosVoting::sChannelName));
	file.write((char*)&ChaosVoting::nNumVoteOptions, sizeof(ChaosVoting::nNumVoteOptions));
	file.write((char*)&ChaosVoting::bAutoReconnect, sizeof(ChaosVoting::bAutoReconnect));
	file.write((char*)&ChaosVoting::bRandomEffectOption, sizeof(ChaosVoting::bRandomEffectOption));
	file.write((char*)&ChaosVoting::bCVotes, sizeof(ChaosVoting::bCVotes));
	file.write((char*)&ChaosVoting::bProportionalVotes, sizeof(ChaosVoting::bProportionalVotes));
	file.write((char*)&bRNGCycles, sizeof(bRNGCycles));
}

void DoChaosSettingsLoad() {
	std::ifstream file("CwoeeChaos/save/settings.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	file.read((char*)&bDarkMode, sizeof(bDarkMode));
	file.read((char*)&nTimesBusted, sizeof(nTimesBusted));
	file.read((char*)&fEffectCycleTimer, sizeof(fEffectCycleTimer));
	file.read((char*)&fEffectX, sizeof(fEffectX));
	file.read((char*)&fEffectY, sizeof(fEffectY));
	file.read((char*)&fEffectSize, sizeof(fEffectSize));
	file.read((char*)&fEffectSpacing, sizeof(fEffectSpacing));
	file.read((char*)&fEffectVotingSize, sizeof(fEffectVotingSize));
	bool connected = false;
	file.read((char*)&connected, sizeof(connected));
	file.read(ChaosVoting::sChannelName, sizeof(ChaosVoting::sChannelName));
	file.read((char*)&ChaosVoting::nNumVoteOptions, sizeof(ChaosVoting::nNumVoteOptions));
	file.read((char*)&ChaosVoting::bAutoReconnect, sizeof(ChaosVoting::bAutoReconnect));
	file.read((char*)&ChaosVoting::bRandomEffectOption, sizeof(ChaosVoting::bRandomEffectOption));
	file.read((char*)&ChaosVoting::bCVotes, sizeof(ChaosVoting::bCVotes));
	file.read((char*)&ChaosVoting::bProportionalVotes, sizeof(ChaosVoting::bProportionalVotes));
	file.read((char*)&bRNGCycles, sizeof(bRNGCycles));

	if (connected && ChaosVoting::sChannelName[0]) {
		ChaosVoting::Connect();
	}
}

void DoChaosSave() {
	std::filesystem::create_directory("CwoeeChaos");
	std::filesystem::create_directory("CwoeeChaos/save");

	DoChaosEffectSave();
	DoChaos173Save();
	DoChaosTeddieSave();
	DoChaos8DownSave();
	DoChaosBombSave();
	DoChaosVergilSave();
	DoChaosSettingsSave();
}

void DoChaosLoad() {
	DoChaosEffectLoad();
	DoChaos173Load();
	DoChaosTeddieLoad();
	DoChaos8DownLoad();
	DoChaosBombLoad();
	DoChaosVergilLoad();
	DoChaosSettingsLoad();
}