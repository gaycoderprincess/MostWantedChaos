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