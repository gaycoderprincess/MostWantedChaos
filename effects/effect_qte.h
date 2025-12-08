class Effect_QTE : public ChaosEffect {
public:
	int nNumPresses = 0;

	static inline const int numRequiredPresses = 30;

	Effect_QTE() : ChaosEffect("Uncategorized") {
		sName = "Quick-Time Event";
		fTimerLength = 5;
	}

	void InitFunction() override {
		nNumPresses = 0;
	}
	void TickFunction(double delta) override {
		tNyaStringData data;
		data.x = 0.5;
		data.y = 0.5 - data.size;
		data.size = 0.04;
		data.XCenterAlign = true;
		data.outlinea = 255;
		data.outlinedist = 0.025;
		DrawString(data, "PRESS X!");
		data.y += data.size;
		DrawString(data, std::format("{}/{}", nNumPresses, numRequiredPresses));
		data.y += data.size;
		DrawString(data, std::format("TIME: {:.1f}", EffectInstance->fTimer));
		if (IsKeyJustPressed('X')) { nNumPresses++; }
		if (IsPadKeyJustPressed(NYA_PAD_KEY_X)) { nNumPresses++; }
	}
	void DeinitFunction() override {
		if (nNumPresses < numRequiredPresses) {
			if (auto ply = GetLocalPlayerInterface<IEngineDamage>()) {
				ply->Sabotage(3);
			}
		}
	}
} E_QTE;