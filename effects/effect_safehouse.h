class Effect_Safehouse : public ChaosEffect {
public:
	Effect_Safehouse() : ChaosEffect() {
		sName = "Exit To Main Menu";
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() { EQuitToFE::Create(GARAGETYPE_MAIN_FE, "MainMenu.fng"); });
	}
} E_Safehouse;