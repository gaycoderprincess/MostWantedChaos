#define EFFECT_CATEGORY_TEMP "Basic"

float GetEffectFadeInOut(ChaosEffect* effect, float time, bool ease) {
	float fade = effect->EffectInstance->fTimer > time ? 1 : effect->EffectInstance->fTimer / time;
	if (effect->EffectInstance->fTimer > effect->fTimerLength - time) {
		fade = (effect->fTimerLength - effect->EffectInstance->fTimer) / time;
	}
	return ease ? easeInOutQuart(fade) : fade;
}

class Effect_Blind : public ChaosEffect {
public:
	Effect_Blind() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Blind";
		fTimerLength = 10;
	}

	void TickFunctionMain(double delta) override {
		DrawRectangle(0, 1, 0, 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
} E_Blind;

class Effect_Flash : public ChaosEffect {
public:
	Effect_Flash() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Flashbang";
		fTimerLength = 4;
	}

	NyaAudio::NyaSound sound = 0;

	void InitFunction() override {
		if (!sound) sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/flashbang_explode2.wav");
		if (sound) {
			NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
			NyaAudio::Play(sound);
		}
	}
	void TickFunctionMain(double delta) override {
		uint8_t alpha = 255;
		if (EffectInstance->fTimer < 2) alpha = (EffectInstance->fTimer * 0.5) * 255;
		DrawRectangle(0, 1, 0, 1, {255,255,255,alpha});
	}
} E_Flash;

class Effect_PortraitMode : public ChaosEffect {
public:
	Effect_PortraitMode() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Portrait Mode";
		fTimerLength = 30;
		IncompatibilityGroups.push_back(Attrib::StringHash32("letterbox_aspect"));
	}

	void TickFunctionMain(double delta) override {
		auto fadeIn = GetEffectFadeInOut(this, 1, true);
		DrawRectangle(0, std::lerp(0,0.33, fadeIn), 0, 1, {0,0,0,255});
		DrawRectangle(std::lerp(1,0.66, fadeIn), 1, 0, 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_PortraitMode;

class Effect_43Borders : public ChaosEffect {
public:
	Effect_43Borders() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "4:3 Letterboxed";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("letterbox_aspect"));
	}

	void TickFunctionMain(double delta) override {
		auto fLetterboxMultiplier = ((4.0 / 3.0) / GetAspectRatio()) * 0.5;
		auto left = 0.5 - fLetterboxMultiplier;
		auto right = 0.5 + fLetterboxMultiplier;

		auto fadeIn = GetEffectFadeInOut(this, 1, true);
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
	Effect_WidescreenMode() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Widescreen Borders";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		auto fadeIn = GetEffectFadeInOut(this, 1, true);
		DrawRectangle(0, 1, 0, std::lerp(0,0.1, fadeIn), {0,0,0,255});
		DrawRectangle(0, 1, std::lerp(1,0.9, fadeIn), 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_WidescreenMode;

class Effect_FakeCrash : public ChaosEffect {
public:
	Effect_FakeCrash() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Fake Crash";
	}

	void InitFunction() override {
		if (rand() % 100 > 50) {
			Sleep(rand() % 10 > 5 ? 3000 : 4000);
		}
		else {
			MessageBoxA(nullptr, "Debug Error!\n\nProgram: speed.exe\n\nR6025\n- pure virtual function call\n\n(Press Retry to debug the application)", "Microsoft Visual C++ Debug Library", MB_ICONERROR | MB_ABORTRETRYIGNORE);
			//Sleep(1000);
		}
	}
} E_FakeCrash;

class Effect_Lag : public ChaosEffect {
public:
	Effect_Lag() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Real Frame Drops";
		fTimerLength = 15;
	}

	void TickFunctionMain(double delta) override {
		static double timer = 0;
		timer += delta;
		while (timer > 0.03) {
			if (rand() % 100 < 5) {
				Sleep(100);
			}
			timer -= 0.03;
		}
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_Lag;

class Effect_CrashChance : public ChaosEffect {
public:
	bool chanceFired = false;

	Effect_CrashChance() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "10% Chance Of Game Crash";
	}

	void InitFunction() override {
		chanceFired = false;
	}
	void TickFunctionMain(double delta) override {
		if (!chanceFired && EffectInstance->fTimer < fTimerLength - 3) {
			if ((rand() % 100) < 10) {
				__debugbreak();
			}
			else {
				EffectInstance->sNameToDisplay = std::format("{} (Failed)", sName);
			}
			chanceFired = true;
		}
	}
	bool IgnoreHUDState() override { return true; }
} E_CrashChance;

class Effect_Nothing : public ChaosEffect {
public:
	Effect_Nothing() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Nothing Happens";
	}
} E_Nothing;

class Effect_BlockyCover : public ChaosEffect {
public:
	Effect_BlockyCover() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Chess Board";
		sFriendlyName = "Chess Board Screen Overlay";
		fTimerLength = 30;
	}

	void TickFunctionMain(double delta) override {
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				float size = 1.0 / 8.0;
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
	Effect_BlockyCoverMissing() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Missing Texture";
		sFriendlyName = "Missing Texture Screen Overlay";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
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

class Effect_NoChaosHUD : public ChaosEffect {
public:
	Effect_NoChaosHUD() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Chaos Mod HUD";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		DisableChaosHUD = true;
	}
	void DeinitFunction() override {
		DisableChaosHUD = false;
	}
	bool HasTimer() override { return true; }
	bool IgnoreHUDState() override { return true; }
} E_NoChaosHUD;

class Effect_PunchHole : public ChaosEffect {
public:
	Effect_PunchHole() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Punch-Hole";
		sFriendlyName = "Punch-Hole Screen Overlay";
		fTimerLength = 30;
	}

	void TickFunctionMain(double delta) override {
		static auto texture = LoadTexture("CwoeeChaos/data/textures/punchhole.png");
		DrawRectangle(0, 1, 0, 1, {255,255,255,255}, 0, texture);
	}
	bool HasTimer() override { return true; }
} E_PunchHole;

class Effect_RandomChaosTimer : public ChaosEffect {
public:
	Effect_RandomChaosTimer() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Chaos For Your Chaos";
		sFriendlyName = "Randomized Chaos Timer";
		fTimerLength = 120;
	}

	void DeinitFunction() override {
		fEffectCycleTimerSpeedMult = 1;
	}
	bool HasTimer() override { return true; }
	void OnAnyEffectTriggered() override {
		fEffectCycleTimerSpeedMult = GetRandomNumber(25, 500) * 0.01;
	}
} E_RandomChaosTimer;

class Effect_SFXPursuitBreaker : public ChaosEffect {
public:
	Effect_SFXPursuitBreaker() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Pursuit Breaker Sound Effect";
	}

	NyaAudio::NyaSound sound = 0;

	void InitFunction() override {
		if (!sound) sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/fish.mp3");
		if (sound) {
			NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
			NyaAudio::Play(sound);
		}
	}
} E_SFXPursuitBreaker;

class Effect_3Effects : public ChaosEffect {
public:
	Effect_3Effects() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Activate 3 Effects";
	}

	void InitFunction() override {
		AddRunningEffect(GetRandomEffect(true));
		AddRunningEffect(GetRandomEffect(true));
		AddRunningEffect(GetRandomEffect(true));
	}
} E_3Effects;