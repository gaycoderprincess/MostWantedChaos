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

class Effect_InvisibleWorld : public ChaosEffect {
public:
	Effect_InvisibleWorld() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Invisible World";
		fTimerLength = 30;
	}

	void TickFunctionMain(double delta) override {
		NyaHookLib::Patch(0x723FA0, 0x530008C2);
		NyaHookLib::Patch<uint8_t>(0x6DEF49, 0xEB); // FE
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x723FA0, 0x5314EC83);
		NyaHookLib::Patch<uint8_t>(0x6DEF49, 0x74);
	}
	bool HasTimer() override { return true; }
} E_InvisibleWorld;

// worse duplicate of wireframeworld2, todo is this worth keeping? it does add craziness to some effect combos because it affects the sky
class Effect_WireframeWorld : public ChaosEffect {
public:
	Effect_WireframeWorld() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Wireframe Mode";
		fTimerLength = 60;
		AddToIncompatiblityGroup("fillmode");
	}

	void TickFunctionMain(double delta) override {
		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
	void DeinitFunction() override {
		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	bool HasTimer() override { return true; }
} E_WireframeWorld;

class Effect_WireframeWorld2 : public ChaosEffect {
public:
	Effect_WireframeWorld2() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Wireframe World";
		fTimerLength = 90;
		AddToIncompatiblityGroup("fillmode");
		//AddToIncompatiblityGroup("world_textures"); // if rainbow road replaces this, it gives a unique effect
	}

	void InitFunction() override {
		g_VisualTreatment = false;
	}
	void TickFunction(eChaosHook hook, double delta) override {
		switch (hook) {
			case HOOK_PRE3D:
			case HOOK_PREPROPS:
				static auto texture = LoadTexture("CwoeeChaos/data/textures/wireframe.png");
				pWorldTextureOverride = texture;
				g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
				break;
			case HOOK_POST3D:
			case HOOK_POSTPROPS:
				pWorldTextureOverride = nullptr;
				g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
				break;
		}
	}
	void DeinitFunction() override {
		g_VisualTreatment = true;
	}
	bool IsAvailable() override {
		return g_VisualTreatment;
	}
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_WireframeWorld2;

class Effect_LaserScanWorld : public ChaosEffect {
public:
	Effect_LaserScanWorld() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Laser-Scanned World";
		fTimerLength = 30;
		AddToIncompatiblityGroup("fillmode");
	}

	//static inline float PointSize = 4;

	void TickFunctionMain(double delta) override {
		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING ? D3DFILL_POINT : D3DFILL_SOLID);
		//g_pd3dDevice->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
		//g_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, PointSize);
	}
	void DeinitFunction() override {
		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool CanQuickTrigger() override { return false; }
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
		AddToIncompatiblityGroup("simsurface_friction");
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
		AddToIncompatiblityGroup("simsurface_friction");
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

class Effect_RoadsOnly : public ChaosEffect {
public:
	Effect_RoadsOnly() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Buildings";
		fTimerLength = 90;
	}

	void InitFunction() override {
		SceneryRoadsOnly = true;
	}
	void DeinitFunction() override {
		SceneryRoadsOnly = false;
	}
	bool HasTimer() override { return true; }
} E_RoadsOnly;

class Effect_SceneryBroken : public ChaosEffect {
public:
	Effect_SceneryBroken() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Inception";
		fTimerLength = 30;
	}

	void InitFunction() override {
		SceneryScale.z = 2;
	}
	void DeinitFunction() override {
		SceneryScale.z = 1;
	}
	bool HasTimer() override { return true; }
} E_SceneryBroken;

class Effect_BreakTextures : public ChaosEffect {
public:
	Effect_BreakTextures() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Acid Trip";
		fTimerLength = 60;
		AddToIncompatiblityGroup("world_textures");
	}

	void InitFunction() override {
		NyaHookLib::Patch<uint64_t>(0x6E05DB, 0x8B90909090909090);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch<uint64_t>(0x6E05DB, 0x8B0000010491FF50);
	}
	bool HasTimer() override { return true; }
} E_BreakTextures;

class Effect_RainbowRoad : public ChaosEffect {
public:
	Effect_RainbowRoad() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Rainbow Road";
		sFriendlyName = "Rainbow World";
		fTimerLength = 90;
		AddToIncompatiblityGroup("world_textures");
	}

	void TickFunction(eChaosHook hook, double delta) override {
		switch (hook) {
			case HOOK_PRE3D:
			case HOOK_PREPROPS:
				static auto texture = LoadTexture("CwoeeChaos/data/textures/rainbow.png");
				pWorldTextureOverride = texture;
				break;
			case HOOK_POST3D:
			case HOOK_POSTPROPS:
				pWorldTextureOverride = nullptr;
				break;
		}
	}
	bool HasTimer() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_RainbowRoad;

// todo this causes permanent framerate issues down the line?
class Effect_Snow : public ChaosEffect {
public:
	Effect_Snow() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Snowy World";
		fTimerLength = 120;
		AddToIncompatiblityGroup("world_textures");
	}

	static inline const char* aBannedModelNames[] = {
			"TREE",
			"BUSH",

			// these are wild guesses and idk if ive seen a single model get fixed by these, todo
			//"BUILDING",
			//"HOUSE",
			//"SKYSCRAPER",
			//"TOWER",
			//"FACTORY",
			//"CRANE",
	};

	static bool ShouldSnowifyModel() {
		if (!SceneryDrawingGround) return false;
		for (auto& name : aBannedModelNames) {
			if (SceneryDrawingModelName.find(name) != std::string::npos) return false;
		}
		return true;
	}

	void InitFunction() override {
		UseAlternateSceneryRendering = true;
		g_VisualTreatment = false;
	}
	void TickFunction(eChaosHook hook, double delta) override {
		switch (hook) {
			case HOOK_PRE3D:
				static auto texture = LoadTexture("CwoeeChaos/data/textures/snow.png");
				if (ShouldSnowifyModel()) pWorldTextureOverride = texture;
				break;
			case HOOK_POST3D:
				pWorldTextureOverride = nullptr;
				break;
		}
	}
	void DeinitFunction() override {
		UseAlternateSceneryRendering = false;
		g_VisualTreatment = true;
	}
	bool IsAvailable() override {
		return g_VisualTreatment;
	}
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
	bool RunWhenBlocked() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_Snow;

class Effect_MissingWorld : public ChaosEffect {
public:
	Effect_MissingWorld() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Uninstall Counter-Strike: Source";
		fTimerLength = 60;
		AddToIncompatiblityGroup("world_textures");
	}

	void TickFunction(eChaosHook hook, double delta) override {
		switch (hook) {
			case HOOK_PRE3D:
			case HOOK_PREPROPS:
				static auto texture = LoadTexture("CwoeeChaos/data/textures/missing.png");
				pWorldTextureOverride = texture;
				break;
			case HOOK_POST3D:
			case HOOK_POSTPROPS:
				pWorldTextureOverride = nullptr;
				break;
		}
	}
	bool HasTimer() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_MissingWorld;