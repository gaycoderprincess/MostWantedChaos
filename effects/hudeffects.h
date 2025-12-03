#define EFFECT_CATEGORY_TEMP "HUD"

class Effect_DragHUD : public ChaosEffect {
public:
	Effect_DragHUD() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Drag HUD";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayer()) {
			ply->SetHud(PHT_DRAG);
		}
	}
} E_DragHUD;

class Effect_ChallengeCam : public ChaosEffect {
public:
	Effect_ChallengeCam() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Challenge Series Camera";
	}

	void InitFunction() override {
		Camera_SetGenericCamera("Cinematics", "Challenge");
	}
} E_Challenge;

class Effect_NoHUD : public ChaosEffect {
public:
	Effect_NoHUD() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable HUD";
		sFriendlyName = "Disable Game HUD";
		fTimerLength = 60;
	}

	static uint32_t __thiscall DetermineHudFeaturesHooked(IHud* pThis, IPlayer* a2) {
		return 0;
	}

	void TickFunction(double delta) override {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x58CA50, &DetermineHudFeaturesHooked);
	}
	void DeinitFunction() override {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x58CA50, 0x57CA60);
	}
	bool HasTimer() override { return true; }
} E_NoHUD;

class Effect_PauseMenu : public ChaosEffect {
public:
	Effect_PauseMenu() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Open Pause Menu";
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() { EPause::Create(0, 0, 0); });
	}
} E_PauseMenu;