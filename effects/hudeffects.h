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

	void TickFunction(double delta) override {
		if (auto hud = GetLocalPlayer()->GetHud()) {
			if (hud->IsHudVisible()) hud->FadeAll(true);
		}
	}
	void DeinitFunction() override {
		if (auto hud = GetLocalPlayer()->GetHud()) {
			hud->FadeAll(false);
		}
	}
	bool HasTimer() override { return true; }
} E_NoHUD;