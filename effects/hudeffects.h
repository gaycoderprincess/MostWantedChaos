class Effect_DragHUD : public ChaosEffect {
public:
	Effect_DragHUD() : ChaosEffect() {
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
	Effect_ChallengeCam() : ChaosEffect() {
		sName = "Challenge Series Camera";
	}

	void InitFunction() override {
		Camera_SetGenericCamera("Cinematics", "Challenge");
	}
} E_Challenge;

class Effect_NoHUD : public ChaosEffect {
public:
	Effect_NoHUD() : ChaosEffect() {
		sName = "Disable HUD";
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