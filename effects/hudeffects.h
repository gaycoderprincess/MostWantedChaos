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