class Effect_Safehouse : public ChaosEffect {
public:
	Effect_Safehouse() : ChaosEffect("Uncategorized") {
		sName = "Exit To Main Menu";
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() { EQuitToFE::Create(GARAGETYPE_MAIN_FE, "MainMenu.fng"); });
	}
	bool RunInMenus() override { return true; }
} E_Safehouse;