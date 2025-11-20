class Effect_Safehouse : public ChaosEffect {
public:
	Effect_Safehouse() : ChaosEffect() {
		sName = "Exit To Safehouse";
	}

	void InitFunction() override {
		aMainLoopFunctions.push_back([]() { EQuitToFE::Create(GARAGETYPE_CAREER_SAFEHOUSE, "MainMenu_Sub.fng"); });
	}
} E_Safehouse;