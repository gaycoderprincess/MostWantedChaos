#define EFFECT_CATEGORY_TEMP "World"

class Effect_ResetProps : public ChaosEffect {
public:
	Effect_ResetProps() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Respawn Props And Pursuit Breakers";
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() { EResetProps::Create(); });
	}
} E_ResetProps;

class Effect_NoTraffic : public ChaosEffect {
public:
	Effect_NoTraffic() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Traffic";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() != DRIVER_TRAFFIC) continue;
			car->Deactivate();
		}
	}
	bool HasTimer() override { return true; }
} E_NoTraffic;

class Effect_SpeedSlow : public ChaosEffect {
public:
	Effect_SpeedSlow() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Slow Motion";
		fTimerLength = 30;
		IncompatibilityGroups.push_back(Attrib::StringHash32("gamespeed"));
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		GameSpeedModifier = 1 - (GetEffectFadeInOut(this, 1, false) * 0.5);
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedSlow;

class Effect_SpeedFast : public ChaosEffect {
public:
	Effect_SpeedFast() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Fast Motion";
		fTimerLength = 30;
		IncompatibilityGroups.push_back(Attrib::StringHash32("gamespeed"));
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		GameSpeedModifier = 1 + GetEffectFadeInOut(this, 1, false);
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedFast;

class Effect_SpeedSpeedBased : public ChaosEffect {
public:
	Effect_SpeedSpeedBased() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Reverse Superhot";
		fTimerLength = 30;
		IncompatibilityGroups.push_back(Attrib::StringHash32("gamespeed"));
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		auto speed = GetLocalPlayerVehicle()->GetSpeed();
		if (speed < TOMPS(10)) {
			GameSpeedModifier = TOMPS(150) / TOMPS(10);
		}
		else {
			GameSpeedModifier = TOMPS(150) / speed;
		}
		if (GameSpeedModifier > 3) GameSpeedModifier = 3;
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedSpeedBased;

class Effect_InvisibleWorld : public ChaosEffect {
public:
	Effect_InvisibleWorld() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Invisible World";
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		NyaHookLib::Patch(0x723FA0, 0x530008C2);
		NyaHookLib::Patch<uint8_t>(0x6DEF49, 0xEB); // FE
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x723FA0, 0x5314EC83);
		NyaHookLib::Patch<uint8_t>(0x6DEF49, 0x74);
	}
	bool HasTimer() override { return true; }
} E_InvisibleWorld;

class Effect_WireframeWorld : public ChaosEffect {
public:
	Effect_WireframeWorld() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Wireframe World";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("fillmode"));
	}

	void TickFunction(double delta) override {
		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
	void DeinitFunction() override {
		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	bool HasTimer() override { return true; }
} E_WireframeWorld;

class Effect_LaserScanWorld : public ChaosEffect {
public:
	Effect_LaserScanWorld() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Laser-Scanned World";
		fTimerLength = 30;
		IncompatibilityGroups.push_back(Attrib::StringHash32("fillmode"));
	}

	//static inline float PointSize = 4;

	void TickFunction(double delta) override {
		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING ? D3DFILL_POINT : D3DFILL_SOLID);
		//g_pd3dDevice->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
		//g_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, PointSize);
	}
	void DeinitFunction() override {
		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_LaserScanWorld;

class Effect_SlipperyWorld : public ChaosEffect {
public:
	struct tBackup {
		uint32_t collection;
		uint32_t key;
		float value;
	};
	std::vector<tBackup> backups;

	Effect_SlipperyWorld() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Slippery World";
		fTimerLength = 30;
		IncompatibilityGroups.push_back(Attrib::StringHash32("simsurface_friction"));
	}

	void InitFunction() override {
		backups.clear();
		auto pClass = Attrib::Database::GetClass(Attrib::Database::sThis, Attrib::StringHash32("simsurface"));
		auto collHash = Attrib::Class::GetFirstCollection(pClass);
		while (collHash) {
			auto collection = Attrib::FindCollection(Attrib::StringHash32("simsurface"), collHash);
			if (auto f1 = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("GROUND_FRICTION"), 0)) {
				backups.push_back({collHash, Attrib::StringHash32("GROUND_FRICTION"), *f1});
				*f1 /= 4;
			}
			if (auto f1 = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("DRIVE_GRIP"), 0)) {
				backups.push_back({collHash, Attrib::StringHash32("DRIVE_GRIP"), *f1});
				*f1 /= 2;
			}
			if (auto f1 = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("LATERAL_GRIP"), 0)) {
				backups.push_back({collHash, Attrib::StringHash32("LATERAL_GRIP"), *f1});
				*f1 /= 2;
			}
			if (auto f1 = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("ROLLING_RESISTANCE"), 0)) {
				backups.push_back({collHash, Attrib::StringHash32("ROLLING_RESISTANCE"), *f1});
				*f1 /= 4;
			}
			collHash = Attrib::Class::GetNextCollection(pClass, collHash);
		}
	}
	void DeinitFunction() override {
		for (auto& backup : backups) {
			auto collection = Attrib::FindCollection(Attrib::StringHash32("simsurface"), backup.collection);
			*(float*)Attrib::Collection::GetData(collection, backup.key, 0) = backup.value;
		}
	}
	bool HasTimer() override { return true; }
} E_SlipperyWorld;

class Effect_GrippyWorld : public ChaosEffect {
public:
	struct tBackup {
		uint32_t collection;
		uint32_t key;
		float value;
	};
	std::vector<tBackup> backups;

	Effect_GrippyWorld() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Grippy World";
		fTimerLength = 30;
		IncompatibilityGroups.push_back(Attrib::StringHash32("simsurface_friction"));
	}

	void InitFunction() override {
		backups.clear();
		auto pClass = Attrib::Database::GetClass(Attrib::Database::sThis, Attrib::StringHash32("simsurface"));
		auto collHash = Attrib::Class::GetFirstCollection(pClass);
		while (collHash) {
			auto collection = Attrib::FindCollection(Attrib::StringHash32("simsurface"), collHash);
			if (auto f1 = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("GROUND_FRICTION"), 0)) {
				backups.push_back({collHash, Attrib::StringHash32("GROUND_FRICTION"), *f1});
				*f1 *= 4;
			}
			if (auto f1 = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("DRIVE_GRIP"), 0)) {
				backups.push_back({collHash, Attrib::StringHash32("DRIVE_GRIP"), *f1});
				*f1 *= 4;
			}
			if (auto f1 = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("LATERAL_GRIP"), 0)) {
				backups.push_back({collHash, Attrib::StringHash32("LATERAL_GRIP"), *f1});
				*f1 *= 4;
			}
			if (auto f1 = (float*)Attrib::Collection::GetData(collection, Attrib::StringHash32("ROLLING_RESISTANCE"), 0)) {
				backups.push_back({collHash, Attrib::StringHash32("ROLLING_RESISTANCE"), *f1});
				*f1 *= 4;
			}
			collHash = Attrib::Class::GetNextCollection(pClass, collHash);
		}
	}
	void DeinitFunction() override {
		for (auto& backup : backups) {
			auto collection = Attrib::FindCollection(Attrib::StringHash32("simsurface"), backup.collection);
			*(float*)Attrib::Collection::GetData(collection, backup.key, 0) = backup.value;
		}
	}
	bool HasTimer() override { return true; }
} E_GrippyWorld;

// todo change ColourBloomTint in visuallook for a rainbow world thing?