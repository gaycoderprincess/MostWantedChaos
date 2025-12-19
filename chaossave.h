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

struct t173Save {
	UMath::Matrix4 matrix;
};

struct tTeddieSave {
	UMath::Matrix4 matrix;
	UMath::Vector3 colPos;
};

void DoChaos173Save() {
	std::vector<t173Save> save;
	for (auto& peanut : Effect_173::aPeanutsInWorld) {
		auto model = &Render3DObjects::aObjects[peanut];
		if (!model->IsActive()) continue;
		save.push_back({model->mMatrix});
	}

	std::ofstream file("CwoeeChaos/save/173.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	int count = save.size();
	file.write((char*)&count, sizeof(count));
	for (auto& data : save) {
		file.write((char*)&data, sizeof(data));
	}
}

void DoChaos173Load() {
	std::ifstream file("CwoeeChaos/save/173.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	int count = 0;
	file.read((char*)&count, sizeof(count));

	for (int i = 0; i < count; i++) {
		t173Save save;
		file.read((char*)&save, sizeof(save));
		Effect_173::SpawnPeanut(save.matrix);
	}
}

void DoChaosTeddieSave() {
	std::vector<tTeddieSave> save;
	for (auto& peanut : Effect_Teddie::aTeddiesInWorld) {
		auto model = &Render3DObjects::aObjects[peanut];
		if (!model->IsActive()) continue;
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
		tTeddieSave save;
		file.read((char*)&save, sizeof(save));
		Effect_Teddie::SpawnTeddie(save.matrix, save.colPos);
	}
}

void DoChaosSettingsSave() {
	std::ofstream file("CwoeeChaos/save/settings.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	file.write((char*)&bDarkMode, sizeof(bDarkMode));
}

void DoChaosSettingsLoad() {
	std::ifstream file("CwoeeChaos/save/settings.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	file.read((char*)&bDarkMode, sizeof(bDarkMode));
}

void DoChaosSave() {
	std::filesystem::create_directory("CwoeeChaos");
	std::filesystem::create_directory("CwoeeChaos/save");

	DoChaosEffectSave();
	DoChaos173Save();
	DoChaosTeddieSave();
	DoChaosSettingsSave();
}

void DoChaosLoad() {
	DoChaosEffectLoad();
	DoChaos173Load();
	DoChaosTeddieLoad();
	DoChaosSettingsLoad();
}