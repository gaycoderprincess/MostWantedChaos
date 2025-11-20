class Effect_ResetProps : public ChaosEffect {
public:
	Effect_ResetProps() : ChaosEffect() {
		sName = "Respawn All Props";
	}

	void InitFunction() override {
		aMainLoopFunctions.push_back([]() { EResetProps::Create(); });
	}
} E_ResetProps;