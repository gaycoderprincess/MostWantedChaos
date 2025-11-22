class Effect_Blind : public ChaosEffect {
public:
	Effect_Blind() : ChaosEffect() {
		sName = "Blind";
		fTimerLength = 10;
	}

	void TickFunction(double delta) override {
		DrawRectangle(0, 1, 0, 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
} E_Blind;

class Effect_Flash : public ChaosEffect {
public:
	Effect_Flash() : ChaosEffect() {
		sName = "Flashbang";
		fTimerLength = 4;
	}

	void TickFunction(double delta) override {
		uint8_t alpha = 255;
		if (EffectInstance->fTimer < 2) alpha = (EffectInstance->fTimer * 0.5) * 255;
		DrawRectangle(0, 1, 0, 1, {255,255,255,alpha});
	}
} E_Flash;

class Effect_Dark : public ChaosEffect {
public:
	Effect_Dark() : ChaosEffect() {
		sName = "Darken Screen";
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		DrawRectangle(0, 1, 0, 1, {0,0,0,127});
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_Dark;

class Effect_Bright : public ChaosEffect {
public:
	Effect_Bright() : ChaosEffect() {
		sName = "Brighten Screen";
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		DrawRectangle(0, 1, 0, 1, {255,255,255,127});
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_Bright;

class Effect_PortraitMode : public ChaosEffect {
public:
	Effect_PortraitMode() : ChaosEffect() {
		sName = "Portrait Mode";
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		float fadeIn = EffectInstance->fTimer > 1 ? 1 : EffectInstance->fTimer;
		if (EffectInstance->fTimer > fTimerLength - 1) {
			fadeIn = fTimerLength - EffectInstance->fTimer;
		}
		DrawRectangle(0, std::lerp(0,0.33, fadeIn), 0, 1, {0,0,0,255});
		DrawRectangle(std::lerp(1,0.66, fadeIn), 1, 0, 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_PortraitMode;

class Effect_43Borders : public ChaosEffect {
public:
	Effect_43Borders() : ChaosEffect() {
		sName = "4:3 Letterboxed";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		auto fLetterboxMultiplier = ((4.0 / 3.0) / GetAspectRatio()) * 0.5;
		auto left = 0.5 - fLetterboxMultiplier;
		auto right = 0.5 + fLetterboxMultiplier;

		float fadeIn = EffectInstance->fTimer > 1 ? 1 : EffectInstance->fTimer;
		if (EffectInstance->fTimer > fTimerLength - 1) {
			fadeIn = fTimerLength - EffectInstance->fTimer;
		}

		DrawRectangle(0, std::lerp(0,left, fadeIn), 0, 1, {0,0,0,255});
		DrawRectangle(std::lerp(1,right, fadeIn), 1, 0, 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		return (((4.0 / 3.0) / GetAspectRatio()) * 0.5) < 0.99;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool RunInMenus() override { return true; }
} E_43Borders;

class Effect_WidescreenMode : public ChaosEffect {
public:
	Effect_WidescreenMode() : ChaosEffect() {
		sName = "Widescreen Borders";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		float fadeIn = EffectInstance->fTimer > 1 ? 1 : EffectInstance->fTimer;
		if (EffectInstance->fTimer > fTimerLength - 1) {
			fadeIn = fTimerLength - EffectInstance->fTimer;
		}
		DrawRectangle(0, 1, 0, std::lerp(0,0.1, fadeIn), {0,0,0,255});
		DrawRectangle(0, 1, std::lerp(1,0.9, fadeIn), 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_WidescreenMode;

class Effect_FakeCrash : public ChaosEffect {
public:
	Effect_FakeCrash() : ChaosEffect() {
		sName = "Fake Crash";
	}

	void InitFunction() override {
		if (rand() % 100 > 50) {
			Sleep(5000);
		}
		else {
			MessageBoxA(nullptr, "Debug Error!\n\nProgram: speed.exe\n\nR6025\n- pure virtual function call\n\n(Press Retry to debug the application)", "Microsoft Visual C++ Debug Library", MB_ICONERROR | MB_ABORTRETRYIGNORE);
			Sleep(1000);
		}
	}
} E_FakeCrash;

class Effect_Lag : public ChaosEffect {
public:
	Effect_Lag() : ChaosEffect() {
		sName = "Real Frame Drops";
		fTimerLength = 15;
	}

	void TickFunction(double delta) override {
		if (rand() % 100 < 5) {
			Sleep(100);
		}
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_Lag;

class Effect_CrashChance : public ChaosEffect {
public:
	bool chanceFired = false;

	Effect_CrashChance() : ChaosEffect() {
		sName = "10% Chance Of Game Crash";
	}

	void InitFunction() override {
		chanceFired = false;
	}
	void TickFunction(double delta) override {
		if (!chanceFired && EffectInstance->fTimer < fTimerLength - 3) {
			if (rand() % 100 < 10) {
				__debugbreak();
			}
			chanceFired = true;
		}
	}
} E_CrashChance;

class Effect_Nothing : public ChaosEffect {
public:
	Effect_Nothing() : ChaosEffect() {
		sName = "Nothing Happens";
	}
} E_Nothing;

class Effect_BlockyCover : public ChaosEffect {
public:
	Effect_BlockyCover() : ChaosEffect() {
		sName = "Chess Board";
		sFriendlyName = "Chess Board Screen Overlay";
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < 16; y++) {
				float size = 1.0 / 16.0;
				if (y % 2 == 0 && (x % 2 == 0)) continue;
				if (y % 2 != 0 && (x % 2 != 0)) continue;
				//if (y % 2 == 0) continue;
				DrawRectangle(x * size, (x + 1) * size, y * size, (y + 1) * size, {0,0,0,255});
			}
		}
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_BlockyCover;

class Effect_BlockyCoverMissing : public ChaosEffect {
public:
	Effect_BlockyCoverMissing() : ChaosEffect() {
		sName = "Missing Texture";
		sFriendlyName = "Missing Texture Screen Overlay";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < 16; y++) {
				float size = 1.0 / 16.0;
				if ((y % 2 == 0 && (x % 2 == 0)) || (y % 2 != 0 && (x % 2 != 0))){
					DrawRectangle(x * size, (x + 1) * size, y * size, (y + 1) * size, {255,0,255,64});
				}
				else {
					DrawRectangle(x * size, (x + 1) * size, y * size, (y + 1) * size, {0,0,0,64});
				}
			}
		}
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_BlockyCoverMissing;